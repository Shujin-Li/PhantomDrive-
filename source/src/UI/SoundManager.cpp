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

namespace {

qreal effectVolumeMultiplier(SoundEffect effect)
{
    switch (effect) {
    case SoundEffect::ButtonClick:
    case SoundEffect::ButtonBack:
    case SoundEffect::Pause:
    case SoundEffect::Resume:
        return 0.50;
    case SoundEffect::CountdownThree:
    case SoundEffect::CountdownTwo:
    case SoundEffect::CountdownOne:
    case SoundEffect::CountdownGo:
        return 1.00;
    case SoundEffect::RaceStart:
        return 0.85;
    case SoundEffect::Checkpoint:
    case SoundEffect::LapComplete:
    case SoundEffect::FinalLap:
    case SoundEffect::RaceFinish:
        return 0.80;
    case SoundEffect::Victory:
    case SoundEffect::Fail:
        return 0.80;
    case SoundEffect::EngineIdle:
        return 0.20;
    case SoundEffect::EngineAccel:
        return 0.25;
    case SoundEffect::EngineHighSpeed:
        return 0.28;
    case SoundEffect::Brake:
        return 0.40;
    case SoundEffect::Crash:
    case SoundEffect::Boost:
        return 0.75;
    case SoundEffect::OffRoad:
        return 0.45;
    case SoundEffect::PowerupCollect:
        return 0.70;
    case SoundEffect::CountdownBeep:
        return 0.90;
    case SoundEffect::Collision:
    case SoundEffect::SpeedBoost:
    case SoundEffect::Violation:
        return 0.85;
    }
    return 0.70;
}

SoundCategory effectCategory(SoundEffect effect)
{
    switch (effect) {
    case SoundEffect::ButtonClick:
    case SoundEffect::ButtonBack:
    case SoundEffect::Pause:
    case SoundEffect::Resume:
    case SoundEffect::Victory:
    case SoundEffect::Fail:
        return SoundCategory::UI;
    case SoundEffect::CountdownThree:
    case SoundEffect::CountdownTwo:
    case SoundEffect::CountdownOne:
    case SoundEffect::CountdownGo:
    case SoundEffect::RaceStart:
        return SoundCategory::Countdown;
    case SoundEffect::Checkpoint:
    case SoundEffect::LapComplete:
    case SoundEffect::FinalLap:
    case SoundEffect::RaceFinish:
        return SoundCategory::Race;
    case SoundEffect::EngineIdle:
    case SoundEffect::EngineAccel:
    case SoundEffect::EngineHighSpeed:
    case SoundEffect::Brake:
    case SoundEffect::Crash:
    case SoundEffect::OffRoad:
    case SoundEffect::Boost:
        return SoundCategory::Car;
    case SoundEffect::PowerupCollect:
        return SoundCategory::Item;
    case SoundEffect::CountdownBeep:
    case SoundEffect::Collision:
    case SoundEffect::SpeedBoost:
    case SoundEffect::Violation:
        return SoundCategory::Car;
    }
    return SoundCategory::UI;
}

qreal outputVolumeForEffect(SoundEffect effect, bool muted, int volume, qreal masterVolume,
                            const QMap<SoundCategory, qreal>& catVols)
{
    if (muted) return 0.0;
    const qreal base = (volume / 100.0) * masterVolume;
    const qreal cat = catVols.value(effectCategory(effect), 1.0);
    return qBound(0.0, base * effectVolumeMultiplier(effect) * cat, 1.0);
}

} // namespace

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
    m_categoryVolumes = {
        { SoundCategory::UI,       1.0 },
        { SoundCategory::Countdown, 1.0 },
        { SoundCategory::Race,    1.0 },
        { SoundCategory::Car,     1.0 },
        { SoundCategory::Item,    1.0 }
    };
    initializeSoundPaths();
}

SoundManager::~SoundManager()
{
    stopAll();
    unloadSounds();
}

