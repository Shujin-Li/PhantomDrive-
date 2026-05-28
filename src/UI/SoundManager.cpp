#include "UI/SoundManager.h"
#include "UI/SoundGenerator.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QTemporaryFile>

using namespace PhantomDrive;

QMutex SoundManager::s_mutex;

SoundManager* SoundManager::s_instance = nullptr;

SoundManager& SoundManager::instance(QObject* parent)
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new SoundManager(parent);
    }
    return *s_instance;
}

SoundManager::SoundManager(QObject* parent)
    : QObject(parent)
    , m_volume(DEFAULT_VOLUME)
    , m_muted(false)
    , m_enabled(true)
    , m_masterVolume(DEFAULT_MASTER_VOLUME)
    , m_preloaded(false)
{
    initializeSoundPaths();
}

SoundManager::~SoundManager()
{
    unloadSounds();
}

void SoundManager::initializeSoundPaths()
{
    QString assetsPath = SoundGenerator::instance().getSoundsDirectory();

    m_soundPaths = {
        { SoundEffect::CountdownBeep, assetsPath + "countdown_beep.wav" },
        { SoundEffect::CountdownGo, assetsPath + "countdown_go.wav" },
        { SoundEffect::Collision, assetsPath + "collision.wav" },
        { SoundEffect::SpeedBoost, assetsPath + "speed_boost.wav" },
        { SoundEffect::Violation, assetsPath + "violation.wav" },
        { SoundEffect::Checkpoint, assetsPath + "checkpoint.wav" },
        { SoundEffect::LapComplete, assetsPath + "lap_complete.wav" },
        { SoundEffect::PowerupCollect, assetsPath + "powerup_collect.wav" },
        { SoundEffect::Engine, assetsPath + "engine.wav" },
        { SoundEffect::Brake, assetsPath + "brake.wav" }
    };

    for (auto it = m_soundPaths.begin(); it != m_soundPaths.end(); ++it) {
        m_loaded[it.key()] = false;
    }
}

void SoundManager::ensurePlayerForEffect(SoundEffect effect)
{
    if (m_players.contains(effect)) {
        return;
    }

    QMediaPlayer* player = new QMediaPlayer(this);
    QAudioOutput* audioOutput = new QAudioOutput(this);

    player->setAudioOutput(audioOutput);

    qreal volumeLevel = m_muted ? 0.0 : (m_volume / 100.0) * m_masterVolume;
    audioOutput->setVolume(volumeLevel);

    m_players[effect] = player;

    connect(player, &QMediaPlayer::errorOccurred,
            this, [this, effect](QMediaPlayer::Error error, const QString& errorString) {
        Q_UNUSED(error);
        qDebug() << "SoundManager: Failed to play effect" << static_cast<int>(effect)
                 << "Error:" << errorString;
    });
}

void SoundManager::play(SoundEffect effect)
{
    if (!m_enabled) return;

    ensurePlayerForEffect(effect);

    QMediaPlayer* player = m_players[effect];
    if (!player) return;

    QString soundPath = m_soundPaths.value(effect, "");

    if (!soundPath.isEmpty() && QFileInfo::exists(soundPath)) {
        player->setSource(QUrl::fromLocalFile(soundPath));
    } else {
        qDebug() << "SoundManager: Sound file not found, using generated sound";
        useGeneratedSound(effect);
        return;
    }

    player->stop();
    player->play();

    emit soundPlayed(effect);
}

