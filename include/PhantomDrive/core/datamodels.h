#ifndef DATAMODELS_H
#define DATAMODELS_H

#include <QString>
#include <QDateTime>
#include <QList>

// 重导出 ScoreReport 以保持向后兼容
// 新代码应直接使用 PhantomDrive::ScoreReport
#include "PhantomDrive/scoring/ScoreReport.h"

// 保持向后兼容的别名
namespace PhantomDrive {
using PracticeReport = ScoreReport;
}

// 全局命名空间的旧名称也可用（但推荐使用 PhantomDrive::ScoreReport）
typedef PhantomDrive::ScoreReport PracticeReport;

#endif // DATAMODELS_H
