#include "edittrainwidget.h"

#include <model/train/timetablestdmodel.h>

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QHeaderView>
#include <QColorDialog>
#include <QMessageBox>
#include <QToolButton>
#include <data/common/qesystem.h>

#include <util/linestylecombo.h>
#include <util/qecontrolledtable.h>
#include <util/buttongroup.hpp>
#include <model/delegate/qetimedelegate.h>
#include <data/train/train.h>
#include <data/train/traintype.h>
#include <data/train/traincollection.h>
#include <data/train/routing.h>
#include <defines/icon_specs.h>
#include <util/selecttraincombo.h>

#include "timetablewidget.h"


EditTrainWidget::EditTrainWidget(TrainCollection &coll,
                                 std::shared_ptr<Train> train,
                                 QWidget *parent):
    QWidget(parent),coll(coll),_train(train),
    ctable(new TimetableWidget(false,this))
{
    initUI();
    setTrain(train);
}

void EditTrainWidget::setTrain(const std::shared_ptr<Train> &train)
{
    _train=train;
    ctable->model()->setTrain(train);

    //暂定更改/初始化Train时才更新一次TypeCombo。
    setupTypeCombo();
    refreshBasicData();
    if (train)
        setWindowTitle(tr("列车编辑 - %1").arg(train->trainName().full()));
    setEnabled(!!train);
}

void EditTrainWidget::resetTrain()
{
    setTrain(nullptr);
}

void EditTrainWidget::refreshData()
{
    refreshBasicData();
    ctable->model()->refreshData();
}

void EditTrainWidget::refreshBasicData()
{
    if(!_train) return;
    edTrainName->setText(_train->trainName().full());
    edDownName->setText(_train->trainName().down());
    edUpName->setText(_train->trainName().up());
    edStaring->setText(_train->starting().toSingleLiteral());
    edTerminal->setText(_train->terminal().toSingleLiteral());

    cbType->setCurrentText(_train->type()->name());
    ckPassenger->setCheckState(static_cast<Qt::CheckState>(_train->passenger()));

    ckAutoPen->setChecked(_train->autoPen());   // 注意这个触发后续操作
    const auto& pen=_train->pen();
    tmpColor=pen.color();
    btnColor->setText(tmpColor.name());

    spWidth->setValue(pen.widthF());
    cbLs->setCurrentIndex(static_cast<int>(pen.style()));

    // 交路部分
    if (auto rt=_train->routing().lock()){
        edRouting->setText(rt->name());
        btnToRouting->setEnabled(true);
    }else{
        btnToRouting->setEnabled(false);
    }

}

TimetableStdModel* EditTrainWidget::getModel()
{
    return ctable->model();
}

bool EditTrainWidget::isSynchronized() const
{
    return btnSync->isChecked();
}

void EditTrainWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    edTrainName=new QLineEdit;
    auto* hlay = new QHBoxLayout;
    connect(edTrainName, &QLineEdit::editingFinished,
        this, &EditTrainWidget::onTrainFullNameUpdate);
    hlay->addWidget(edTrainName);

    auto*tb = new QToolButton;
    tb->setIcon(QEICN_train_editor_change);
    tb->setToolTip(tr("切换车次\n切换当前面板所编辑的车次，请注意当前所做的修改不会保存"));
    connect(tb, &QToolButton::clicked, [this]() {
        auto train = SelectTrainCombo::dialogGetTrain(coll, this, tr("选择车次"),
            tr("请选择要切换到的车次，当前所做的修改不会保存"));
        if (train) {
            this->setTrain(train);
        }
        });
    hlay->addWidget(tb);

    tb = new QToolButton;
    btnSync = tb;
    tb->setIcon(QEICN_train_editor_sync);
    tb->setToolTip(tr("与当前车次同步\n保持当前面板所编辑的车次与所选择的车次同步。当选择新车次时，自动切换到新车次。"
        "请注意及时保存更改。"));
    tb->setCheckable(true);
    connect(tb, &QToolButton::clicked, this, &EditTrainWidget::synchronizationChanged);
    hlay->addWidget(tb);

    flay->addRow(tr("全车次"), hlay);

    hlay=new QHBoxLayout;
    edDownName=new QLineEdit;
    hlay->addWidget(edDownName);
    auto* lab=new QLabel(tr("/"));
    lab->setFixedWidth(30);
    lab->setAlignment(Qt::AlignCenter);
    edUpName=new QLineEdit;
    hlay->addWidget(lab);
    hlay->addWidget(edUpName);
    flay->addRow(tr("下行/上行"),hlay);

    hlay=new QHBoxLayout;
    edStaring=new QLineEdit;
    lab=new QLabel(tr("->"));
    lab->setFixedWidth(30);
    lab->setAlignment(Qt::AlignCenter);
    hlay->addWidget(edStaring);
    hlay->addWidget(lab);
    edTerminal=new QLineEdit;
    hlay->addWidget(edTerminal);
    flay->addRow(tr("始发终到"),hlay);

    hlay=new QHBoxLayout;
    cbType=new QComboBox;
    cbType->setEditable(true);
    cbType->setMaximumWidth(200);
    hlay->addWidget(cbType);
    ckPassenger=new QCheckBox(tr("旅客列车"));
    ckPassenger->setTristate(true);
    hlay->addWidget(ckPassenger);
    flay->addRow(tr("列车种类"),hlay);

    hlay=new QHBoxLayout;
    ckAutoPen=new QCheckBox(tr("自动"));
    hlay->addWidget(ckAutoPen);
    connect(ckAutoPen,&QCheckBox::toggled,this,&EditTrainWidget::onAutoPenToggled);

    btnColor=new QPushButton;
    hlay->addWidget(btnColor);
    connect(btnColor, &QPushButton::clicked,this,&EditTrainWidget::setLineColor);

    spWidth=new QDoubleSpinBox;
    spWidth->setPrefix(tr("宽度 "));
    spWidth->setSingleStep(0.5);
    spWidth->setRange(0,100);
    spWidth->setDecimals(2);
    hlay->addWidget(spWidth);

    cbLs=new PenStyleCombo;
    hlay->addWidget(cbLs);
    flay->addRow(tr("运行线设置"),hlay);

    hlay=new QHBoxLayout;
    edRouting=new QLineEdit;
    edRouting->setFocusPolicy(Qt::NoFocus);
    hlay->addWidget(edRouting);
    auto* btn=new QPushButton(tr("转到.."));
    btnToRouting=btn;
    connect(btn, &QPushButton::clicked,this,&EditTrainWidget::actSwitchToRouting);
    hlay->addWidget(btn);
    flay->addRow(tr("车底交路"),hlay);
    vlay->addLayout(flay);

    table=ctable->table();
    vlay->addWidget(ctable);

    auto* h=new ButtonGroup<3>({"确定","还原","删除车次"});
    vlay->addLayout(h);
    h->connectAll(SIGNAL(clicked()),this,
                  {SLOT(actApply()),SLOT(actCancel()),SLOT(actRemove())});

}

