#pragma once

#include "PhantomDrive_global.h"

#include <QObject>

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT CurrencyManager : public QObject
{
    Q_OBJECT

public:
    explicit CurrencyManager(QObject* parent = nullptr);

    int coins() const;
    void setCoins(int amount);
    void addCoins(int amount);
    bool canAfford(int amount) const;
    bool spendCoins(int amount);

signals:
    void coinsChanged(int coins);

private:
    int m_coins;
};

} // namespace PhantomDrive
