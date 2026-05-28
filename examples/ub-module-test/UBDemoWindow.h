#ifndef UBDEMOWINDOW_H
#define UBDEMOWINDOW_H

#include <QWidget>

namespace PhantomDrive {

class ArcadeHUD;

class UBDemoWindow : public QWidget
{
    Q_OBJECT
public:
    explicit UBDemoWindow();
    ~UBDemoWindow();

private slots:
    void onPositiveClicked();
    void onWarningClicked();
    void onCriticalClicked();
    void onPowerupClicked();
    void onMilestoneClicked();
    void onCountdownClicked();
    void onBeepClicked();
    void onCollisionClicked();
    void onViolationClicked();
    void onLapCompleteClicked();
    void onShowHUDClicked();
    void onTestHUDClicked();

private:
    void setupUI();
    ArcadeHUD* m_arcadeHUD;
};

} // namespace PhantomDrive

#endif // UBDEMOWINDOW_H
