#include "UI/SoundGenerator.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QRandomGenerator>
#include <cmath>

using namespace PhantomDrive;

SoundGenerator* SoundGenerator::s_instance = nullptr;

SoundGenerator& SoundGenerator::instance()
{
    if (!s_instance) {
        s_instance = new SoundGenerator();
    }
    return *s_instance;
}

SoundGenerator::SoundGenerator(QObject* parent)
    : QObject(parent)
{
}

SoundGenerator::~SoundGenerator()
{
}

QString SoundGenerator::getSoundsDirectory() const
{
    return QCoreApplication::applicationDirPath() + "/assets/sounds/";
}

void SoundGenerator::ensureSoundsDirectory() const
{
    QDir dir(getSoundsDirectory());
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

void SoundGenerator::writeInt16LittleEndian(QByteArray& buffer, quint16 value) const
{
    buffer.append(static_cast<char>(value & 0xFF));
    buffer.append(static_cast<char>((value >> 8) & 0xFF));
}

void SoundGenerator::writeInt32LittleEndian(QByteArray& buffer, quint32 value) const
{
    buffer.append(static_cast<char>(value & 0xFF));
    buffer.append(static_cast<char>((value >> 8) & 0xFF));
    buffer.append(static_cast<char>((value >> 16) & 0xFF));
    buffer.append(static_cast<char>((value >> 24) & 0xFF));
}

QByteArray SoundGenerator::createWavHeader(int dataSize, int numChannels, int sampleRate, int bitsPerSample) const
{
    QByteArray header;
    header.reserve(44);

    header.append("RIFF");
    writeInt32LittleEndian(header, 36 + dataSize);
    header.append("WAVE");

    header.append("fmt ");
    writeInt32LittleEndian(header, 16);
    writeInt16LittleEndian(header, 1);
    writeInt16LittleEndian(header, static_cast<quint16>(numChannels));
    writeInt32LittleEndian(header, sampleRate);
    writeInt32LittleEndian(header, sampleRate * numChannels * bitsPerSample / 8);
    writeInt16LittleEndian(header, static_cast<quint16>(numChannels * bitsPerSample / 8));
    writeInt16LittleEndian(header, static_cast<quint16>(bitsPerSample));

    header.append("data");
    writeInt32LittleEndian(header, dataSize);

    return header;
}

QByteArray SoundGenerator::createBeepWav(int frequency, int durationMs, int sampleRate) const
{
    const int numSamples = (sampleRate * durationMs) / 1000;
    const int numChannels = 1;
    const int bitsPerSample = 16;

    QByteArray audioData;
    audioData.reserve(numSamples * sizeof(quint16));

    const double amplitude = 0.75;
    const double pi = 3.14159265358979323846;
    const double fadeTime = 0.012;

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double envelope = 1.0;

        if (t < fadeTime) {
            envelope = t / fadeTime;
        } else if (t > (durationMs / 1000.0 - fadeTime)) {
            envelope = (durationMs / 1000.0 - t) / fadeTime;
        }
        envelope = qBound(0.0, envelope, 1.0);

        const double fundamental = std::sin(2.0 * pi * frequency * t);
        const double harmonic2 = std::sin(2.0 * pi * frequency * 2.0 * t) * 0.15;
        const double harmonic3 = std::sin(2.0 * pi * frequency * 3.0 * t) * 0.05;
        double sample = amplitude * envelope * (fundamental + harmonic2 + harmonic3);
        qint16 sampleInt = static_cast<qint16>(qBound(-32767.0, sample * 32767.0, 32767.0));
        writeInt16LittleEndian(audioData, static_cast<quint16>(sampleInt));
    }

    QByteArray header = createWavHeader(audioData.size(), numChannels, sampleRate, bitsPerSample);
    QByteArray wavFile;
    wavFile.append(header);
    wavFile.append(audioData);
    return wavFile;
}

