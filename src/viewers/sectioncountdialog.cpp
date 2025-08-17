#include "sectioncountdialog.h"

#include "data/diagram/diagram.h"
#include "data/common/qesystem.h"
#include "data/rail/railway.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QScroller>

SectionCountModel::SectionCountModel(Diagram& diagram_,
	std::shared_ptr<Railway> railway_,
	QObject* parent) :
	QStandardItemModel(parent), diagram(diagram_), railway(railway_),
	secs(diagram.sectionTrainCount(railway))
{
	setupModel();
}

void SectionCountModel::setupModel()
{
	using SI = QStandardItem;
	setHorizontalHeaderLabels({
		tr("行别"),tr("发站"),tr("到站"),tr("数量")
		});
	setRowCount(2 * railway->stationCount());
	setColumnCount(ColMAX);

	int row = 0;
	for (auto p = railway->firstDownInterval(); p;
		p = railway->nextIntervalCirc(p), row++)
	{
		setItem(row, ColDir, new SI(DirFunc::dirToString(p->direction())));
		setItem(row, ColStart, new SI(p->fromStation()->name.toSingleLiteral()));
		setItem(row, ColEnd, new SI(p->toStation()->name.toSingleLiteral()));
		setItem(row, ColCount, new SI(QString::number(secs[p])));
	}
	setRowCount(row);
}

SectionCountDialog::SectionCountDialog(Diagram& diagram_,
	std::shared_ptr<Railway> railway_,
	QWidget* parent) :
	QDialog(parent), diagram(diagram_), railway(railway_),
	model(new SectionCountModel(diagram_, railway_, this))
{
	setAttribute(Qt::WA_DeleteOnClose);
	initUI();
}

void SectionCountDialog::initUI()
{
	setWindowTitle(tr("断面对数表 - %1").arg(railway->name()));
	resize(600, 600);

	auto* vlay = new QVBoxLayout;
	auto* label = new QLabel(tr("此功能提供ETRC算法的线路断面对数表。对本线上的每一条[运行线]，"
		"作为该运行线入图站至出图站之间每一个线路区间的一个车次"
		"（不论列车时刻表中是否存在该站）。"));
	label->setWordWrap(true);
	vlay->addWidget(label);

	table = new QTableView;
	table->setModel(model);
	table->verticalHeader()->setDefaultSectionSize(
		SystemJson::get().table_row_height);
	table->resizeColumnsToContents();
	table->setEditTriggers(QTableView::NoEditTriggers);
    QScroller::grabGesture(table,QScroller::TouchGesture);

	vlay->addWidget(table);

	auto* btn = new QPushButton(tr("关闭"));
	connect(btn, SIGNAL(clicked()), this, SLOT(close()));
	vlay->addWidget(btn);

	setLayout(vlay);
}
