#include "batchcopytraindialog.h"
#include "util/buttongroup.hpp"

#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QSet>
#include <QSpinBox>
#include <QTimeEdit>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include "data/common/qesystem.h"
#include "data/diagram/diagram.h"
#include "model/delegate/traintimedelegate.h"
#include "util/utilfunc.h"
#include "data/train/train.h"

BatchCopyTrainModel::BatchCopyTrainModel(QObject* parent) :
    QEMoveableModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({ tr("车次"),tr("首站时刻") });
}

void BatchCopyTrainModel::addRow(const QString& trainName, const QTime& time)
{
    int row = rowCount();
    insertRow(row);
    setupNewRow(row);
    item(row, ColTrainName)->setText(trainName);
    item(row, ColStartTime)->setData(time, Qt::EditRole);
}

void BatchCopyTrainModel::setupNewRow(int row)
{
    setItem(row, ColTrainName, new QStandardItem);
    auto* it = new QStandardItem;
    it->setData(QTime(), Qt::EditRole);
    setItem(row, ColStartTime, it);
}

BatchCopyTrainDialog::BatchCopyTrainDialog(Diagram& diagram_,
    std::shared_ptr<Train> train_, QWidget* parent) :
    QDialog(parent),diagram(diagram_), coll(diagram.trainCollection()),
    train(train_), model(new BatchCopyTrainModel(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(600, 800);
    setWindowTitle(tr("批量复制运行线"));
    initUI();
}

void BatchCopyTrainDialog::initUI()
{
    auto* vlay = new QVBoxLayout(this);
    auto* flay = new QFormLayout;
    cbTrain = new SelectTrainCombo(coll);
    flay->addRow(tr("原型车次"), cbTrain);
    edRef = new QLineEdit;
    edRef->setFocusPolicy(Qt::NoFocus);
    flay->addRow(tr("起始站"), edRef);
    connect(cbTrain, &SelectTrainCombo::currentTrainChanged,
        this, &BatchCopyTrainDialog::setTrain);
    vlay->addLayout(flay);

    auto* box = new QGroupBox(tr("生成阵列"));
    flay = new QFormLayout;
    edFormat = new QLineEdit;
    edFormat->setToolTip(tr("生成车次阵列的格式，采用[%1]表示待填充的数字。例如G712%1。"));
    flay->addRow(tr("车次格式"), edFormat);
    spStartNumber = new QSpinBox;
    spStartNumber->setRange(0, 100000000);
    flay->addRow(tr("起始编号"), spStartNumber);

    edStartTime = new QTimeEdit;
    edStartTime->setToolTip(tr("阵列中第一个车次的首站时刻。"));
    edStartTime->setWrapping(true);
    edStartTime->setDisplayFormat("hh:mm:ss");
    flay->addRow(tr("起始时刻"), edStartTime);

    auto* chlay = new QHBoxLayout;
    spMin = new QSpinBox;
    spMin->setRange(0, 1000000000);
    spMin->setSuffix(tr(" 分 (min)"));
    spMin->setToolTip(tr("相邻两车次的首站时刻间距，即发车间隔。"));
    chlay->addWidget(spMin);
    spSec = new QSpinBox;
    spSec->setRange(0, 59);
    spSec->setSingleStep(10);
    spSec->setSuffix(tr(" 秒 (s)"));
    spSec->setWrapping(true);
    chlay->addWidget(spSec);
    flay->addRow(tr("间隔时间"), chlay);

    spCount = new QSpinBox;
    spCount->setValue(1);
    spCount->setRange(1, 1000000);
    spCount->setToolTip(tr("要生成的车次数量"));
    flay->addRow(tr("阵列数量"), spCount);

    auto* hlay = new QHBoxLayout;
    hlay->addLayout(flay);
    auto* b = new ButtonGroup<2, QVBoxLayout>({ "生成","说明" });
    b->connectAll(SIGNAL(clicked()), this, { SLOT(onGenerateArray()),SLOT(onShowHelp()) });
    hlay->addLayout(b);

    box->setLayout(hlay);
    vlay->addWidget(box);

    cbTrain->setTrain(train);    //导致edRef变化

    ctab = new QEControlledTable;
    table = ctab->table();
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setEditTriggers(QTableView::AllEditTriggers);
    table->setModel(model);
    table->setItemDelegateForColumn(BatchCopyTrainModel::ColStartTime,
        new TrainTimeDelegate(diagram.options(), this));
    vlay->addWidget(ctab);

    auto* g = new ButtonGroup<2>({ "应用","关闭" });
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()), this, { SLOT(onApply()),SLOT(close()) });
}

