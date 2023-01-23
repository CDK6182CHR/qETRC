#include "trainfilterdialog.h"
#include "data/train/traincollection.h"
#include "editors/train/trainfilterbasicwidget.h"
#include "editors/train/trainfiltercombo.h"
#include "data/train/predeftrainfiltercore.h"

#include <QFormLayout>
#include <QVBoxLayout>

TrainFilterDialog::TrainFilterDialog(TrainCollection &coll, QWidget *parent):
    QDialog(parent), coll(coll)
{
    resize(500,600);
    setWindowTitle(tr("车次筛选器"));
    initUI();
}

bool TrainFilterDialog::check(std::shared_ptr<const Train> train) const
{
    return core.check(train);
}

QList<std::shared_ptr<Train> > TrainFilterDialog::selectedTrains() const
{
    QList<std::shared_ptr<Train>> res;
    foreach(auto train, coll.trains()) {
        if (check(train))
            res.push_back(train);
    }
    return res;
}

void TrainFilterDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;
    combo=new TrainFilterCombo(coll);
    connect(combo,&TrainFilterCombo::filterChanged,
            this,&TrainFilterDialog::onComboChanged);
    flay->addRow(tr("预设"), combo);
    vlay->addLayout(flay);

    basic=new TrainFilterBasicWidget(coll,&core);
    basic->layout()->setContentsMargins(0,0,0,0);
    basic->setCore(&core);
    vlay->addWidget(basic);

    auto* g=new ButtonGroup<3>({"确定","清空","关闭"});
    g->connectAll(&QPushButton::clicked, this,  &TrainFilterDialog::actApply,
                  &TrainFilterDialog::clearFilter,&TrainFilterDialog::close);
    vlay->addLayout(g);
}

void TrainFilterDialog::clearFilter()
{
    basic->clearFilter();
}

void TrainFilterDialog::onComboChanged(const PredefTrainFilterCore *data)
{
    if (data){
        basic->refreshDataWith(data);
    }
}

void TrainFilterDialog::actApply()
{
    basic->actApply();
    emit filterApplied(this);
    done(Accepted);
}
