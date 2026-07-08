#pragma once

#include "PhantomDrive_global.h"

#include <QObject>
#include <QString>
#include <QStringList>

namespace PhantomDrive {

struct PlayerProfile {
    int coins;
    QStringList ownedSkins;
    QString currentSkin;

    PlayerProfile()
        : coins(0)
        , ownedSkins{QStringLiteral("default")}
        , currentSkin(QStringLiteral("default"))
    {}
};

class PHANTOMDRIVE_EXPORT PlayerProfileStore : public QObject
{
    Q_OBJECT

public:
    explicit PlayerProfileStore(QObject* parent = nullptr);

    PlayerProfile loadProfile() const;
    bool saveProfile(const PlayerProfile& profile) const;
    QString profileFilePath() const;

    static PlayerProfile defaultProfile();

private:
    PlayerProfile sanitizeProfile(const PlayerProfile& profile) const;
};

} // namespace PhantomDrive
