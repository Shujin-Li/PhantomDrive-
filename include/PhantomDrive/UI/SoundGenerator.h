#ifndef SOUND_GENERATOR_H
#define SOUND_GENERATOR_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QMap>

#include "PhantomDrive_global.h"
#include "SoundEffects.h"

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT SoundGenerator : public QObject
{
    Q_OBJECT
public:
    static SoundGenerator& instance();

    QByteArray generateWavFile(SoundEffect effect, int durationMs = 200) const;
    QString getSoundFilePath(SoundEffect effect) const;
    QString getSoundsDirectory() const;
    void ensureSoundsDirectory() const;
    void generateAllSounds() const;

    static QByteArray createBeepWav(int frequency, int durationMs, int sampleRate = 44100);
    static QByteArray createNoiseWav(int durationMs, int sampleRate = 44100);

private:
    explicit SoundGenerator(QObject* parent = nullptr);
    ~SoundGenerator();
    SoundGenerator(const SoundGenerator&) = delete;
    SoundGenerator& operator=(const SoundGenerator&) = delete;

    static QByteArray createWavHeader(int dataSize, int numChannels, int sampleRate, int bitsPerSample);
    static void writeInt16LittleEndian(QByteArray& buffer, quint16 value);
    static void writeInt32LittleEndian(QByteArray& buffer, quint32 value);

    static SoundGenerator* s_instance;
};

} // namespace PhantomDrive

#endif // SOUND_GENERATOR_H
