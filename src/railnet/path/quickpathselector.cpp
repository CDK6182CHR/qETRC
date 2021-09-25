#include "quickpathselector.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QHeaderView>
#include "data/common/qesystem.h"
#include "railnet/graph/railnet.h"

#include <util/qecontrolledtable.h>
#include <model/general/qemoveablemodel.h>
#include <QMessageBox>
#include <data/rail/railway.h>



QuickPathSelector::QuickPathSelector(RailNet &net, QWidget *parent):
    QWidget(parent),net(net)
{
    initUI();
}

void QuickPathSelector::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    auto* hlay=new QHBoxLayout;
    ckRuler=new QCheckBox(tr("同时导出标尺"));
    hlay->addWidget(ckRuler);
    hlay->addStretch(2);
    spRuler=new QSpinBox;
    spRuler->setRange(1,100000);
    hlay->addWidget(new QLabel(tr("最小标尺区间数: ")));
    hlay->addWidget(spRuler);
    hlay->addStretch(1);
    spRuler->setToolTip(tr("最小标尺区间数\n指定导出标尺所需要具有的最少有效区间数。"
        "有效区间数小于指定值的标尺将被删除。"
        "设置为1以导出全部标尺。"));
    flay->addRow(tr("标尺"),hlay);
    spRuler->setEnabled(false);
    connect(ckRuler,&QCheckBox::toggled, spRuler,
            &QSpinBox::setEnabled);

    gpUp=new RadioButtonGroup<3>({"自动计算","手动给出经由","强制对称"}, this);
    gpUp->get(0)->setChecked(true);
    gpUp->connectAllTo(SIGNAL(toggled(bool)),this,SLOT(onUpModeChanged()));
    flay->addRow(tr("反向径路"),gpUp);
    vlay->addLayout(flay);

    hlay=new QHBoxLayout;
    hlay->addWidget(new QLabel(tr("正向径路")));
    hlay->addWidget(new QLabel(tr("反向径路")));
    vlay->addLayout(hlay);

    hlay=new QHBoxLayout;
    ctbDown=new QEControlledTable(this, true);
    tbDown=ctbDown->table();
    tbDown->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    tbDown->setEditTriggers(QTableView::AllEditTriggers);
    mdDown=new QEMoveableModel(this);
    mdDown->setColumnCount(1);
    mdDown->setHorizontalHeaderLabels({tr("站名")});
    tbDown->setModel(mdDown);
    hlay->addWidget(ctbDown);

    ctbUp=new QEControlledTable(this, true);
    tbUp=ctbUp->table();
    tbUp->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    tbUp->setEditTriggers(QTableView::AllEditTriggers);
    hlay->addWidget(ctbUp);
    ctbUp->setEnabled(false);
    mdUp=new QEMoveableModel(this);
    mdUp->setColumnCount(1);
    mdUp->setHorizontalHeaderLabels({tr("站名")});
    tbUp->setModel(mdUp);

    vlay->addLayout(hlay);

    auto* g=new ButtonGroup<2>({"预览","强制生成"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actGenerate()),SLOT(actForce())});
}

QVector<QString> QuickPathSelector::pathFromModel(const QEMoveableModel *model)
{
    QVector<QString> res;
    for(int i=0;i<model->rowCount();i++){
        if (auto* it = model->item(i, 0)) {
            QString&& s = it->text();
            if (!s.isEmpty()) {
                res.push_back(std::forward<QString>(s));
            }
        }
    }
    return res;
}

void QuickPathSelector::actGenerate()
{
    QString report;
    bool withRuler=ckRuler->isChecked();
    int rulerCount=spRuler->value();
    auto downPath=pathFromModel(mdDown);
    if(downPath.isEmpty()){
        QMessageBox::warning(this,tr("错误"),tr("正向径路为空，无法生成。\n"
        "注：径路表中的空白项将被忽略。"));
        return;
    }

    RailNet::rail_ret_t ret;

    if(gpUp->get(0)->isChecked()){
        // 单向径路
        ret=net.sliceBySinglePath(downPath, withRuler,&report, rulerCount);
    }else if(gpUp->get(1)->isChecked()){
        // 双向径路
        auto upPath=pathFromModel(mdUp);
        ret=net.sliceByDoublePath(downPath,upPath,withRuler,&report,rulerCount);
    }else{
        ret=net.sliceBySymmetryPath(downPath,withRuler,&report,rulerCount);
    }

    if (ret.railway){
        QString pathString = QString("正向径路：\n%1\n\n反向径路：\n%2\n").arg(
            net.pathToString(ret.downPath), net.pathToString(ret.upPath));
        emit railGenerated(ret.railway, pathString);
    }else{
        QMessageBox::warning(this,tr("错误"),tr("径路未能生成，原因如下：\n%1")
                             .arg(report));
    }
}

void QuickPathSelector::actForce()
{
    auto path = pathFromModel(mdDown);
    if (path.size() < 2) {
        QMessageBox::warning(this, tr("错误"), tr("使用强制生成径路功能，"
            "需给出至少两个非空关键点。"));
        return;
    }
    auto rail = std::make_shared<Railway>(tr("%1-%2").arg(path.front(), path.back()));
    int row = 0;
    foreach(const auto & p, path) {
        rail->appendStation(p, 10 * (row++), 4);
    }
    if (informForce) {
        QMessageBox::information(this, tr("说明"), tr("强制生成功能允许根据输入的关键点表，"
            "每个关键点直接作为一个车站，生成线路，车站间距为10 km。"
            "提供此功能是为了支持查看快速任意区间的列车时刻情况，无论有没有基线数据。\n"
            "请注意此时无论[反向径路]如何选择，反向的径路关键点表不被使用。\n"
            "此消息在程序每次运行期间弹出一次。"));
        informForce = false;
    }

    emit railGenerated(rail, tr("该径路由强制生成功能生成，无经由信息"));
}

void QuickPathSelector::onUpModeChanged()
{
    ctbUp->setEnabled(gpUp->get(1)->isChecked());
}
