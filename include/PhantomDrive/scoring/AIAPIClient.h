#pragma once

#include "PhantomDrive/scoring/ScoreReport.h"

#include <QObject>

namespace PhantomDrive {

class AIAPIClient : public QObject
{
    Q_OBJECT

public:
    explicit AIAPIClient(QObject* parent = nullptr);

    QString generateCoachReport(const ScoreReport& report) const;
    QString generateShortSummary(const ScoreReport& report) const;

private:
    QString generateMockCoachReport(const ScoreReport& report,
                                    const QString& fallbackReason = QString()) const;
};

}