QByteArray SoundGenerator::createNoiseWav(int durationMs, int sampleRate) const
{
    const int numSamples = (sampleRate * durationMs) / 1000;
    const int numChannels = 1;
    const int bitsPerSample = 16;
    const double totalSeconds = qMax(0.001, durationMs / 1000.0);

    QByteArray audioData;
    audioData.reserve(numSamples * sizeof(quint16));
    const double fadeTime = 0.010;
    const double pi = 3.14159265358979323846;

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double envelope = 1.0;

        if (t < fadeTime) {
            envelope = t / fadeTime;
        } else if (t > (totalSeconds - fadeTime)) {
            envelope = (totalSeconds - t) / fadeTime;
        }
        envelope = qBound(0.0, envelope, 1.0);

        const double decay = std::exp(-5.0 * t / totalSeconds);
        const double thump = std::sin(2.0 * pi * 65.0 * t) * 0.45
            + std::sin(2.0 * pi * 130.0 * t) * 0.20;
        const double noise = (static_cast<double>(QRandomGenerator::global()->bounded(RAND_MAX)) / RAND_MAX) * 2.0 - 1.0;
        double sample = 0.80 * envelope * decay * (thump + 0.30 * noise);
        qint16 sampleInt = static_cast<qint16>(qBound(-32767.0, sample * 32767.0, 32767.0));
        writeInt16LittleEndian(audioData, static_cast<quint16>(sampleInt));
    }

    QByteArray header = createWavHeader(audioData.size(), numChannels, sampleRate, bitsPerSample);
    QByteArray wavFile;
    wavFile.append(header);
    wavFile.append(audioData);
    return wavFile;
}

QByteArray SoundGenerator::createVoiceWav(const QList<QPair<int,int>>& formants, int durationMs, int sampleRate) const
{
    const int numSamples = (sampleRate * durationMs) / 1000;
    const int numChannels = 1;
    const int bitsPerSample = 16;
    const double pi = 3.14159265358979323846;
    const double fadeTime = 0.020;

    QByteArray audioData;
    audioData.reserve(numSamples * sizeof(quint16));

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double envelope = 1.0;

        if (t < fadeTime) {
            envelope = t / fadeTime;
        } else if (t > (durationMs / 1000.0 - fadeTime)) {
            envelope = (durationMs / 1000.0 - t) / fadeTime;
        }
        envelope = qBound(0.0, envelope, 1.0);
        envelope = envelope * envelope;

        double sample = 0.0;
        for (const auto& fm : formants) {
            int f = fm.first;
            int bw = fm.second;
            double vibrato = 1.0 + 0.003 * std::sin(2.0 * pi * 6.0 * t);
            sample += std::sin(2.0 * pi * f * vibrato * t) * std::exp(-bw * t);
        }
        sample = qBound(-1.0, sample * 0.45 * envelope, 1.0);
        qint16 sampleInt = static_cast<qint16>(sample * 32767.0);
        writeInt16LittleEndian(audioData, static_cast<quint16>(sampleInt));
    }

    QByteArray header = createWavHeader(audioData.size(), numChannels, sampleRate, bitsPerSample);
    QByteArray wavFile;
    wavFile.append(header);
    wavFile.append(audioData);
    return wavFile;
}

