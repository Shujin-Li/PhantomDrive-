#ifndef GAME_TOP_BAR_H
#define GAME_TOP_BAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

namespace PhantomDrive {

#include "PhantomDrive_global.h"

// Floating mode-title bar — overlay at top of game page.
// Shows the current game mode name and an EXIT GAME button.
class PHANTOMDRIVE_EXPORT GameTopBar : public QWidget
{
    Q_OBJECT
public:
    explicit GameTopBar(QWidget* parent = nullptr);
    void setMode(const QString& mode);   // "Arcade", "Learning", "Custom Track"
    void showFinishButton(bool show);    // no-op (button removed from here)

signals:
    void exitGameClicked();

private:
    QLabel*    m_modeLabel;
    QPushButton* m_exitBtn;
};

} // namespace PhantomDrive

#endif // GAME_TOP_BAR_H
