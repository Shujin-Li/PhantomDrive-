#include "economy/CurrencyManager.h"

#include <QtGlobal>

namespace PhantomDrive {

CurrencyManager::CurrencyManager(QObject* parent)
    : QObject(parent)
    , m_coins(0)
{
}

int CurrencyManager::coins() const
{
    return m_coins;
}

void CurrencyManager::setCoins(int amount)
{
    const int safeAmount = qMax(0, amount);
    if (m_coins == safeAmount) {
        return;
    }

    m_coins = safeAmount;
    emit coinsChanged(m_coins);
}

void CurrencyManager::addCoins(int amount)
{
    if (amount <= 0) {
        return;
    }

    setCoins(m_coins + amount);
}

bool CurrencyManager::canAfford(int amount) const
{
    if (amount <= 0) {
        return true;
    }
    return m_coins >= amount;
}

bool CurrencyManager::spendCoins(int amount)
{
    if (amount <= 0) {
        return true;
    }
    if (!canAfford(amount)) {
        return false;
    }

    setCoins(m_coins - amount);
    return true;
}

} // namespace PhantomDrive
