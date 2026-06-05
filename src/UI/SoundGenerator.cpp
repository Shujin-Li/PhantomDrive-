#include "UI/SoundGenerator.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QRandomGenerator>
#include <cmath>
#include <cstring>

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
    QString basePath = QCoreApplication::applicationDirPath();
    return basePath + "/assets/sounds/";
}

void SoundGenerator::ensureSoundsDirectory() const
{
    QDir dir(getSoundsDirectory());
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

void SoundGenerator::writeInt16LittleEndian(QByteArray& buffer, quint16 value)
{
    buffer.append(static_cast<char>(value & 0xFF));
    buffer.append(static_cast<char>((value >> 8) & 0xFF));
}

void SoundGenerator::writeInt32LittleEndian(QByteArray& buffer, quint32 value)
{
    buffer.append(static_cast<char>(value & 0xFF));
    buffer.append(static_cast<char>((value >> 8) & 0xFF));
    buffer.append(static_cast<char>((value >> 16) & 0xFF));
    buffer.append(static_cast<char>((value >> 24) & 0xFF));
}

QByteArray SoundGenerator::createWavHeader(int dataSize, int numChannels, int sampleRate, int bitsPerSample)
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

QByteArray SoundGenerator::createBeepWav(int frequency, int durationMs, int sampleRate)
{
    const int numSamples = (sampleRate * durationMs) / 1000;
    const int numChannels = 1;
    const int bitsPerSample = 16;

    QByteArray audioData;
    audioData.reserve(numSamples * sizeof(quint16));

    const double amplitude = 0.5;
    const double pi = 3.14159265358979323846;

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double envelope = 1.0;

        double fadeTime = 0.01;
        if (t < fadeTime) {
            envelope = t / fadeTime;
        } else if (t > (durationMs / 1000.0 - fadeTime)) {
            envelope = (durationMs / 1000.0 - t) / fadeTime;
        }

        double sample = amplitude * envelope * std::sin(2.0 * pi * frequency * t);
        qint16 sampleInt = static_cast<qint16>(sample * 32767.0);

        writeInt16LittleEndian(audioData, static_cast<quint16>(sampleInt));
    }

    QByteArray header = createWavHeader(audioData.size(), numChannels, sampleRate, bitsPerSample);

    QByteArray wavFile;
    wavFile.append(header);
    wavFile.append(audioData);

    return wavFile;
}

QByteArray SoundGenerator::createNoiseWav(int durationMs, int sampleRate)
{
    const int numSamples = (sampleRate * durationMs) / 1000;
    const int numChannels = 1;
    const int bitsPerSample = 16;

    QByteArray audioData;
    audioData.reserve(numSamples * sizeof(quint16));

    const double amplitude = 0.3;

    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;

        double envelope = 1.0;
        double fadeTime = 0.01;
        if (t < fadeTime) {
            envelope = t / fadeTime;
        } else if (t > (durationMs / 1000.0 - fadeTime)) {
            envelope = (durationMs / 1000.0 - t) / fadeTime;
        }

        double noise = (static_cast<double>(QRandomGenerator::global()->bounded(RAND_MAX)) / RAND_MAX) * 2.0 - 1.0;
        double sample = amplitude * envelope * noise;

        qint16 sampleInt = static_cast<qint16>(sample * 32767.0);
        writeInt16LittleEndian(audioData, static_cast<quint16>(sampleInt));
    }

    QByteArray header = createWavHeader(audioData.size(), numChannels, sampleRate, bitsPerSample);

    QByteArray wavFile;
    wavFile.append(header);
    wavFile.append(audioData);

    return wavFile;
}

QByteArray SoundGenerator::generateWavFile(SoundEffect effect, int durationMs) const
{
    switch (effect) {
    case SoundEffect::CountdownBeep:
        return createBeepWav(880, durationMs);
    case SoundEffect::CountdownGo:
        return createBeepWav(1760, durationMs * 2);
    case SoundEffect::Collision:
        return createNoiseWav(durationMs);
    case SoundEffect::SpeedBoost:
        return createBeepWav(1200, durationMs);
    case SoundEffect::Violation:
        return createBeepWav(300, durationMs * 2);
    case SoundEffect::Checkpoint:
        return createBeepWav(1000, durationMs);
    case SoundEffect::LapComplete:
        return createBeepWav(1500, durationMs * 2);
    case SoundEffect::PowerupCollect:
        return createBeepWav(2000, durationMs);
    default:
        return createBeepWav(440, durationMs);
    }
}

QString SoundGenerator::getSoundFilePath(SoundEffect effect) const
{
    QString fileName;
    switch (effect) {
    case SoundEffect::CountdownBeep: fileName = "countdown_beep.wav"; break;
    case SoundEffect::CountdownGo: fileName = "countdown_go.wav"; break;
    case SoundEffect::Collision: fileName = "collision.wav"; break;
    case SoundEffect::SpeedBoost: fileName = "speed_boost.wav"; break;
    case SoundEffect::Violation: fileName = "violation.wav"; break;
    case SoundEffect::Checkpoint: fileName = "checkpoint.wav"; break;
    case SoundEffect::LapComplete: fileName = "lap_complete.wav"; break;
    case SoundEffect::PowerupCollect: fileName = "powerup_collect.wav"; break;
    default: fileName = "default.wav"; break;
    }
    return getSoundsDirectory() + fileName;
}

void SoundGenerator::generateAllSounds() const
{
    ensureSoundsDirectory();

    QMap<SoundEffect, int> durations = {
        { SoundEffect::CountdownBeep, 200 },
        { SoundEffect::CountdownGo, 400 },
        { SoundEffect::Collision, 300 },
        { SoundEffect::SpeedBoost, 250 },
        { SoundEffect::Violation, 400 },
        { SoundEffect::Checkpoint, 200 },
        { SoundEffect::LapComplete, 500 },
        { SoundEffect::PowerupCollect, 200 }
    };

    for (auto it = durations.constBegin(); it != durations.constEnd(); ++it) {
        QString filePath = getSoundFilePath(it.key());

        if (QFile::exists(filePath)) {
            continue;
        }

        QByteArray wavData = generateWavFile(it.key(), it.value());

        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(wavData);
            file.close();
            qDebug() << "Generated:" << filePath;
        } else {
            qWarning() << "Failed to create:" << filePath;
        }
    }

    qDebug() << "Sound generation complete!";
}