QByteArray SoundGenerator::createEngineLoopWav(int durationMs, int baseFreq, int sampleRate) const
{
    const int numSamples = (sampleRate * durationMs) / 1000;
    const int numChannels = 1;
    const int bitsPerSample = 16;
    const double pi = 3.14159265358979323846;
    const double fadeTime = 0.050;
    const double totalSeconds = durationMs / 1000.0;

    QByteArray audioData;
    audioData.reserve(numSamples * sizeof(quint16));

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double envelope = 1.0;

        if (t < fadeTime) {
            envelope = t / fadeTime;
        } else if (t > (totalSeconds - fadeTime)) {
            envelope = (totalSeconds - t) / fadeTime;
        }
        envelope = qBound(0.0, envelope, 1.0);

        double rpm = baseFreq * (1.0 + 0.15 * std::sin(2.0 * pi * 7.0 * t));
        double sample = std::sin(2.0 * pi * rpm * t) * 0.30;
        sample += std::sin(2.0 * pi * rpm * 2.0 * t) * 0.15;
        sample += std::sin(2.0 * pi * rpm * 3.0 * t) * 0.08;
        sample += std::sin(2.0 * pi * rpm * 4.0 * t) * 0.04;

        const double noise = (static_cast<double>(QRandomGenerator::global()->bounded(RAND_MAX)) / RAND_MAX) * 2.0 - 1.0;
        sample += noise * 0.03;

        sample = qBound(-1.0, sample * envelope, 1.0);
        qint16 sampleInt = static_cast<qint16>(sample * 32767.0);
        writeInt16LittleEndian(audioData, static_cast<quint16>(sampleInt));
    }

    QByteArray header = createWavHeader(audioData.size(), numChannels, sampleRate, bitsPerSample);
    QByteArray wavFile;
    wavFile.append(header);
    wavFile.append(audioData);
    return wavFile;
}

QByteArray SoundGenerator::createBoostWav(int durationMs, int sampleRate) const
{
    const int numSamples = (sampleRate * durationMs) / 1000;
    const int numChannels = 1;
    const int bitsPerSample = 16;
    const double pi = 3.14159265358979323846;

    QByteArray audioData;
    audioData.reserve(numSamples * sizeof(quint16));

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double envelope = std::exp(-3.0 * t);
        double freq = 400.0 + 800.0 * t;
        double sample = std::sin(2.0 * pi * freq * t) * 0.5;
        sample += std::sin(2.0 * pi * freq * 2.0 * t) * 0.3;
        sample += std::sin(2.0 * pi * freq * 3.0 * t) * 0.2;
        sample = qBound(-1.0, sample * envelope, 1.0);
        qint16 sampleInt = static_cast<qint16>(sample * 32767.0);
        writeInt16LittleEndian(audioData, static_cast<quint16>(sampleInt));
    }

    QByteArray header = createWavHeader(audioData.size(), numChannels, sampleRate, bitsPerSample);
    QByteArray wavFile;
    wavFile.append(header);
    wavFile.append(audioData);
    return wavFile;
}

QByteArray SoundGenerator::createChirpWav(int startFreq, int endFreq, int durationMs, int sampleRate) const
{
    const int numSamples = (sampleRate * durationMs) / 1000;
    const int numChannels = 1;
    const int bitsPerSample = 16;
    const double pi = 3.14159265358979323846;
    const double fadeTime = 0.010;
    const double totalSeconds = durationMs / 1000.0;

    QByteArray audioData;
    audioData.reserve(numSamples * sizeof(quint16));

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double envelope = 1.0;

        if (t < fadeTime) {
            envelope = t / fadeTime;
        } else if (t > (totalSeconds - fadeTime)) {
            envelope = (totalSeconds - t) / fadeTime;
        }
        envelope = qBound(0.0, envelope, 1.0);

        double freq = startFreq + (endFreq - startFreq) * (t / totalSeconds);
        double phase = 2.0 * pi * startFreq * t + pi * (endFreq - startFreq) * (t * t / totalSeconds);
        double sample = std::sin(phase) * 0.7;
        sample += std::sin(phase * 2.0) * 0.15;
        sample = qBound(-1.0, sample * envelope, 1.0);
        qint16 sampleInt = static_cast<qint16>(sample * 32767.0);
        writeInt16LittleEndian(audioData, static_cast<quint16>(sampleInt));
    }

    QByteArray header = createWavHeader(audioData.size(), numChannels, sampleRate, bitsPerSample);
    QByteArray wavFile;
    wavFile.append(header);
    wavFile.append(audioData);
    return wavFile;
}

QByteArray SoundGenerator::createFanfareWav(int numNotes, const int* notes, const int* durationsMs, int sampleRate) const
{
    QByteArray result;
    for (int n = 0; n < numNotes; ++n) {
        QByteArray note = createBeepWav(notes[n], durationsMs[n], sampleRate);
        result.append(note);
    }
    return result;
}

