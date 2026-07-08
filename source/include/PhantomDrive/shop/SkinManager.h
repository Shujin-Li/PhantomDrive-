#pragma once

#include "PhantomDrive_global.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>

namespace PhantomDrive {

class CurrencyManager;

struct SkinInfo {
    QString id;
    QString name;
    int price;
    bool unlocked;

    SkinInfo()
        : price(0)
        , unlocked(false)
    {}

    SkinInfo(const QString& skinId, const QString& skinName, int skinPrice, bool skinUnlocked = false)
        : id(skinId)
        , name(skinName)
        , price(skinPrice)
        , unlocked(skinUnlocked)
    {}
};

class PHANTOMDRIVE_EXPORT SkinManager : public QObject
{
    Q_OBJECT

public:
    explicit SkinManager(QObject* parent = nullptr);

    QList<SkinInfo> skins() const;
    bool hasSkin(const QString& skinId) const;
    bool isUnlocked(const QString& skinId) const;
    bool purchaseSkin(const QString& skinId, CurrencyManager& currency);
    bool selectSkin(const QString& skinId);
    QString currentSkinId() const;
    QStringList unlockedSkinIds() const;
    void setUnlockedSkinIds(const QStringList& ids);
    void setCurrentSkinId(const QString& skinId);

signals:
    void skinsChanged();
    void currentSkinChanged(const QString& skinId);

private:
    QList<SkinInfo> m_catalog;
    QStringList m_unlockedSkinIds;
    QString m_currentSkinId;
};

} // namespace PhantomDrive
