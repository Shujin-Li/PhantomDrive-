#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <PhantomDrive/core/saveloadmanager.h>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "========================================";
    qDebug() << "     SaveLoadManager 存档模块测试";
    qDebug() << "========================================";

    // ========== 1. 测试保存 ==========
    qDebug() << "\n【测试1】保存记录";

    PhantomDrive::ScoreReport report1;
    report1.sessionId = "session_001";
    report1.vehicleId = "car_01";
    report1.generatedAt = QDateTime::currentDateTime();
    report1.totalScore = 95.5;
    report1.grade = "A";
    report1.summary = "第一次测试驾驶记录";

    if (SaveLoadManager::instance().saveReport(report1)) {
        qDebug() << "  ✅ 保存成功！";
    }
    else {
        qDebug() << "  ❌ 保存失败！";
    }

    // ========== 2. 测试保存第二条 ==========
    qDebug() << "\n【测试2】再保存一条记录";

    PhantomDrive::ScoreReport report2;
    report2.sessionId = "session_002";
    report2.vehicleId = "car_01";
    report2.generatedAt = QDateTime::currentDateTime();
    report2.totalScore = 88.0;
    report2.grade = "B";
    report2.summary = "第二次测试驾驶记录";

    if (SaveLoadManager::instance().saveReport(report2)) {
        qDebug() << "  ✅ 保存成功！";
    }
    else {
        qDebug() << "  ❌ 保存失败！";
    }

    // ========== 3. 测试加载所有记录 ==========
    qDebug() << "\n【测试3】加载所有历史记录";

    auto history = SaveLoadManager::instance().loadHistory();
    qDebug() << "  📋 共加载了" << history.size() << "条记录";

    for (int i = 0; i < history.size(); i++) {
        const auto& r = history[i];
        qDebug() << "    [" << i << "] sessionId:" << r.sessionId;
        qDebug() << "        总分:" << r.totalScore;
        qDebug() << "        等级:" << r.grade;
        qDebug() << "        总结:" << r.summary;
        qDebug() << "        时间:" << r.generatedAt.toString("yyyy-MM-dd hh:mm:ss");
        qDebug() << "        ---";
    }

    // ========== 4. 测试删除 ==========
    if (history.size() > 0) {
        qDebug() << "\n【测试4】删除最后一条记录";

        int lastIndex = history.size() - 1;
        if (SaveLoadManager::instance().deleteReport(lastIndex)) {
            qDebug() << "  ✅ 删除成功！";
        }
        else {
            qDebug() << "  ❌ 删除失败！";
        }

        // 验证删除结果
        auto newHistory = SaveLoadManager::instance().loadHistory();
        qDebug() << "  📋 删除后还剩" << newHistory.size() << "条记录";
    }

    // ========== 5. 测试更新 ==========
    qDebug() << "\n【测试5】更新第一条记录";

    auto currentHistory = SaveLoadManager::instance().loadHistory();
    if (currentHistory.size() > 0) {
        PhantomDrive::ScoreReport updated = currentHistory[0];
        updated.totalScore = 100.0;
        updated.grade = "S";
        updated.summary = "【已更新】完美驾驶！";

        if (SaveLoadManager::instance().updateReport(0, updated)) {
            qDebug() << "  ✅ 更新成功！";
        }
        else {
            qDebug() << "  ❌ 更新失败！";
        }

        // 验证更新结果
        auto verifyHistory = SaveLoadManager::instance().loadHistory();
        if (verifyHistory.size() > 0) {
            qDebug() << "  📋 更新后的第一条记录总分:" << verifyHistory[0].totalScore;
        }
    }

    // ========== 测试完成 ==========
    qDebug() << "\n========================================";
    qDebug() << "          测试完成！";
    qDebug() << "========================================";

    return 0;
}