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

void TrainFilterSelector::initUI()
{
    auto* hlay=new QHBoxLayout(this);
    combo=new TrainFilterCombo(coll);
    hlay->addWidget(combo);
    auto* btn=new QPushButton(tr("编辑筛选器"));
    hlay->addWidget(btn);

    dlg=new TrainFilterDialog(coll,this);
    connect(btn,&QPushButton::clicked,dlg,
            &TrainFilterDialog::show);
    connect(dlg,&TrainFilterDialog::filterApplied,
            this,&TrainFilterSelector::onDialogApplied);
    connect(combo,&TrainFilterCombo::filterChanged,
            this,&TrainFilterSelector::onComboChanged);
}

void TrainFilterSelector::onComboChanged(const PredefTrainFilterCore *core)
{
    if (core){
        _current=core;
    }else{
        _current=&dlg->getCore();
    }
    emit filterChanged(_current);
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

