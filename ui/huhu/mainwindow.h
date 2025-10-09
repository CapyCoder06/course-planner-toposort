#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include "PlannerService.h"   // ✅ PlanResult, Specialization

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnOpenJson_clicked();        // nút "Open JSON"
    void on_btnBuildPlan_clicked();       // nút "Build Plan"

private:
    Ui::MainWindow *ui;
    PlannerService planner_;              // engine

    Specialization currentSpec() const;   // đọc từ combobox
    void renderPlan(const PlanResult& r); // vẽ 8 bảng

    // helper hiển thị prerequisite
    QString prereqStringFor(const QString& courseId) const;
};
