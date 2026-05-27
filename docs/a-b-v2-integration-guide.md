# A-B v2 Integration Guide

This guide describes the final DDL integration surface for the A-B scoring report, AI coach, interaction feedback text, and violation loop.

## Session Lifecycle

Use one `ScoreManager` instance for one learning-mode session.

```cpp
auto* scoreManager = new PhantomDrive::ScoreManager(this);
scoreManager->startSession(vehicleId);
```

When learning mode ends, call `finishSession(collector)`. If no collector is available, call `finishSession()` and A-B will score the pending violation/collision events recorded through `recordViolation()` and `recordCollision()`.

## U-B Floating Feedback

Connect `feedbackReady` and display the text as a floating prompt.

```cpp
connect(scoreManager, &PhantomDrive::ScoreManager::feedbackReady,
        this, [this](const QString& text, int pointsDelta, const QString& severity) {
    showFloatingHint(text, pointsDelta, severity);
});
```

Expected examples include `Great! Safe Driving!`, `Speeding! -5`, `Wall Hit! -8`, `Red Light Violation! -10`, and `Pedestrian Zone Violation! -15`.

Duplicate feedback for the same violation type is throttled for about 1.2 seconds. Safe-driving ticks are not written into the final report.

## U-A Report Window

Connect `reportReady` and render the typed report.

```cpp
connect(scoreManager, &PhantomDrive::ScoreManager::reportReady,
        this, [this](const PhantomDrive::ScoreReport& report) {
    reportWindow->setReport(report);
    reportWindow->show();
});
```

`finishSession()` also emits legacy `scoreReady(report)` for existing demos and integrations.

## U-B History JSON

Connect `reportJsonReady` and save the JSON object as the history record.

```cpp
connect(scoreManager, &PhantomDrive::ScoreManager::reportJsonReady,
        this, [this](const QJsonObject& reportJson) {
    historyStore->append(reportJson);
});
```

The JSON includes `qLearningFeedback`, `metrics`, `breakdown`, `violations`, and `coachAdvices`.

## A-A Adaptive AI

Connect `qLearningFeedbackReady` and consume the reward/action hint.

```cpp
connect(scoreManager, &PhantomDrive::ScoreManager::qLearningFeedbackReady,
        adaptiveAi, &AdaptiveAI::onQLearningFeedbackReady);
```

Important fields are `reward`, `normalizedScore`, `safetyRisk`, `ruleCompliance`, and `recommendedActionHint`.

## E-A Collisions

E-A can convert wall, boundary, and obstacle hits into a unified scoring collision.

```cpp
scoreManager->recordCollision(QPointF(x, y), currentSpeed, QStringLiteral("Wall hit"));
```

`recordCollision()` builds a `ViolationEvent` with `ViolationType::Collision`, current timestamp, position, speed, description, and a conservative UI penalty. The scoring algorithm still uses the existing `DrivingScoreCalculator` collision logic.

## E-B Rule Violations

E-B can directly submit traffic rule events.

```cpp
PhantomDrive::ViolationEvent event;
event.timestamp = QDateTime::currentMSecsSinceEpoch();
event.type = PhantomDrive::ViolationType::SpeedOverLimit;
event.description = QStringLiteral("Speed over limit");
event.position = QVector2D(x, y);
event.speedAtViolation = currentSpeed;
event.speedLimit = speedLimit;
event.penaltyPoints = 5;
scoreManager->recordViolation(event);
```

Supported DDL types are `SpeedOverLimit`, `RedLight`, `PedestrianCollision`, and `Collision`.

For object-manager based integration, E-B can use the existing bridge.

```cpp
scoreManager->setTrafficObjectManager(trafficObjectManager);
```

`ScoreManager` connects `TrafficObjectManager::violationDetected` to `recordViolation()` through the existing `onViolationDetected()` slot. Legacy `addViolation()` also reuses `recordViolation()`.

## Safe Driving Ticks

The learning-mode loop can call:

```cpp
scoreManager->recordSafeDrivingTick(QDateTime::currentMSecsSinceEpoch(), currentSpeed);
```

A-B emits `Great! Safe Driving!` only when speed is above the low-speed threshold, no violation happened in the last 8 seconds, and no Great prompt was shown in the last 10 seconds.

## Async AI Coach And Fallback

Use `finishSession()` or `generateCoachReportAsync(report)` to generate the AI coach report without blocking the UI thread.

```cpp
connect(scoreManager, &PhantomDrive::ScoreManager::coachReportReady,
        this, &CoachPanel::setMarkdown);
```

The async path creates a local `AIAPIClient` inside a background `QThread`, calls `generateCoachReport(report)`, then emits `coachReportReady(markdown)` back on the `ScoreManager` thread.

`AIAPIClient` falls back to a local mock report when API mode is `mock`, API keys are missing, the request times out, or a provider returns an error. This fallback does not affect local scoring, `ScoreReport` generation, JSON history saving, or Q-Learning feedback.

For stable local demos:

```powershell
$env:PHANTOMDRIVE_AI_MODE = "mock"
```

Run `a_b_ddl_event_integration_demo` to verify event injection, feedback prompts, report signals, JSON output, Q-Learning feedback, and async mock coach output.
