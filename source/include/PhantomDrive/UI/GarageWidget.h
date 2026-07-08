#pragma once

#include "PhantomDrive_global.h"
#include "shop/SkinManager.h"

#include <QObject>
#include <QWidget>

class QLabel;
class QVBoxLayout;

namespace PhantomDrive {

class PHANTOMDRIVE_EXPORT GarageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GarageWidget(QWidget* parent = nullptr);

    void setGarageState(const QList<SkinInfo>& skins,
                        const QString& currentSkinId,
                        int coins);

signals:
    void backRequested();
    void purchaseRequested(const QString& skinId);
    void selectRequested(const QString& skinId);

private:
    QLabel* m_titleLabel;
    QLabel* m_coinLabel;
    QVBoxLayout* m_skinListLayout;
};

} // namespace PhantomDrive
