#include "diagramcomparedialog.h"

#include <chrono>
#include <QCheckBox>
#include <QLabel>
#include <QAction>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <data/diagram/diadiff.h>
#include <data/train/train.h>
#include <editors/train/trainfilterselector.h>
#include <data/common/qesystem.h>
#include <data/diagram/diagram.h>
#include <viewers/traintimetableplane.h>
#include <util/dialogadapter.h>

#include "traincomparedialog.h"


DiagramCompareModel::DiagramCompareModel(QObject *parent):
    QStandardItemModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({ tr("车次"),tr("始发1"),tr("终到1"),tr("变更数"),tr("始发2"),tr("终到2") });
}

void DiagramCompareModel::refreshData()
{
    using SI = QStandardItem;
    setRowCount(diagram_diff.size());
    for(size_t i=0;i<diagram_diff.size();i++){
        auto t=diagram_diff.at(i);
        auto* nameit=new SI(t->trainName().full());
        setItem(i,ColTrainName,nameit);
        //直接一个个来
        if (t->type==TrainDifference::Unchanged){
            auto* it=new SI(t->train1->starting().toSingleLiteral());
            setItem(i,ColStarting1,it);
            it=new SI(t->train1->terminal().toSingleLiteral());
            setItem(i,ColTerminal1,it);
            setItem(i,ColDiff,new SI("0"));
            it=new SI(t->train2->starting().toSingleLiteral());
            setItem(i,ColStarting2,it);
            it=new SI(t->train2->terminal().toSingleLiteral());
            setItem(i,ColTerminal2,it);
        }else if(t->type==TrainDifference::Changed){
            nameit->setForeground(Qt::red);
            auto* it=new SI(t->train1->starting().toSingleLiteral());
            setItem(i,ColStarting1,it);
            it->setForeground(Qt::red);
            it=new SI(t->train1->terminal().toSingleLiteral());
            setItem(i,ColTerminal1,it);
            it->setForeground(Qt::red);
            setItem(i,ColDiff,new SI(QString::number(t->difference)));
            it=new SI(t->train2->starting().toSingleLiteral());
            setItem(i,ColStarting2,it);
            it->setForeground(Qt::red);
            it=new SI(t->train2->terminal().toSingleLiteral());
            setItem(i,ColTerminal2,it);
            it->setForeground(Qt::red);
        }else if(t->type==TrainDifference::NewAdded){
            nameit->setForeground(Qt::blue);
            setItem(i,ColDiff,new SI(tr("新增")));
            auto* it=new SI(t->train2->starting().toSingleLiteral());
            setItem(i,ColStarting2,it);
            it->setForeground(Qt::blue);
            it=new SI(t->train2->terminal().toSingleLiteral());
            setItem(i,ColTerminal2,it);
            it->setForeground(Qt::blue);
        }else{
            // 删除
            nameit->setForeground(Qt::darkGray);
            auto* it=new SI(t->train1->starting().toSingleLiteral());
            setItem(i,ColStarting1,it);
            it->setForeground(Qt::darkGray);
            it=new SI(t->train1->terminal().toSingleLiteral());
            setItem(i,ColTerminal1,it);
            it->setForeground(Qt::darkGray);
            setItem(i,ColDiff,new SI(tr("删除")));
        }
    }
}

void DiagramCompareModel::resetData(diagram_diff_t &&diff)
{
    diagram_diff=std::move(diff);
    refreshData();
}

DiagramCompareDialog::DiagramCompareDialog(Diagram& diagram, QWidget *parent):
    QDialog(parent), diagram(diagram), model(new DiagramCompareModel(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("运行图对比"));
    initUI();
}

void DiagramCompareDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* hlay=new QHBoxLayout;
    edFile=new QLineEdit;
    edFile->setReadOnly(true);
    hlay->addWidget(edFile);
    auto* btn=new QPushButton(tr("浏览"));
    hlay->addWidget(btn);
    connect(btn,&QPushButton::clicked,this,&DiagramCompareDialog::viewFile);
    vlay->addLayout(hlay);

    filter=new TrainFilterSelector(diagram.trainCollection(),this);

    hlay=new QHBoxLayout;
    ckLocal=new QCheckBox(tr("仅对比铺画车次"));
    hlay->addWidget(ckLocal);
    connect(ckLocal,&QCheckBox::toggled,this,&DiagramCompareDialog::onLocalToggled);
    ckChanged=new QCheckBox(tr("仅显示变化车次"));
    hlay->addWidget(ckChanged);
    connect(ckChanged,&QCheckBox::toggled,this,&DiagramCompareDialog::onChangeToggled);
    hlay->addWidget(filter);
    connect(filter,&TrainFilterSelector::filterChanged,
            this,&DiagramCompareDialog::onFilterApplied);
    vlay->addLayout(hlay);

    auto* lab=new QLabel(tr("双击，或通过右键菜单查看车次时刻对比。"
        "\n如果选择“仅对比经过本线的车次”，则这里的逻辑相当于使用“导入车次”前对比两张运行图。"
        "\n请注意如果运行图规模比较大，打开文件后将会耗费较多时间。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    table=new QTableView;
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    table->setModel(model);
    vlay->addWidget(table);
    connect(table, &QTableView::doubleClicked, this, &DiagramCompareDialog::onDoubleClicked);

    auto* act = new QAction(tr("时刻对照"), table);
    table->addAction(act);
    connect(act, &QAction::triggered, this, &DiagramCompareDialog::actTrainDiff);
    
    act = new QAction(tr("显示时刻表"), table);
    table->addAction(act);
    connect(act, &QAction::triggered, this, &DiagramCompareDialog::actTrainTimetable);
    table->setContextMenuPolicy(Qt::ActionsContextMenu);

    btn=new QPushButton(tr("关闭"));
    vlay->addWidget(btn);
    connect(btn,&QPushButton::clicked,this,&QWidget::close);
}

bool DiagramCompareDialog::loadFile(const QString &filename)
{
    Diagram dia;
    bool flag=dia.fromJson(filename);
    if (!flag){
        QMessageBox::warning(this,tr("错误"),tr("运行图文件错误或为空，无法读取。"));
        return false;
    }
    using namespace std::chrono_literals;
    auto c_start = std::chrono::system_clock::now();
    auto t=diagram.trainCollection().diffWith(dia.trainCollection());
    auto c_end = std::chrono::system_clock::now();
    model->resetData(std::move(t));
    auto c_end2 = std::chrono::system_clock::now();
    table->resizeColumnsToContents();
    auto c_end3 = std::chrono::system_clock::now();
    emit showStatus(tr("运行图对比  计算用时 %1 毫秒  整理用时 %2 毫秒  调整用时 %3 毫秒").arg((c_end - c_start) / 1ms)
        .arg((c_end2 - c_end) / 1ms).arg((c_end3-c_end2)/1ms));
    return true;
}

void DiagramCompareDialog::viewFile()
{
    QString filename=QFileDialog::getOpenFileName(this,tr("运行图对比"),{},
             tr("qETRC/pyETRC运行图文件 (*.pyetgr)\n"
                "qETRC/pyETRC车次数据库文件 (*.pyetdb)\n"
                "JSON文件 (*.json)\n"
                "所有文件 (*)"));
    if (filename.isEmpty())
        return;
    edFile->setText(filename);
    loadFile(filename);
}

void DiagramCompareDialog::onLocalToggled(bool on)
{
    Q_UNUSED(on);
    resetRowShow();
}

void DiagramCompareDialog::onChangeToggled(bool on)
{
    Q_UNUSED(on);
    resetRowShow();
}

void DiagramCompareDialog::onFilterApplied()
{
    resetRowShow();
}

void DiagramCompareDialog::openTrainDialog(std::shared_ptr<TrainDifference> diff)
{

    auto* trainDialog = new TrainCompareDialog(diff, this);
    trainDialog->resize(700, 800);
    trainDialog->show();
}

void DiagramCompareDialog::openTimetable(std::shared_ptr<TrainDifference> diff)
{
    const auto& idx = table->currentIndex();
    if (!idx.isValid()) return;
    if ((idx.column() <= DiagramCompareModel::ColDiff && diff->train1) || !diff->train2)
        showTrainTimetable(diff->train1);
    else 
        showTrainTimetable(diff->train2);
}

void DiagramCompareDialog::showTrainTimetable(std::shared_ptr<const Train> train)
{
    auto* w = new TrainTimetablePlane(diagram.options());
    w->setWindowFlag(Qt::Dialog);
    w->setTrain(train);
    w->resize(700, 800);
    auto* dia = new DialogAdapter(w, this);
    dia->show();
}

void DiagramCompareDialog::onDoubleClicked(const QModelIndex& idx)
{
    if (!idx.isValid())
        return;
    auto diff = model->diagramDiff().at(idx.row());
    if (diff->type == TrainDifference::Changed || diff->type == TrainDifference::Unchanged) {
        openTrainDialog(diff);
    }
    else {
        openTimetable(diff);
    }
}

void DiagramCompareDialog::resetRowShow()
{
    for (int i = 0; i < model->rowCount(); i++) {
        auto t = model->diagramDiff().at(i);
        
        bool flag = false;

        // 三个筛选条件为AND关系，逐一判定，
        // 都通过后为true
        do {
            // 仅本运行图列车
            if (ckLocal->isChecked() && (!t->train1 || t->train1->adapters().empty()))
                break;
            // 仅变化车次
            if (ckChanged->isChecked() && t->type == TrainDifference::Unchanged)
                break;
            // 列车筛选器
            if (t->train1 && !filter->filter()->check(t->train1))
                break;
            flag = true;
        } while (false);

        table->setRowHidden(i, !flag);
    }
}

void DiagramCompareDialog::actTrainDiff()
{
    onDoubleClicked(table->currentIndex());
}

void DiagramCompareDialog::actTrainTimetable()
{
    const auto& idx = table->currentIndex();
    if (!idx.isValid()) return;
    openTimetable(model->diagramDiff().at(idx.row()));
}
