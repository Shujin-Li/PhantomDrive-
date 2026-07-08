#include "profile/PlayerProfileStore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QtGlobal>

namespace PhantomDrive {

namespace {

QStringList knownSkinIds()
{
    return {
        QStringLiteral("default"),
        QStringLiteral("blue"),
        QStringLiteral("neon"),
        QStringLiteral("gold")
    };
}

bool isKnownSkinId(const QString& skinId)
{
    return knownSkinIds().contains(skinId);
}

} // namespace

PlayerProfileStore::PlayerProfileStore(QObject* parent)
    : QObject(parent)
{
}

PlayerProfile PlayerProfileStore::defaultProfile()
{
    return PlayerProfile();
}

PlayerProfile PlayerProfileStore::loadProfile() const
{
    QFile file(profileFilePath());
    if (!file.exists()) {
        return defaultProfile();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return defaultProfile();
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return defaultProfile();
    }

    const QJsonObject object = document.object();
    PlayerProfile profile;
    profile.coins = object.value(QStringLiteral("coins")).toInt(0);

    QStringList ownedSkins;
    const QJsonArray ownedArray = object.value(QStringLiteral("ownedSkins")).toArray();
    for (const QJsonValue& value : ownedArray) {
        const QString skinId = value.toString().trimmed();
        if (!skinId.isEmpty()) {
            ownedSkins.append(skinId);
        }
    }
    profile.ownedSkins = ownedSkins;
    profile.currentSkin = object.value(QStringLiteral("currentSkin")).toString(QStringLiteral("default"));
    return sanitizeProfile(profile);
}

bool PlayerProfileStore::saveProfile(const PlayerProfile& profile) const
{
    const PlayerProfile safeProfile = sanitizeProfile(profile);
    const QString path = profileFilePath();
    const QFileInfo fileInfo(path);
    QDir dir(fileInfo.absolutePath());
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    QJsonArray ownedArray;
    for (const QString& skinId : safeProfile.ownedSkins) {
        ownedArray.append(skinId);
    }

    QJsonObject root;
    root.insert(QStringLiteral("coins"), safeProfile.coins);
    root.insert(QStringLiteral("ownedSkins"), ownedArray);
    root.insert(QStringLiteral("currentSkin"), safeProfile.currentSkin);

    const QJsonDocument document(root);
    return file.write(document.toJson(QJsonDocument::Indented)) >= 0;
}

QString PlayerProfileStore::profileFilePath() const
{
    const QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(appDataDir).filePath(QStringLiteral("user_profile.json"));
}

PlayerProfile PlayerProfileStore::sanitizeProfile(const PlayerProfile& profile) const
{
    PlayerProfile safeProfile = profile;
    safeProfile.coins = qMax(0, safeProfile.coins);

    QStringList normalizedOwnedSkins;
    const QStringList validSkinIds = knownSkinIds();
    for (const QString& skinId : safeProfile.ownedSkins) {
        if (validSkinIds.contains(skinId) && !normalizedOwnedSkins.contains(skinId)) {
            normalizedOwnedSkins.append(skinId);
        }
    }

    if (!normalizedOwnedSkins.contains(QStringLiteral("default"))) {
        normalizedOwnedSkins.prepend(QStringLiteral("default"));
    }
    safeProfile.ownedSkins = normalizedOwnedSkins;

    if (!isKnownSkinId(safeProfile.currentSkin)
        || !safeProfile.ownedSkins.contains(safeProfile.currentSkin)) {
        safeProfile.currentSkin = QStringLiteral("default");
    }

    return safeProfile;
}

} // namespace PhantomDrive