void SoundManager::initializeSoundPaths()
{
    QString assetsPath = SoundGenerator::instance().getSoundsDirectory();

    m_soundPaths = {
        { SoundEffect::CountdownThree,   assetsPath + "countdown_three.wav" },
        { SoundEffect::CountdownTwo,     assetsPath + "countdown_two.wav" },
        { SoundEffect::CountdownOne,     assetsPath + "countdown_one.wav" },
        { SoundEffect::CountdownGo,      assetsPath + "countdown_go.wav" },
        { SoundEffect::ButtonClick,      assetsPath + "ui_click.wav" },
        { SoundEffect::ButtonBack,       assetsPath + "ui_back.wav" },
        { SoundEffect::RaceStart,        assetsPath + "race_start.wav" },
        { SoundEffect::Pause,            assetsPath + "pause.wav" },
        { SoundEffect::Resume,           assetsPath + "resume.wav" },
        { SoundEffect::Victory,          assetsPath + "victory.wav" },
        { SoundEffect::Fail,             assetsPath + "fail.wav" },
        { SoundEffect::EngineIdle,       assetsPath + "engine_idle.wav" },
        { SoundEffect::EngineAccel,      assetsPath + "engine_accel.wav" },
        { SoundEffect::EngineHighSpeed,  assetsPath + "engine_high.wav" },
        { SoundEffect::Brake,            assetsPath + "brake.wav" },
        { SoundEffect::Crash,            assetsPath + "crash.wav" },
        { SoundEffect::OffRoad,          assetsPath + "offroad.wav" },
        { SoundEffect::Boost,            assetsPath + "boost.wav" },
        { SoundEffect::Checkpoint,       assetsPath + "checkpoint.wav" },
        { SoundEffect::LapComplete,      assetsPath + "lap_complete.wav" },
        { SoundEffect::FinalLap,         assetsPath + "final_lap.wav" },
        { SoundEffect::RaceFinish,      assetsPath + "race_finish.wav" },
        { SoundEffect::PowerupCollect,   assetsPath + "powerup_collect.wav" },
        { SoundEffect::CountdownBeep,   assetsPath + "countdown_beep.wav" },
        { SoundEffect::Collision,       assetsPath + "collision.wav" },
        { SoundEffect::SpeedBoost,      assetsPath + "speed_boost.wav" },
        { SoundEffect::Violation,       assetsPath + "violation.wav" }
    };

    for (auto it = m_soundPaths.begin(); it != m_soundPaths.end(); ++it) {
        m_loaded[it.key()] = false;
    }
}

void SoundManager::ensurePlayerForEffect(SoundEffect effect)
{
    if (m_players.contains(effect)) return;

    QMediaPlayer* player = new QMediaPlayer(this);
    QAudioOutput* audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);

    qreal vol = outputVolumeForEffect(effect, m_muted, m_volume, m_masterVolume, m_categoryVolumes);
    audioOutput->setVolume(vol);

    m_players[effect] = player;

    connect(player, &QMediaPlayer::errorOccurred,
            this, [this, effect](QMediaPlayer::Error, const QString& errorString) {
        qWarning() << "SoundManager: error playing" << static_cast<int>(effect)
                   << ":" << errorString;
    });
}

void SoundManager::useGeneratedSound(SoundEffect effect)
{
    static QMap<SoundEffect, QString> tempFiles;

    if (tempFiles.contains(effect)) {
        QString tpath = tempFiles[effect];
        if (QFileInfo::exists(tpath)) {
            ensurePlayerForEffect(effect);
            m_players[effect]->setSource(QUrl::fromLocalFile(tpath));
            m_players[effect]->stop();
            m_players[effect]->play();
            emit soundPlayed(effect);
            return;
        }
    }

    QTemporaryFile* tempFile = new QTemporaryFile(QDir::tempPath() + "/phantom_sound_XXXXXX.wav");
    if (tempFile->open()) {
        QString tpath = tempFile->fileName();
        tempFile->close();

        QByteArray wavData = SoundGenerator::instance().generateWavFile(effect);
        QFile file(tpath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(wavData);
            file.close();

            tempFiles[effect] = tpath;
            ensurePlayerForEffect(effect);
            m_players[effect]->setSource(QUrl::fromLocalFile(tpath));
            m_players[effect]->stop();
            m_players[effect]->play();
            emit soundPlayed(effect);
        } else {
            qWarning() << "SoundManager: cannot write temp file for effect" << static_cast<int>(effect);
        }
    }
}

void SoundManager::play(SoundEffect effect)
{
    if (!m_enabled) return;

    if (m_loopPlayers.contains(effect)) {
        stopLoop(effect);
    }

    ensurePlayerForEffect(effect);
    QMediaPlayer* player = m_players[effect];
    if (!player) return;

    QString path = m_soundPaths.value(effect, "");
    if (!path.isEmpty() && QFileInfo::exists(path)) {
        player->setSource(QUrl::fromLocalFile(path));
    } else {
        useGeneratedSound(effect);
        return;
    }

    player->stop();
    player->play();
    emit soundPlayed(effect);
}

void SoundManager::play(const QString& customSoundPath)
{
    if (!m_enabled || customSoundPath.isEmpty()) return;

    if (!QFileInfo::exists(customSoundPath)) {
        qWarning() << "SoundManager: custom sound file not found:" << customSoundPath;
        return;
    }

    QMediaPlayer* player = new QMediaPlayer(this);
    QAudioOutput* audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(outputVolumeForEffect(SoundEffect::ButtonClick, m_muted, m_volume, m_masterVolume, m_categoryVolumes));
    player->setSource(QUrl::fromLocalFile(customSoundPath));
    player->play();

    connect(player, &QMediaPlayer::playbackStateChanged, player, &QMediaPlayer::deleteLater);
}

