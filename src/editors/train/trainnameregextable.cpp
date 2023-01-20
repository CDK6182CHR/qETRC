#include "trainnameregextable.h"
#include "model/general/qemoveablemodel.h"
#include <QTableView>
#include <QHeaderView>
#include "data/common/qesystem.h"


TrainNameRegexTable::TrainNameRegexTable(TrainCollection &coll_, QWidget *parent):
    QEControlledTable(parent),coll(coll_),model(new QEMoveableModel(this))
{
    initUI();
}

void TrainNameRegexTable::initUI()
{
    model->setColumnCount(1);
    model->setHorizontalHeaderLabels({tr("车次正则")});

    table()->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table()->setEditTriggers(QTableView::AllEditTriggers);
    table()->setModel(model);
}
