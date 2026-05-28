#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMutex>
#include <QUrl>
#include "PhantomDrive_global.h"
#include "SoundEffects.h"

namespace PhantomDrive {

class SoundGenerator;

class PHANTOMDRIVE_EXPORT SoundManager : public QObject
{
    Q_OBJECT

public:
    static SoundManager& instance(QObject* parent = nullptr);

    void play(SoundEffect effect);
    void play(const QString& customSoundPath);

    void setVolume(int volume);
    int volume() const;

    void setMuted(bool muted);
    bool isMuted() const;

    void setEnabled(bool enabled);
    bool isEnabled() const;

    void preloadSounds();
    void unloadSounds();

    void setMasterVolume(qreal volume);
    qreal masterVolume() const;

signals:
    void volumeChanged(int volume);
    void mutedChanged(bool muted);
    void soundPlayed(SoundEffect effect);

private:
    explicit SoundManager(QObject* parent = nullptr);
    ~SoundManager();

    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    void initializeSoundPaths();
    QString getSoundPath(SoundEffect effect);
    void ensurePlayerForEffect(SoundEffect effect);
    void useGeneratedSound(SoundEffect effect);
    SoundEffect convertToGeneratorEffect(SoundEffect effect);

    static SoundManager* s_instance;
    static QMutex s_mutex;

    QMap<SoundEffect, QMediaPlayer*> m_players;
    QMap<SoundEffect, QString> m_soundPaths;
    QMap<SoundEffect, bool> m_loaded;

    int m_volume;
    bool m_muted;
    bool m_enabled;
    qreal m_masterVolume;
    bool m_preloaded;

    static constexpr int DEFAULT_VOLUME = 70;
    static constexpr qreal DEFAULT_MASTER_VOLUME = 0.8;
};

} // namespace PhantomDrive

#endif // SOUND_MANAGER_H
