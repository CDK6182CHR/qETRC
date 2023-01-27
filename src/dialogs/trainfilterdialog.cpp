#include "trainfilterdialog.h"
#include "data/train/traincollection.h"
#include "editors/train/trainfilterbasicwidget.h"
#include "editors/train/trainfiltercombo.h"
#include "data/train/predeftrainfiltercore.h"

#include <QFormLayout>
#include <QMessageBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QApplication>

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
    auto* hlay=new QHBoxLayout;
    combo=new TrainFilterCombo(coll);
    connect(combo,&TrainFilterCombo::filterChanged,
            this,&TrainFilterDialog::onComboChanged);
    hlay->addWidget(combo);
    auto* btn=new QToolButton;
    btn->setIcon(qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation));
    connect(btn,&QPushButton::clicked,this,&TrainFilterDialog::informPredef);
    hlay->addWidget(btn);

    flay->addRow(tr("预设"), hlay);
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

void TrainFilterDialog::refreshData()
{
    basic->refreshData();
}

void TrainFilterDialog::showDialog()
{
    refreshData();
    show();
}

void TrainFilterDialog::actApply()
{
    basic->actApply();
    emit filterApplied(this);
    done(Accepted);
}

void TrainFilterDialog::informPredef()
{
    QMessageBox::information(this,tr("提示"),
                             tr("自1.3.0版本起，列车筛选器支持定义预设，并提供数个系统自定义预设。"
                                "此处可使用预设的筛选器，然后进一步修改筛选器。\n"
                                "如需修改预设的筛选器，请前往工具栏[列车]->[预设筛选]。"));
}
