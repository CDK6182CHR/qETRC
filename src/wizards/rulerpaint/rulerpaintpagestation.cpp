#include "rulerpaintpagestation.h"

#include "model/rail/railstationmodel.h"

#include <QtWidgets>
#include <QVariant>
#include "util/railrulercombo.h"
#include "data/diagram/diagram.h"

#include "rulerpaintwizard.h"

//Q_DECLARE_METATYPE(std::shared_ptr<Railway>);
//Q_DECLARE_METATYPE(std::shared_ptr<Ruler>);
//Q_DECLARE_METATYPE(Direction);

RulerPaintPageStation::RulerPaintPageStation(RailCategory &cat_, QWidget *parent):
    QWizardPage(parent),cat(cat_),model(new RailStationModel(false,this))
{
    initUI();
}

Direction RulerPaintPageStation::getDir() const
{
    return gpDir->get(0)->isChecked()?
                Direction::Down:Direction::Up;
}

bool RulerPaintPageStation::validatePage()
{
    _ruler=cbRuler->ruler();
    if(!_railway || !_ruler){
        QMessageBox::warning(this,tr("错误"),
                             tr("必须选择要排图的线路或者标尺。当前线路可能没有可用标尺。"));
        return false;
    }

    const auto& idx=table->currentIndex();
    if(!idx.isValid()){
        QMessageBox::warning(this,tr("错误"),tr("请在站表中选取一个锚点车站！"));
        return false;
    }
    const auto& v = model->item(idx.row(), RailStationModel::ColName)
        ->data(qeutil::RailStationRole);
    _anchorStation = qvariant_cast<std::shared_ptr<const RailStation>>(v);
    return true;
}

void RulerPaintPageStation::initUI()
{
    setTitle(tr("排图参数"));
//    setSubTitle(tr(""));
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout();

    cbRuler=new RailRulerCombo(cat);
    flay->addRow(tr("线路、标尺"),cbRuler);
    connect(cbRuler,&RailRulerCombo::railwayChagned,
            this,&RulerPaintPageStation::onRailwayChanged);

    gpDir=new RadioButtonGroup<2>({"下行","上行"},this);
    gpDir->get(0)->setChecked(true);
    flay->addRow(tr("本线运行方向"),gpDir);
    gpDir->connectAllTo(SIGNAL(toggled(bool)),this,SLOT(onDirChanged()));

    vlay->addLayout(flay);

    auto* lab=new QLabel(tr("请选择排图锚点站\n"
            "注：与pyETRC不同，可以给定一个锚点站，然后向前、向后两个方向排图，"
            "而不仅限于给定起始站向后排图"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    table=new QTableView;
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setSelectionBehavior(QTableView::SelectRows);
    table->setSelectionMode(QTableView::SingleSelection);
    table->setModel(model);

    onRailwayChanged(cbRuler->railway());     //此操作触发初始化表格！

    vlay->addWidget(table);
}

void RulerPaintPageStation::onRailwayChanged(std::shared_ptr<Railway> railway)
{
    _railway=railway;
    if(railway){
        model->setRailwayForDir(_railway,getDir());
        table->resizeColumnsToContents();
    }
}

void RulerPaintPageStation::onDirChanged()
{
    model->setRailwayForDir(_railway,getDir());
    table->resizeColumnsToContents();
}
