#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QDir>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    ui->comboSpec->clear();
    ui->comboSpec->addItem("SE - Software Engineering",    int(Specialization::SE));
    ui->comboSpec->addItem("NNS - Network & Security",     int(Specialization::NNS));
    ui->comboSpec->addItem("IS - Information Systems",     int(Specialization::IS));
    ui->comboSpec->addItem("AI - Artificial Intelligence", int(Specialization::AI));
}

MainWindow::~MainWindow() { delete ui; }

Specialization MainWindow::currentSpec() const {
    switch (ui->comboSpec->currentData().toInt()) {
        case int(Specialization::SE):  return Specialization::SE;
        case int(Specialization::NNS): return Specialization::NNS;
        case int(Specialization::IS):  return Specialization::IS;
        case int(Specialization::AI):  return Specialization::AI;
        default:                       return Specialization::SE;
    }
}

void MainWindow::on_btnOpenJson_clicked() {
    QString path = QFileDialog::getOpenFileName(
        this, tr("Open Curriculum JSON"), QDir::currentPath(),
        tr("JSON Files (*.json)")
    );
    if (path.isEmpty()) return;

    std::string err;
    if (!planner_.loadCurriculum(path.toStdString(), err)) {
        QMessageBox::critical(this, tr("Error"), QString::fromStdString(err));
        return;
    }
    QMessageBox::information(this, tr("Loaded"), tr("Curriculum loaded successfully."));
}

void MainWindow::on_btnBuildPlan_clicked() {
    auto spec = currentSpec();
    auto res = planner_.buildPlan(spec, /*maxCreditsPerTerm*/ 28);
    renderPlan(res);
}

QString MainWindow::prereqStringFor(const QString& courseId) const {
    QString s;
    auto v = planner_.prereqsOf(courseId.toStdString());
    for (size_t i = 0; i < v.size(); ++i) {
        s += QString::fromStdString(v[i]);
        if (i + 1 < v.size()) s += ", ";
    }
    return s;
}

static QWidget* makeTermTable(QWidget* parent,
                              const PlannedTerm& term,
                              const std::function<QString(const QString&)>& prereqProvider) {
    auto* w = new QTableWidget(parent);
    w->setColumnCount(4);
    QStringList headers; headers << "ID" << "Tên môn" << "Tín chỉ" << "Prerequisite";
    w->setHorizontalHeaderLabels(headers);
    w->verticalHeader()->setVisible(false);
    w->setEditTriggers(QAbstractItemView::NoEditTriggers);
    w->setSelectionMode(QAbstractItemView::NoSelection);

    int r = 0;
    w->setRowCount(int(term.courses.size()) + 1); // +1 cho dòng tổng

    for (const auto& c : term.courses) {
        const QString qid = QString::fromStdString(c.id);
        w->setItem(r,0,new QTableWidgetItem(qid));
        w->setItem(r,1,new QTableWidgetItem(QString::fromStdString(c.name)));
        w->setItem(r,2,new QTableWidgetItem(QString::number(c.credits)));
        w->setItem(r,3,new QTableWidgetItem(prereqProvider(qid)));
        ++r;
    }

    // Total row
    auto* totalLbl = new QTableWidgetItem("Tổng tín chỉ: ");
    totalLbl->setFlags(totalLbl->flags() & ~Qt::ItemIsEditable);
    totalLbl->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    w->setItem(r, 1, totalLbl);
    w->setItem(r, 2, new QTableWidgetItem(QString::number(term.totalCredits)));

    w->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    w->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    w->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    w->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    return w;
}

void MainWindow::renderPlan(const PlanResult& r) {
    ui->tabs->clear();

    if (!r.ok) {
        QMessageBox::warning(this, tr("Plan failed"),
                             QString::fromStdString(r.message));
        return;
    }

    for (const auto& term : r.terms) {
        auto* table = makeTermTable(
            this, term,
            [this](const QString& id){ return this->prereqStringFor(id); }
        );
        ui->tabs->addTab(table, QString("Kỳ %1").arg(term.index));
    }
}
