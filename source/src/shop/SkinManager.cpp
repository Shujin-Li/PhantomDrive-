#include "shop/SkinManager.h"

#include "economy/CurrencyManager.h"

namespace PhantomDrive {

namespace {

QString defaultSkinId()
{
    return QStringLiteral("default");
}

} // namespace

SkinManager::SkinManager(QObject* parent)
    : QObject(parent)
    , m_catalog{
        SkinInfo(QStringLiteral("default"), QStringLiteral("Default Car"), 0, true),
        SkinInfo(QStringLiteral("blue"), QStringLiteral("Blue Racer"), 100, false),
        SkinInfo(QStringLiteral("aurora"), QStringLiteral("Aurora Flux"), 220, false),
        SkinInfo(QStringLiteral("neon"), QStringLiteral("Neon Phantom"), 300, false),
        SkinInfo(QStringLiteral("violet"), QStringLiteral("Violet Surge"), 420, false),
        SkinInfo(QStringLiteral("splitfire"), QStringLiteral("Splitfire GT"), 520, false),
        SkinInfo(QStringLiteral("gold"), QStringLiteral("Golden Phantom"), 650, false)
    }
    , m_unlockedSkinIds{defaultSkinId()}
    , m_currentSkinId(defaultSkinId())
{
}

QList<SkinInfo> SkinManager::skins() const
{
    QList<SkinInfo> result;
    result.reserve(m_catalog.size());
    for (const SkinInfo& baseInfo : m_catalog) {
        SkinInfo info = baseInfo;
        info.unlocked = m_unlockedSkinIds.contains(info.id);
        result.append(info);
    }
    return result;
}

bool SkinManager::hasSkin(const QString& skinId) const
{
    for (const SkinInfo& info : m_catalog) {
        if (info.id == skinId) {
            return true;
        }
    }
    return false;
}

bool SkinManager::isUnlocked(const QString& skinId) const
{
    return m_unlockedSkinIds.contains(skinId);
}

bool SkinManager::purchaseSkin(const QString& skinId, CurrencyManager& currency)
{
    if (!hasSkin(skinId)) {
        return false;
    }
    if (isUnlocked(skinId)) {
        return true;
    }

    int price = 0;
    for (const SkinInfo& info : m_catalog) {
        if (info.id == skinId) {
            price = info.price;
            break;
        }
    }

    if (!currency.spendCoins(price)) {
        return false;
    }

    m_unlockedSkinIds.append(skinId);
    emit skinsChanged();
    return true;
}

bool SkinManager::selectSkin(const QString& skinId)
{
    if (!hasSkin(skinId) || !isUnlocked(skinId)) {
        return false;
    }
    if (m_currentSkinId == skinId) {
        return true;
    }

    m_currentSkinId = skinId;
    emit currentSkinChanged(m_currentSkinId);
    return true;
}

QString SkinManager::currentSkinId() const
{
    return m_currentSkinId;
}

QStringList SkinManager::unlockedSkinIds() const
{
    return m_unlockedSkinIds;
}

void SkinManager::setUnlockedSkinIds(const QStringList& ids)
{
    QStringList normalizedIds;
    for (const SkinInfo& info : m_catalog) {
        if (ids.contains(info.id) && !normalizedIds.contains(info.id)) {
            normalizedIds.append(info.id);
        }
    }

    if (!normalizedIds.contains(defaultSkinId())) {
        normalizedIds.prepend(defaultSkinId());
    }

    if (m_unlockedSkinIds == normalizedIds) {
        if (!isUnlocked(m_currentSkinId)) {
            setCurrentSkinId(defaultSkinId());
        }
        return;
    }

    m_unlockedSkinIds = normalizedIds;
    if (!isUnlocked(m_currentSkinId)) {
        m_currentSkinId = defaultSkinId();
        emit currentSkinChanged(m_currentSkinId);
    }
    emit skinsChanged();
}

void SkinManager::setCurrentSkinId(const QString& skinId)
{
    if (!hasSkin(skinId) || !isUnlocked(skinId)) {
        if (m_currentSkinId == defaultSkinId()) {
            return;
        }
        m_currentSkinId = defaultSkinId();
        emit currentSkinChanged(m_currentSkinId);
        return;
    }

    if (m_currentSkinId == skinId) {
        return;
    }

    m_currentSkinId = skinId;
    emit currentSkinChanged(m_currentSkinId);
}

} // namespace PhantomDrive