void BatchCopyTrainDialog::setTrain(std::shared_ptr<Train> t)
{
    if (t && !t->empty()) {
        train = t;
        auto st = train->firstStation();
        edRef->setText(tr("%1  %2").arg(st->name.toSingleLiteral(),
            st->timeStringCompressed()));
        edStartTime->setTime(st->arrive);
    }
}

void BatchCopyTrainDialog::onApply()
{
    if (!train || train->empty()) {
        QMessageBox::warning(this, tr("错误"),
            tr("请先选择一个时刻表非空的原型列车！"));
        return;
    }
    const QTime& reftime = train->firstStation()->arrive;

    QVector<std::shared_ptr<Train>> trains;
    QList<int> invalidRows;
    QSet<QString> names;
    for (int i = 0; i < model->rowCount(); i++) {
        const QString& name = model->item(i, BatchCopyTrainModel::ColTrainName)->text();
        if (coll.trainNameIsValid(name, {}) && !names.contains(name)) {
            const QTime& tm = model->item(i, BatchCopyTrainModel::ColStartTime)->
                data(Qt::EditRole).toTime();
            auto nt = std::make_shared<Train>(train->translation(name,
                qeutil::secsTo(reftime, tm), diagram.options().period_hours));
            diagram.updateTrain(nt);
            trains.append(nt);
            names.insert(name);
        }
        else {
            invalidRows.append(i);
        }
    }

    QString invalidReport;
    if (!invalidRows.isEmpty()) {
        invalidReport = tr("\n有以下数据因为车次无效（为空或与既有冲突）未能添加：");
        for (auto i : invalidRows) {
            invalidReport.append(tr("\n第%1行, 车次[%2]").arg(i)
                .arg(model->item(i, BatchCopyTrainModel::ColTrainName)->text()));
        }
    }

    if (trains.isEmpty()) {
        QMessageBox::information(this, tr("信息"), tr("没有有效的车次被复制")
            + invalidReport);
    }
    else {
        emit applied(trains);
        QMessageBox::information(this, tr("信息"), tr("成功添加%1个车次%2")
            .arg(trains.size()).arg(invalidReport));
    }
}

void BatchCopyTrainDialog::onGenerateArray()
{
    auto&& fmt = edFormat->text();
    int num = spStartNumber->value();
    int count = spCount->value();
    QTime tm = edStartTime->time();
    int ds = spMin->value() * 60 + spSec->value();
    for (int i = 0; i < count; i++) {
        model->addRow(fmt.arg(num), tm);
        tm = tm.addSecs(ds);
        num += 2;
    }
}

void BatchCopyTrainDialog::onShowHelp()
{
    auto&& s = tr("车次阵列功能允许按照一定的格式生成一组车次名称以及对应的" 
        "始发时刻。在[车次格式]一栏中以“%1”替代待填充的数字，" 
        "以[起始编号]为第一个编号，每次递增2，创建一组车次名称，" 
        "在[间隔时间]设置相邻两车次的发车间隔。\n" 
        "例如，格式“G711%1”，起始编号3，起始时刻7:00，间隔60分钟，" 
        "阵列数量2，将得到G7113和G7115两个车次，始发时刻分别是8:00和9:00。\n" 
        "点击[生成]在下方表格中产生这些车次以及首站时刻。"
        "请注意阵列功能只是个快速产生车次的工具，" 
        "并不会直接提交结果。");
    QMessageBox::information(this, tr("操作提示"), s);
}