QByteArray SoundGenerator::generateWavFile(SoundEffect effect, int durationMs) const
{
    switch (effect) {
    // --- Countdown voice synthesis ---
    case SoundEffect::CountdownThree: {
        QList<QPair<int,int>> f3 = {
            {270, 60}, {2300, 80}, {3000, 100}, {3700, 120}
        };
        return createVoiceWav(f3, 500, 44100);
    }
    case SoundEffect::CountdownTwo: {
        QList<QPair<int,int>> f2 = {
            {280, 60}, {2100, 80}, {2900, 100}, {3500, 120}
        };
        return createVoiceWav(f2, 480, 44100);
    }
    case SoundEffect::CountdownOne: {
        QList<QPair<int,int>> f1 = {
            {300, 60}, {2200, 80}, {3000, 100}, {3800, 120}
        };
        return createVoiceWav(f1, 400, 44100);
    }
    case SoundEffect::CountdownGo: {
        QList<QPair<int,int>> fgo = {
            {300, 80}, {800, 80}, {2000, 80}, {2800, 100}, {3500, 120}
        };
        return createVoiceWav(fgo, 500, 44100);
    }

    // --- UI ---
    case SoundEffect::ButtonClick: {
        QByteArray b;
        int notes1[] = {800, 1200};
        int durs1[] = {60, 60};
        return createFanfareWav(2, notes1, durs1, 44100);
    }
    case SoundEffect::ButtonBack: {
        QByteArray b;
        int notes1[] = {600, 400};
        int durs1[] = {60, 60};
        return createFanfareWav(2, notes1, durs1, 44100);
    }
    case SoundEffect::RaceStart: {
        int notes[] = {660, 880, 1100, 1320};
        int durs[] = {120, 120, 120, 200};
        return createFanfareWav(4, notes, durs, 44100);
    }
    case SoundEffect::Pause: {
        return createBeepWav(600, 150, 44100);
    }
    case SoundEffect::Resume: {
        return createBeepWav(900, 150, 44100);
    }
    case SoundEffect::Victory: {
        int notes[] = {523, 659, 784, 1047};
        int durs[] = {200, 200, 200, 400};
        return createFanfareWav(4, notes, durs, 44100);
    }
    case SoundEffect::Fail: {
        int notes[] = {400, 350, 300, 200};
        int durs[] = {150, 150, 200, 400};
        return createFanfareWav(4, notes, durs, 44100);
    }

    // --- Car ---
    case SoundEffect::EngineIdle:
        return createEngineLoopWav(2000, 80, 44100);
    case SoundEffect::EngineAccel:
        return createEngineLoopWav(3000, 140, 44100);
    case SoundEffect::EngineHighSpeed:
        return createEngineLoopWav(4000, 200, 44100);
    case SoundEffect::Brake: {
        return createChirpWav(600, 100, 300, 44100);
    }
    case SoundEffect::Crash:
        return createNoiseWav(250, 44100);
    case SoundEffect::OffRoad: {
        return createChirpWav(300, 150, 200, 44100);
    }
    case SoundEffect::Boost:
        return createBoostWav(800, 44100);

    // --- Race ---
    case SoundEffect::Checkpoint: {
        return createBeepWav(1175, 200, 44100);
    }
    case SoundEffect::LapComplete: {
        int notes[] = {1047, 1319, 1568};
        int durs[] = {150, 150, 300};
        return createFanfareWav(3, notes, durs, 44100);
    }
    case SoundEffect::FinalLap: {
        int notes[] = {784, 988, 1175, 1568};
        int durs[] = {100, 100, 100, 250};
        return createFanfareWav(4, notes, durs, 44100);
    }
    case SoundEffect::RaceFinish: {
        int notes[] = {523, 659, 784, 1047, 1319, 1568};
        int durs[] = {120, 120, 120, 120, 120, 400};
        return createFanfareWav(6, notes, durs, 44100);
    }

    // --- Items ---
    case SoundEffect::PowerupCollect: {
        return createChirpWav(800, 2000, 250, 44100);
    }

    // --- Legacy ---
    case SoundEffect::CountdownBeep:
        return createBeepWav(880, 160, 44100);
    case SoundEffect::Collision:
        return createNoiseWav(180, 44100);
    case SoundEffect::SpeedBoost:
        return createBoostWav(250, 44100);
    case SoundEffect::Violation:
        return createBeepWav(300, 400, 44100);

    default:
        return createBeepWav(440, 200, 44100);
    }
}

