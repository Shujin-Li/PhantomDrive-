#pragma once

#include "PhantomDrive_global.h"

#include <QObject>
#include <QPixmap>
#include <QString>
#include <QWidget>

class QPushButton;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;

namespace PhantomDrive {

class MenuCardWidget;

class PHANTOMDRIVE_EXPORT StartGameOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit StartGameOverlay(QWidget* parent = nullptr);

    void setCoinCount(int coins);
    void setReferenceBackground(const QString& path);
    bool hasReferenceBackground() const;

signals:
    void arcadeRequested();
    void coinChallengeRequested();
    void learningRequested();
    void aiDemoRequested();
    void backRequested();
    void guideRequested();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void relayout();

    QPixmap m_referenceBackground;
    int m_coinCount = 0;
    MenuCardWidget* m_arcadeCard = nullptr;
    MenuCardWidget* m_coinChallengeCard = nullptr;
    MenuCardWidget* m_learningCard = nullptr;
    MenuCardWidget* m_aiDemoCard = nullptr;
    QPushButton* m_backButton = nullptr;
    QPushButton* m_guideButton = nullptr;
};

} // namespace PhantomDrive
