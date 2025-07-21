#include "timeinterppagepreview.h"
#include "data/train/train.h"
#include "data/train/traintype.h"
#include "data/diagram/trainadapter.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>

#include "data/common/qesystem.h"
#include "model/delegate/generaldoublespindelegate.h"
#include "data/diagram/diagramoptions.h"

TimeInterpPreviewModel::TimeInterpPreviewModel(const DiagramOptions& ops, QObject *parent):
	QStandardItemModel(parent), _ops(ops)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({tr("车次"),tr("类型"),tr("始发"),tr("终到"),tr("相对误差")});
}


void TimeInterpPreviewModel::setupModel(std::shared_ptr<Railway> railway,
                                        std::shared_ptr<Ruler> ruler,
                                        const QVector<std::shared_ptr<Train> > trains)
{
    using SI=QStandardItem;
    setRowCount(trains.size());
    for(int i=0;i<trains.size();i++){
        auto train=trains.at(i);
        setItem(i,ColName,new SI(train->trainName().full()));
        setItem(i,ColStarting,new SI(train->starting().toSingleLiteral()));
        setItem(i,ColTerminal,new SI(train->terminal().toSingleLiteral()));
        setItem(i,ColType,new SI(train->type()->name()));
        auto adp=train->adapterFor(*railway);
        auto* it=new SI;
        setItem(i,ColError,it);
        QColor color(Qt::transparent);
        if(!adp){
            it->setData(0,Qt::EditRole);
        }
        else{
            double er=adp->relativeError(ruler, _ops.period_hours);
            it->setData(er, Qt::EditRole);
            color=Qt::yellow;
            color.setAlphaF(er);
        }
        for(int c=0;c<ColMAX;c++){
            item(i,c)->setBackground(color);
        }
    }
}

TimeInterpPagePreview::TimeInterpPagePreview(const DiagramOptions& ops, QWidget *parent):
    QWizardPage(parent), _ops(ops), model(new TimeInterpPreviewModel(_ops, this))
{
    setTitle(tr("确认"));
    initUI();
}

void TimeInterpPagePreview::setupData(std::shared_ptr<Railway> railway, std::shared_ptr<Ruler> ruler, const QVector<std::shared_ptr<Train> > trains)
{
    model->setupModel(railway,ruler,trains);
    table->resizeColumnsToContents();
}

void TimeInterpPagePreview::initUI()
{
    setSubTitle(tr("以下是所选的各个车次关于通过标尺的总体相对误差，选择确定修改，点击上一步返回"
                               "重新选择。行的颜色越深表明误差越大。"));
    auto* vlay=new QVBoxLayout(this);
    table=new QTableView;
    vlay->addWidget(table);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->setItemDelegateForColumn(TimeInterpPreviewModel::ColError,
        new GeneralDoubleSpinDelegate(this));
}
