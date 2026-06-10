#ifndef SOUND_GENERATOR_H
#define SOUND_GENERATOR_H

#include <QObject>
#include <QMap>
#include <QByteArray>
#include "PhantomDrive_global.h"
#include "UI/SoundEffects.h"

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT SoundGenerator : public QObject
{
    Q_OBJECT
public:
    static SoundGenerator& instance();

    QByteArray generateWavFile(SoundEffect effect, int durationMs = -1) const;
    void generateAllSounds() const;

    QString getSoundsDirectory() const;
    void ensureSoundsDirectory() const;
    QString getSoundFilePath(SoundEffect effect) const;

    bool hasFileFallback(SoundEffect effect) const;

private:
    explicit SoundGenerator(QObject* parent = nullptr);
    ~SoundGenerator();
    SoundGenerator(const SoundGenerator&) = delete;
    SoundGenerator& operator=(const SoundGenerator&) = delete;

    static SoundGenerator* s_instance;

    QByteArray createBeepWav(int frequency, int durationMs, int sampleRate = 44100) const;
    QByteArray createNoiseWav(int durationMs, int sampleRate = 44100) const;
    QByteArray createVoiceWav(const QList<QPair<int,int>>& formants,
                               int durationMs, int sampleRate = 44100) const;
    QByteArray createEngineLoopWav(int durationMs, int baseFreq, int sampleRate = 44100) const;
    QByteArray createBoostWav(int durationMs, int sampleRate = 44100) const;
    QByteArray createChirpWav(int startFreq, int endFreq, int durationMs, int sampleRate = 44100) const;
    QByteArray createFanfareWav(int numNotes, const int* notes, const int* durationsMs,
                                int sampleRate = 44100) const;

    QByteArray createWavHeader(int dataSize, int numChannels, int sampleRate, int bitsPerSample) const;
    void writeInt16LittleEndian(QByteArray& buffer, quint16 value) const;
    void writeInt32LittleEndian(QByteArray& buffer, quint32 value) const;
};

} // namespace PhantomDrive

#endif // SOUND_GENERATOR_H
