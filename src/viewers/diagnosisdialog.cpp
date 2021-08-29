#include "diagnosisdialog.h"
#include "data/diagram/diagram.h"
#include "data/common/qesystem.h"

#include <QtWidgets>
#include "util/buttongroup.hpp"
#include "util/selecttraincombo.h"

DiagnosisModel::DiagnosisModel(Diagram &diagram_, QObject *parent):
    QStandardItemModel(parent), diagram(diagram_)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({tr("车次"),tr("线路"),tr("位置"), tr("等级"),tr("类型"),tr("描述")});
}

void DiagnosisModel::setupModel()
{
    using SI = QStandardItem;
    setRowCount(lst.size());
    for (int i = 0; i < lst.size(); i++) {
        const auto& iss = lst.at(i);
        auto train = iss.line->train();
        setItem(i, ColTrainName, new SI(train->trainName().full()));
        setItem(i, ColRailway, new SI(iss.line->railway().name()));
        setItem(i, ColPos, new SI(iss.posString()));
        setItem(i, ColLevel, new SI(qeutil::diagnoLevelString(iss.level)));
        setItem(i, ColType, new SI(qeutil::diagnoTypeString(iss.type)));
        setItem(i, ColDescription, new SI(iss.description));
        QColor color(Qt::black);
        switch (iss.level)
        {
        case qeutil::Information:color = Qt::black;
            break;
        case qeutil::Warning:color = Qt::blue;
            break;
        case qeutil::Error:color = Qt::red;
            break;
        default:
            break;
        }
        for (int c = 0; c < ColMAX; c++) {
            item(i, c)->setForeground(color);
        }
    }
}

void DiagnosisModel::setupForTrain(std::shared_ptr<Train> train, bool withIntMeet)
{
    lst = diagram.diagnoseTrain(*train,withIntMeet);
    setupModel();
}

void DiagnosisModel::setupForAll(bool withIntMeet)
{
    lst=diagram.diagnoseAllTrains(withIntMeet);
    setupModel();
}



DiagnosisDialog::DiagnosisDialog(Diagram& diagram_, QWidget *parent):
    QDialog(parent), diagram(diagram_),model(new DiagnosisModel(diagram_,this))
{
    setWindowTitle(tr("时刻诊断"));
    resize(800, 800);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

DiagnosisDialog::DiagnosisDialog(Diagram& diagram_, std::shared_ptr<Train> train,
    QWidget* parent) :
    QDialog(parent), diagram(diagram_), model(new DiagnosisModel(diagram_, this))
{
    setWindowTitle(tr("时刻诊断"));
    resize(800, 800);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
    cbTrain->setTrain(train);
    actApply();
}

void DiagnosisDialog::initUI()
{
    auto* vlay = new QVBoxLayout(this);
    auto* group = new QGroupBox(tr("配置"));

    auto* hlay = new QHBoxLayout;
    auto* flay = new QFormLayout;

    auto* g = new RadioButtonGroup<2, QVBoxLayout>({ "单车次","所有车次" }, this);
    rdSingle = static_cast<QRadioButton*>(g->get(0));
    flay->addRow(tr("范围"), g);
    rdSingle->setChecked(true);
    connect(rdSingle, &QRadioButton::toggled,
        this, &DiagnosisDialog::onSingleToggled);

    cbTrain = new SelectTrainCombo(diagram.trainCollection());
    flay->addRow(tr("选择车次"), cbTrain);

    ckIntMeet = new QCheckBox(tr("检测区间会车 （单线线路）"));
    flay->addRow(tr("选项"), ckIntMeet);

    hlay->addLayout(flay);
    auto* cv = new QVBoxLayout;
    auto* btn = new QPushButton(tr("确定"));
    cv->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, &DiagnosisDialog::actApply);

    btn = new QPushButton(tr("说明"));
    connect(btn, &QPushButton::clicked, this, &DiagnosisDialog::actHelp);
    cv->addWidget(btn);
    hlay->addLayout(cv);

    group->setLayout(hlay);
    vlay->addWidget(group);

    table = new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setModel(model);
    connect(table->verticalHeader(),
        qOverload<int, Qt::SortOrder>(&QHeaderView::sortIndicatorChanged),
        table,
        qOverload<int, Qt::SortOrder>(&QTableView::sortByColumn));
    table->horizontalHeader()->setSortIndicatorShown(true);
    vlay->addWidget(table);
}

void DiagnosisDialog::actApply()
{
    if (rdSingle->isChecked()) {
        auto train = cbTrain->train();
        if (!train) {
            QMessageBox::warning(this, tr("错误"), tr("请先选择车次!"));
            return;
        }
        model->setupForTrain(train, ckIntMeet->isChecked());
    }
    else {
        model->setupForAll(ckIntMeet->isChecked());
    }
    if (model->rowCount())
        QMessageBox::information(this, tr("提示"), tr("当前所选范围内未发现问题。"));
    else
        table->resizeColumnsToContents();
}

void DiagnosisDialog::onSingleToggled(bool on)
{
    cbTrain->setEnabled(on);
}

void DiagnosisDialog::actHelp()
{
    auto&& text = tr("此功能基于列车事件表算法，对列车时刻表中可能存在的错误"
        "情况进行推断和提示。目前支持的错误类型有：\n"
        "1. 区间越行（让行、共线）：两同向运行线在非图定站内交叉。\n"
        "2. 区间会车（可选）：两对向运行线在非图定站内交叉。\n"
        "3. 天窗冲突：区间运行线与（同方向的）天窗发生冲突。\n"
        "4. 停车时间过长：图定站内停车时间过长。若超过12小时，提示可能造成"
        "事件表排序出错；若超过20小时，提示可能出现到发时刻填反的情况。\n"
        "5. 区间运行时间过长：同一运行线上，两相邻的区间车站之间运行时间过长。"
        "若超过12小时，提示可能导致事件表排序出错；若超过20小时，提示可能出现时刻表错排问题。\n"
        "提示内容仅供参考。"
    );
    QMessageBox::information(this, tr("提示"), text);
}