bool SoundGenerator::hasFileFallback(SoundEffect effect) const
{
    Q_UNUSED(effect);
    return false;
}

QString SoundGenerator::getSoundFilePath(SoundEffect effect) const
{
    QString name;
    switch (effect) {
#define CASE(x, n) case SoundEffect::x: name = n; break;
    CASE(CountdownThree, "countdown_three.wav")
    CASE(CountdownTwo, "countdown_two.wav")
    CASE(CountdownOne, "countdown_one.wav")
    CASE(CountdownGo, "countdown_go.wav")
    CASE(ButtonClick, "ui_click.wav")
    CASE(ButtonBack, "ui_back.wav")
    CASE(RaceStart, "race_start.wav")
    CASE(Pause, "pause.wav")
    CASE(Resume, "resume.wav")
    CASE(Victory, "victory.wav")
    CASE(Fail, "fail.wav")
    CASE(EngineIdle, "engine_idle.wav")
    CASE(EngineAccel, "engine_accel.wav")
    CASE(EngineHighSpeed, "engine_high.wav")
    CASE(Brake, "brake.wav")
    CASE(Crash, "crash.wav")
    CASE(OffRoad, "offroad.wav")
    CASE(Boost, "boost.wav")
    CASE(Checkpoint, "checkpoint.wav")
    CASE(LapComplete, "lap_complete.wav")
    CASE(FinalLap, "final_lap.wav")
    CASE(RaceFinish, "race_finish.wav")
    CASE(PowerupCollect, "powerup_collect.wav")
    CASE(CountdownBeep, "countdown_beep.wav")
    CASE(Collision, "collision.wav")
    CASE(SpeedBoost, "speed_boost.wav")
    CASE(Violation, "violation.wav")
#undef CASE
    default: name = "default.wav";
    }
    return getSoundsDirectory() + name;
}

void SoundGenerator::generateAllSounds() const
{
    ensureSoundsDirectory();

    QList<SoundEffect> effects = {
        SoundEffect::CountdownThree,
        SoundEffect::CountdownTwo,
        SoundEffect::CountdownOne,
        SoundEffect::CountdownGo,
        SoundEffect::ButtonClick,
        SoundEffect::ButtonBack,
        SoundEffect::RaceStart,
        SoundEffect::Pause,
        SoundEffect::Resume,
        SoundEffect::Victory,
        SoundEffect::Fail,
        SoundEffect::EngineIdle,
        SoundEffect::EngineAccel,
        SoundEffect::EngineHighSpeed,
        SoundEffect::Brake,
        SoundEffect::Crash,
        SoundEffect::OffRoad,
        SoundEffect::Boost,
        SoundEffect::Checkpoint,
        SoundEffect::LapComplete,
        SoundEffect::FinalLap,
        SoundEffect::RaceFinish,
        SoundEffect::PowerupCollect,
        SoundEffect::CountdownBeep,
        SoundEffect::Collision,
        SoundEffect::SpeedBoost,
        SoundEffect::Violation
    };

    for (SoundEffect effect : effects) {
        QString filePath = getSoundFilePath(effect);
        if (QFile::exists(filePath)) {
            continue;
        }

        QByteArray wavData = generateWavFile(effect);
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(wavData);
            file.close();
        } else {
            qWarning() << "SoundGenerator: Failed to create:" << filePath;
        }
    }
}
