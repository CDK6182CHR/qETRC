#include "readrulerpagepreview.h"

#include <QtWidgets>
#include "data/diagram/diagram.h"

ReadRulerPreviewModel::ReadRulerPreviewModel(const ReadRulerReport& data_,
    const QVector<std::shared_ptr<RailInterval>>& intervals_, QObject* parent):
	QStandardItemModel(parent),data(data_),intervals(intervals_)
{
	setColumnCount(ColMAX);
	setHorizontalHeaderLabels({
		tr("区间"),tr("通通"),tr("起步"),tr("停车"),tr("数据类"),tr("数据总量"),
		tr("满足车次")
		});
}

ReadRulerPagePreview::ReadRulerPagePreview(QWidget* parent):
	QWizardPage(parent),model(new ReadRulerPreviewModel(data, this))
{
	initUI();
}

void ReadRulerPagePreview::initUI()
{
    setTitle(tr("预览"));
    setSubTitle("计算结果如下表所示。\n"
                           "点击[完成]应用结果，否则结果不会被应用。\n"
                           "双击行显示详细计算数据。\n"
                           "表中的[数据类]是指在[通通]/[起通]/[通停]/[起停]这四种情况中，"
                           "有多少种情况的有效数据；"
                           "[数据总量]是指用于计算的数据总条数；[满足车次]是指用于计算的车次中，"
                           "严格满足标尺的数量。"
                           );
    auto* vlay=new QVBoxLayout(this);
    table=new QTableView;
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    vlay->addWidget(table);
}

