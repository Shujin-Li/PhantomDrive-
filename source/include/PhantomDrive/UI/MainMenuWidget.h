#pragma once

#include "PhantomDrive_global.h"

#include <QGraphicsOpacityEffect>
#include <QObject>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QString>
#include <QWidget>

class QPushButton;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;

namespace PhantomDrive {

class MenuCardWidget;
class StartGameOverlay;

class PHANTOMDRIVE_EXPORT MainMenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainMenuWidget(QWidget* parent = nullptr);

    void setCoinCount(int coins);
    void setReferenceBackgrounds(const QString& mainPath, const QString& startPath);
    void showPrimaryMenu();

signals:
    void arcadeRequested();
    void coinChallengeRequested();
    void learningRequested();
    void aiDemoRequested();
    void garageRequested();
    void trackStudioRequested();
    void recordsRequested();
    void guideRequested();
    void exitRequested();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void relayout();
    void showStartGameOverlay();
    void hideStartGameOverlay();

    QPixmap m_referenceBackground;
    int m_coinCount = 0;
    MenuCardWidget* m_startGameCard = nullptr;
    MenuCardWidget* m_garageCard = nullptr;
    MenuCardWidget* m_trackStudioCard = nullptr;
    MenuCardWidget* m_recordsGuideCard = nullptr;
    QPushButton* m_guideButton = nullptr;
    QPushButton* m_exitButton = nullptr;
    StartGameOverlay* m_startGameOverlay = nullptr;
    QGraphicsOpacityEffect* m_overlayOpacityEffect = nullptr;
    QPropertyAnimation* m_overlayAnimation = nullptr;
};

} // namespace PhantomDrive