void SoundManager::useGeneratedSound(SoundEffect effect)
{
    static QMap<SoundEffect, QString> tempFiles;

    if (tempFiles.contains(effect)) {
        QString tempPath = tempFiles[effect];
        if (QFileInfo::exists(tempPath)) {
            m_players[effect]->setSource(QUrl::fromLocalFile(tempPath));
            m_players[effect]->stop();
            m_players[effect]->play();
            emit soundPlayed(effect);
            return;
        }
    }

    QTemporaryFile* tempFile = new QTemporaryFile(QDir::tempPath() + "/phantom_sound_XXXXXX.wav");
    if (tempFile->open()) {
        QString tempPath = tempFile->fileName();
        tempFile->close();

        QByteArray wavData = SoundGenerator::instance().generateWavFile(effect);

        QFile file(tempPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(wavData);
            file.close();

            tempFiles[effect] = tempPath;
            m_players[effect]->setSource(QUrl::fromLocalFile(tempPath));
            m_players[effect]->stop();
            m_players[effect]->play();
            emit soundPlayed(effect);
        }
    }
}

SoundEffect SoundManager::convertToGeneratorEffect(SoundEffect effect)
{
    return effect;
}

void SoundManager::play(const QString& customSoundPath)
{
    if (!m_enabled || customSoundPath.isEmpty()) return;

    QMediaPlayer* player = new QMediaPlayer(this);
    QAudioOutput* audioOutput = new QAudioOutput(this);

    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(m_muted ? 0.0 : (m_volume / 100.0) * m_masterVolume);

    if (QFileInfo::exists(customSoundPath)) {
        player->setSource(QUrl::fromLocalFile(customSoundPath));
    } else {
        delete player;
        return;
    }

    player->play();

    connect(player, &QMediaPlayer::playbackStateChanged,
            player, &QMediaPlayer::deleteLater);
}

void SoundManager::setVolume(int volume)
{
    m_volume = qBound(0, volume, 100);

    qreal volumeLevel = m_muted ? 0.0 : (m_volume / 100.0) * m_masterVolume;

    for (auto player : m_players) {
        if (player && player->audioOutput()) {
            player->audioOutput()->setVolume(volumeLevel);
        }
    }

    emit volumeChanged(m_volume);
}

int SoundManager::volume() const
{
    return m_volume;
}

void SoundManager::setMuted(bool muted)
{
    if (m_muted == muted) return;

    m_muted = muted;

    qreal volumeLevel = m_muted ? 0.0 : (m_volume / 100.0) * m_masterVolume;

    for (auto player : m_players) {
        if (player && player->audioOutput()) {
            player->audioOutput()->setVolume(volumeLevel);
        }
    }

    emit mutedChanged(m_muted);
}

bool SoundManager::isMuted() const
{
    return m_muted;
}

void SoundManager::setEnabled(bool enabled)
{
    m_enabled = enabled;

    if (!enabled) {
        for (auto player : m_players) {
            if (player) {
                player->stop();
            }
        }
    }
}

bool SoundManager::isEnabled() const
{
    return m_enabled;
}

void SoundManager::preloadSounds()
{
    if (m_preloaded) return;

    for (auto it = m_soundPaths.begin(); it != m_soundPaths.end(); ++it) {
        ensurePlayerForEffect(it.key());

        QString path = it.value();
        if (!path.isEmpty() && QFileInfo::exists(path)) {
            m_players[it.key()]->setSource(QUrl::fromLocalFile(path));
            m_loaded[it.key()] = true;
        }
    }

    m_preloaded = true;
    qDebug() << "SoundManager: Sounds preloaded";
}

void SoundManager::unloadSounds()
{
    for (auto player : m_players) {
        if (player) {
            player->stop();
            player->deleteLater();
        }
    }
    m_players.clear();
    m_preloaded = false;
}

void SoundManager::setMasterVolume(qreal volume)
{
    m_masterVolume = qBound(0.0, volume, 1.0);

    qreal volumeLevel = m_muted ? 0.0 : (m_volume / 100.0) * m_masterVolume;

    for (auto player : m_players) {
        if (player && player->audioOutput()) {
            player->audioOutput()->setVolume(volumeLevel);
        }
    }
}

qreal SoundManager::masterVolume() const
{
    return m_masterVolume;
}

QString SoundManager::getSoundPath(SoundEffect effect)
{
    return m_soundPaths.value(effect, "");
}
