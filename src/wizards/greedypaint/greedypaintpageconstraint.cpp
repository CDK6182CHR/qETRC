#include "greedypaintpageconstraint.h"

#include <model/rail/forbidlistmodel.h>
#include <model/rail/gapconstraintmodel.h>
#include <data/gapset/crset.h>
#include <data/gapset/transparentset.h>
#include <QCheckBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QListView>
#include <QLabel>
#include <util/railrulercombo.h>
#include <data/diagram/diagram.h>
#include <data/common/qesystem.h>
#include <model/delegate/generalspindelegate.h>
#include <data/calculation/greedypainter.h>


GreedyPaintPageConstraint::GreedyPaintPageConstraint(Diagram& diagram_, GreedyPainter &_painter,
                                                     QWidget *parent):
    QWidget(parent), diagram(diagram_), painter(_painter),
    _model(new GapConstraintModel(this)),
    _mdForbid(new SelectForbidModel(this))
{
    setWindowTitle(tr("排图参数"));
    initGapSets();
    initUI();
}

void GreedyPaintPageConstraint::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    cbRuler=new RailRulerCombo(diagram.railCategory());
    flay->addRow(tr("线路、标尺"), cbRuler);
    vlay->addLayout(flay);

    auto* hlay=new QHBoxLayout;
    spBack=new QSpinBox;
    spBack->setRange(0,10000000);
    spBack->setValue(3);
    hlay->addWidget(spBack);
    hlay->addStretch(1);

    ckSingle=new QCheckBox(tr("单线"));
    hlay->addWidget(ckSingle);

    flay->addRow(tr("最大尝试回溯次数"),hlay);

    gpGapSet=new RadioButtonGroup<2>({"追踪/会车间隔方案","完整方案"}, this);
    flay->addRow(tr("间隔控制方案"),gpGapSet);
    connect(gpGapSet->group(),&QButtonGroup::idToggled,
            this,&GreedyPaintPageConstraint::onGapSetToggled);

    vlay->addWidget(new QLabel(tr("考虑以下天窗: ")));
    lstForbid=new QListView;
    lstForbid->setModel(_mdForbid);
    vlay->addWidget(lstForbid,1);
    connect(cbRuler,&RailRulerCombo::railwayChagned,
            _mdForbid,&SelectForbidModel::setRailway);
    _mdForbid->setRailway(cbRuler->railway());

    table = new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setEditTriggers(QTableView::AllEditTriggers);
    table->setModel(_model);
    table->setItemDelegateForColumn(GapConstraintModel::ColLimit,
            new TrainGapSpinDelegate(this));
    gpGapSet->get(0)->setChecked(true);   // 此操作导致table的刷新

    vlay->addWidget(new QLabel(tr("列车间隔规定：")));
    vlay->addWidget(table,3);

    connect(ckSingle, &QCheckBox::toggled,
            this, &GreedyPaintPageConstraint::onSingleLineChanged);

    hlay=new QHBoxLayout;
    hlay->addStretch(1);
    auto* btn=new QPushButton(tr("确定"));
    connect(btn,&QPushButton::clicked,this,&GreedyPaintPageConstraint::onApply);
    hlay->addWidget(btn);

    btn=new QPushButton(tr("关闭"));
    connect(btn,&QPushButton::clicked,this,&GreedyPaintPageConstraint::actClose);
    hlay->addWidget(btn);
    vlay->addLayout(hlay);
}

void GreedyPaintPageConstraint::initGapSets()
{
    _availableGapSets[0] = std::make_unique<gapset::cr::CRSet>();
    _availableGapSets[1] = std::make_unique<gapset::TransparentSet>();

    for(int i=0;i<GAP_SET_COUNT;i++){
        _availableGapSets[i]->buildSet();
    }
}

void GreedyPaintPageConstraint::onApply()
{
    auto rail=cbRuler->railway();
    auto ruler=cbRuler->ruler();
    if(!rail || rail->empty() || ! ruler){
        QMessageBox::warning(this,tr("错误"),tr("未选中线路或标尺，或空线路！"));
        return;
    }

    auto & cns=painter.constraints();
    cns.setSingleLine(ckSingle->isChecked());
    painter.setRailway(rail);
    painter.setRuler(ruler);
    painter.setMaxBackoffTimes(spBack->value());

    painter.usedForbids()=_mdForbid->selectedForbids();

    // 设置间隔
    painter.constraints().clear();
    auto* gs = _model->gapSet();
    for (const auto& gapgroup : *gs) {
        for (const auto& t : *gapgroup) {
            painter.constraints()[t] = gapgroup->limit();
        }
    }

    for (auto t : gs->remainTypes()) {
        painter.constraints()[t] = 0;
    }

    emit constraintChanged();
}

void GreedyPaintPageConstraint::onSingleLineChanged(bool on)
{
    _model->setSingleLine(on);
}

void GreedyPaintPageConstraint::onGapSetToggled(int id, bool on)
{
    if(on){
        _model->setGapSet(_availableGapSets[id].get(),ckSingle->isChecked());
    }

}
