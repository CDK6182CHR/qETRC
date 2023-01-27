#include "dialogs/trainfilterdialog.h"
#include "trainfiltercombo.h"
#include "trainfilterselector.h"
#include "data/train/predeftrainfiltercore.h"

#include <QHBoxLayout>
#include <QPushButton>



TrainFilterSelector::TrainFilterSelector(TrainCollection &coll, QWidget *parent):
    QWidget(parent),coll(coll)
{
    initUI();
}

TrainFilterSelector::TrainFilterSelector(TrainCollection& coll, const TrainFilterCore& initData, QWidget* parent):
    TrainFilterSelector(coll,parent)
{
    dlg->core.operator=(initData);
}

void TrainFilterSelector::initUI()
{
    auto* hlay=new QHBoxLayout(this);
    combo=new TrainFilterCombo(coll);
    combo->setToolTip(tr("选择预设以直接应用预设筛选器，或者编辑临时使用的筛选器。"));
    hlay->addWidget(combo);
    auto* btn=new QPushButton(tr("编辑筛选器"));
    btn->setToolTip(tr("编辑此处临时使用的列车筛选器，编辑的内容不会保存为预设。"));
    hlay->addWidget(btn);

    dlg=new TrainFilterDialog(coll,this);
    selector._core = &dlg->getCore();
    connect(btn,&QPushButton::clicked,dlg,
            &TrainFilterDialog::showDialog);
    connect(dlg,&TrainFilterDialog::filterApplied,
            this,&TrainFilterSelector::onDialogApplied);
    connect(combo,&TrainFilterCombo::filterChanged,
            this,&TrainFilterSelector::onComboChanged);
    hlay->setContentsMargins(0, 0, 0, 0);
}

void TrainFilterSelector::onComboChanged(const PredefTrainFilterCore *core)
{
    if (core){
        selector._core=core;
    }else{
        selector._core=&dlg->getCore();
    }
    emit filterChanged(selector._core);
}

void TrainFilterSelector::onDialogApplied()
{
    int idx0=combo->currentIndex();
    combo->setCurrentIndex(0);
    if (idx0==0){
        // for idx0!=0, combo would change, and this signal would be emitted
        emit filterChanged(&dlg->getCore());
    }
}