void SoundManager::playLoop(SoundEffect effect)
{
    if (!m_enabled) return;

    if (m_loopPlayers.contains(effect)) {
        m_loopPlayers[effect]->play();
        return;
    }

    QMediaPlayer* player = new QMediaPlayer(this);
    QAudioOutput* audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);

    qreal vol = outputVolumeForEffect(effect, m_muted, m_volume, m_masterVolume, m_categoryVolumes);
    audioOutput->setVolume(vol);

    QString path = m_soundPaths.value(effect, "");
    if (!path.isEmpty() && QFileInfo::exists(path)) {
        player->setSource(QUrl::fromLocalFile(path));
        player->setLoops(QMediaPlayer::Infinite);
        player->play();
    } else {
        QTemporaryFile* tempFile = new QTemporaryFile(QDir::tempPath() + "/phantom_loop_XXXXXX.wav");
        if (tempFile->open()) {
            QString tpath = tempFile->fileName();
            tempFile->close();
            QByteArray wavData = SoundGenerator::instance().generateWavFile(effect);
            QFile file(tpath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(wavData);
                file.close();
                player->setSource(QUrl::fromLocalFile(tpath));
                player->setLoops(QMediaPlayer::Infinite);
                player->play();
            }
        }
    }

    m_loopPlayers[effect] = player;
}

void SoundManager::stopLoop(SoundEffect effect)
{
    if (m_loopPlayers.contains(effect)) {
        m_loopPlayers[effect]->stop();
        m_loopPlayers[effect]->deleteLater();
        m_loopPlayers.remove(effect);
    }
}

void SoundManager::stopAll()
{
    for (auto it = m_loopPlayers.begin(); it != m_loopPlayers.end(); ++it) {
        it.value()->stop();
        it.value()->deleteLater();
    }
    m_loopPlayers.clear();

    for (auto player : m_players) {
        if (player) player->stop();
    }
}

void SoundManager::setVolume(int volume)
{
    m_volume = qBound(0, volume, 100);

    for (auto it = m_players.begin(); it != m_players.end(); ++it) {
        QMediaPlayer* player = it.value();
        if (player && player->audioOutput()) {
            player->audioOutput()->setVolume(
                outputVolumeForEffect(it.key(), m_muted, m_volume, m_masterVolume, m_categoryVolumes));
        }
    }
    for (auto it = m_loopPlayers.begin(); it != m_loopPlayers.end(); ++it) {
        QMediaPlayer* player = it.value();
        if (player && player->audioOutput()) {
            player->audioOutput()->setVolume(
                outputVolumeForEffect(it.key(), m_muted, m_volume, m_masterVolume, m_categoryVolumes));
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

    auto applyMute = [this](QMediaPlayer* player, SoundEffect effect) {
        if (player && player->audioOutput()) {
            player->audioOutput()->setVolume(
                outputVolumeForEffect(effect, m_muted, m_volume, m_masterVolume, m_categoryVolumes));
        }
    };

    for (auto it = m_players.begin(); it != m_players.end(); ++it)
        applyMute(it.value(), it.key());
    for (auto it = m_loopPlayers.begin(); it != m_loopPlayers.end(); ++it)
        applyMute(it.value(), it.key());

    emit mutedChanged(m_muted);
}

bool SoundManager::isMuted() const { return m_muted; }

void SoundManager::setEnabled(bool enabled)
{
    m_enabled = enabled;
    if (!enabled) stopAll();
}

bool SoundManager::isEnabled() const { return m_enabled; }

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
}

void SoundManager::unloadSounds()
{
    for (auto player : m_players) {
        if (player) { player->stop(); player->deleteLater(); }
    }
    m_players.clear();
    m_preloaded = false;
}

void SoundManager::setMasterVolume(qreal volume)
{
    m_masterVolume = qBound(0.0, volume, 1.0);
    setVolume(m_volume);
}

qreal SoundManager::masterVolume() const { return m_masterVolume; }

void SoundManager::setCategoryVolume(SoundCategory cat, qreal vol)
{
    m_categoryVolumes[cat] = qBound(0.0, vol, 1.0);
    setVolume(m_volume);
}

qreal SoundManager::categoryVolume(SoundCategory cat) const
{
    return m_categoryVolumes.value(cat, 1.0);
}

qreal SoundManager::effectVolumeMultiplier(SoundEffect effect) const
{
    Q_UNUSED(effect);
    return ::effectVolumeMultiplier(effect);
}

SoundCategory SoundManager::effectCategory(SoundEffect effect) const
{
    return ::effectCategory(effect);
}

QString SoundManager::getSoundPath(SoundEffect effect)
{
    return m_soundPaths.value(effect, "");
}
