#include "addroutingnodedialog.h"

#include "util/selecttraincombo.h"
#include "util/buttongroup.hpp"
#include "data/train/train.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>

AddRoutingNodeDialog::AddRoutingNodeDialog(TrainCollection &coll_, QWidget *parent):
    QDialog(parent), coll(coll_)
{
    resize(500,500);
    setWindowTitle(tr("添加车次"));
    initUI();
}

void AddRoutingNodeDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* g=new QButtonGroup(this);

    auto* lab=new QLabel(tr("[实体车次]是指该车次属于本运行图中的一个实际车次，"
        "每个车次只能以实体车次的方式，属于至多一个交路；\n"
        "[虚拟车次]仅包含车次名称信息，只是个占位，只需输入一个非空的车次即可，并不检查重复性。"
        "可以通过[识别]操作，根据车次名称，将虚拟车次转换为实体车次。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* rd=new QRadioButton(tr("实体车次(&T)"));
    g->addButton(rd);
    auto* flay=new QFormLayout;
    cbTrain=new SelectTrainCombo(coll);
    connect(cbTrain, &SelectTrainCombo::currentTrainChanged,
        this, &AddRoutingNodeDialog::onTrainChanged);
    rd->setChecked(true);
    flay->addRow(rd,cbTrain);

    rd=new QRadioButton(tr("虚拟车次(&V)"));
    rdVirtual = rd;
    connect(rd, &QRadioButton::toggled, this, &AddRoutingNodeDialog::virtualToggled);
    g->addButton(rd);
    edName=new QLineEdit;
    edName->setEnabled(false);
    flay->addRow(rd,edName);

    edStarting=new QLineEdit;
    //edStarting->setFocusPolicy(Qt::NoFocus);
    flay->addRow(tr("始发站"),edStarting);
    edTerminal=new QLineEdit;
    //edTerminal->setFocusPolicy(Qt::NoFocus);
    flay->addRow(tr("终到站"),edTerminal);

    ckLink=new QCheckBox(tr("开始处连线"));
    flay->addRow(tr("选项"),ckLink);
    ckLink->setChecked(true);
    vlay->addLayout(flay);

    lab=new QLabel(tr("[开始处连线]是指是否尝试在本列车运行线开始处，与前一车次结束处绘制"
        "连线。只有设置为连线，并且前一车次终到站与本次列车始发站为运行图中同一个站时，"
        "才会实际画出连线记号。交路第一个车次的这个设置项无效，可以任意设置。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* h=new ButtonGroup<2>({"确定","取消"});
    vlay->addLayout(h);
    h->connectAll(SIGNAL(clicked()), this, {SLOT(actApply()),SLOT(close())});
}

void AddRoutingNodeDialog::virtualToggled(bool on)
{
    edName->setEnabled(on);
    cbTrain->setEnabled(!on);
    edStarting->setEnabled(on);
    edTerminal->setEnabled(on);
    if (on) {
        onTrainChanged(nullptr);
        edName->setFocus();
    }
    else {
        cbTrain->focusIn();
    }
}

void AddRoutingNodeDialog::onTrainChanged(std::shared_ptr<Train> train)
{
    if(train){
        edStarting->setText(train->starting().toSingleLiteral());
        edTerminal->setText(train->terminal().toSingleLiteral());
    }else{
        edStarting->clear();
        edTerminal->clear();
    }
}

void AddRoutingNodeDialog::actApply()
{
    if(rdVirtual->isChecked()){
        //虚拟
        auto&& s=edName->text();
        if(s.isEmpty()){
            QMessageBox::warning(this,tr("错误"),tr("请输入一个非空的车次。\n"
            "虚拟车次不检查是否重复，但需要输入非空车次。"));
            return;
        }else{
            emit virtualTrainAdded(row,edName->text(), edStarting->text(), edTerminal->text(), 
                ckLink->isChecked());
        }

    }else{
        auto train=cbTrain->train();
        if (train){
            emit realTrainAdded(row,train,ckLink->isChecked());
        }else{
            QMessageBox::warning(this,tr("错误"),tr("请先选择车次"));
            return;
        }
    }
    clearPage();
    done(Accepted);
}

void AddRoutingNodeDialog::clearPage()
{
    cbTrain->resetTrain();
    onTrainChanged(nullptr);
    edName->clear();
}

void AddRoutingNodeDialog::openForRow(int row)
{
    this->row=row;
    virtualToggled(rdVirtual->isChecked());
    open();
}