void EditTrainWidget::actApply()
{
    if (!_train)return;
    //生成新的列车对象，新对象不包含时刻表信息，专门作为装基础信息的容器
    TrainName name(edTrainName->text(), edDownName->text(), edUpName->text());
    if (!coll.trainNameIsValid(name, _train)) {
        QMessageBox::warning(this, tr("错误"),
            tr("全车次不能为空或与其他车次冲突，请重新编辑。"));
        return;
    }
    auto t = std::make_shared<Train>(name);
    t->setStarting(StationName::fromSingleLiteral(edStaring->text()));
    t->setTerminal(StationName::fromSingleLiteral(edTerminal->text()));

    auto& mana = coll.typeManager();
    const QString& tps = cbType->currentText();
    std::shared_ptr<TrainType> tp;
    if (tps.isEmpty()) {
        tp = mana.fromRegex(name);
    }
    else {
        tp = mana.findOrCreate(tps);
    }
    t->setType(tp);
    t->setPassenger(static_cast<TrainPassenger>(ckPassenger->checkState()));

    bool autopen = ckAutoPen->isChecked();
    if (autopen) {
        t->resetPen();
    }
    else {
        QColor color = tmpColor;
        if (!color.isValid())
            color = _train->pen().color();
        QPen pen = QPen(color, spWidth->value(),
                        static_cast<Qt::PenStyle>(cbLs->currentIndex()));
        t->setPen(pen);
    }
    emit trainInfoChanged(_train,t);

    ctable->model()->actApply();
}

void EditTrainWidget::actCancel()
{
    refreshData();
}

void EditTrainWidget::actRemove()
{
    if (_train)
        emit removeTrain(_train);
}

void EditTrainWidget::onAutoPenToggled(bool on)
{
    btnColor->setEnabled(!on);
    spWidth->setEnabled(!on);
    cbLs->setEnabled(!on);
}

void EditTrainWidget::setLineColor()
{
    auto color=QColorDialog::getColor(tmpColor, this, tr("运行线颜色"));
    if(color.isValid()){
        tmpColor=color;
        btnColor->setText(tmpColor.name());
    }
}

void EditTrainWidget::actSwitchToRouting()
{
    if(!_train) return;
    if(auto rt=_train->routing().lock()){
        emit switchToRouting(rt);
    }
}

void EditTrainWidget::setupTypeCombo()
{
    cbType->clear();
    const auto& types = coll.typeCount();
    for (auto p = types.begin(); p != types.end(); ++p) {
        if (p.value() >= 0) {
            cbType->addItem(p.key()->name());
        }
    }
}

void EditTrainWidget::onTrainFullNameUpdate()
{
    TrainName tn(edTrainName->text());
    edDownName->setText(tn.down());
    edUpName->setText(tn.up());
}
