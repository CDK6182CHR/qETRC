#include "traineventdialog.h"

#include "data/diagram/trainadapter.h"
#include "data/rail/rail.h"
#include "data/diagram/trainevents.h"
#include "util/buttongroup.hpp"
#include "util/dialogadapter.h"
#include "model/delegate/qedelegate.h"
#include "model/delegate/generaldoublespindelegate.h"
#include "model/delegate/qetimedelegate.h"
#include "util/pagecomboforrail.h"
#include "data/diagram/diagram.h"
#include "data/common/qesystem.h"
#include "data/train/train.h"

#include <QFile>
#include <QTableView>
#include <QHeaderView>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextBrowser>

TrainEventModel::TrainEventModel(std::shared_ptr<Train> train_, Diagram& diagram_, QObject* parent):
	QStandardItemModel(parent),train(train_),diagram(diagram_)
{
	setupModel();
}

bool TrainEventModel::exportToCsv(const QString& filename)
{
	QFile file(filename);
	file.open(QFile::WriteOnly);
	if (!file.isOpen())return false;
	QTextStream s(&file);

	//标题
	for (int i = 0; i < ColMAX; i++) {
		s << horizontalHeaderItem(i)->text() << ",";
	}
	s << Qt::endl;

	//内容
	for (int i = 0; i < rowCount(); i++) {
		for (int j = 0; j < ColMAX; j++) {
			s << item(i, j)->text() << ",";
		}
		s << Qt::endl;
	}
	file.close();
	return true;
}

QTime TrainEventModel::timeForRow(int row) const
{
	return item(row, ColTime)->data(Qt::EditRole).toTime();
}

std::shared_ptr<Railway> TrainEventModel::railForRow(int row) const
{
	return qvariant_cast<std::shared_ptr<Railway>>(
		item(row, ColRail)->data(qeutil::RailwayRole));
}

double TrainEventModel::mileForRow(int row) const
{
	return item(row, ColMile)->data(Qt::EditRole).toDouble();
}

void TrainEventModel::setupModel()
{
	TrainEventList res = diagram.listTrainEvents(*train);
	beginResetModel();
	setColumnCount(ColMAX);
	setRowCount(0);
	setHorizontalHeaderLabels({
		tr("线名"),tr("时间"),tr("地点"),tr("里程"),tr("事件"),tr("客体"),tr("备注")
		});
	for (auto p = res.begin(); p != res.end(); ++p) {
		auto adp = p->first;
		const AdapterEventList& lst = p->second;
		etrcReport += train->trainName().full() + 
			tr("在%1的事件表:\n").arg(adp->railway()->name());
		for (const StationEventList& st : lst) {
			int row = rowCount();
			setRowCount(row + st.stEvents.size() + st.itEvents.size());
			//采用MergeSort的办法处理...
			auto pst = st.stEvents.begin();
			auto pit = st.itEvents.begin();
			while (pst != st.stEvents.end() && pit != st.itEvents.end()) {
				if (qeutil::timeCompare(pit->time, pst->time))
					setIntervalRow(row++, adp, *(pit++));
				else
					setStationRow(row++, adp, *(pst++));
			}
			for (; pst != st.stEvents.end(); ++pst) {
				setStationRow(row++, adp, *pst);
			}
			for (; pit != st.itEvents.end(); ++pit) {
				setIntervalRow(row++, adp, *pit);
			}
		}
	}
	endResetModel();
}

void TrainEventModel::setStationRow(int row,std::shared_ptr<TrainAdapter> adp, const StationEvent& t)
{
	using SI = QStandardItem;
	auto* it = new SI(adp->railway()->name());
	QVariant v;
	v.setValue(adp->railway());
	it->setData(v, qeutil::RailwayRole);
	setItem(row, ColRail, it);

	it = new SI();
	it->setData(t.time, Qt::EditRole);
	setItem(row, ColTime, it);

	auto rst = t.station.lock();
	setItem(row, ColPlace, new SI(rst->name.toSingleLiteral()));
	it = new SI();
	it->setData(t.station.lock()->mile, Qt::EditRole);
	setItem(row, ColMile, it);
	setItem(row, ColEvent, new SI(qeutil::eventTypeString(t.type)));
	if (t.another.has_value())
		setItem(row, ColOther, new SI(t.another.value().get().trainName().full()));
	else
		setItem(row, ColOther, new SI);
	setItem(row, ColNote, new SI(t.note));
	etrcReport += QString::number(row + 1) + ". " + t.toString() + "\n";
}

void TrainEventModel::setIntervalRow(int row, std::shared_ptr<TrainAdapter> adp, const IntervalEvent& t)
{
	using SI = QStandardItem;
	auto* it = new SI(adp->railway()->name());
	QVariant v;
	v.setValue(adp->railway());
	it->setData(v, qeutil::RailwayRole);
	setItem(row, ColRail, it);
	it = new SI;
	it->setData(t.time, Qt::EditRole);
	setItem(row, ColTime, it);
	setItem(row, ColPlace, new SI(tr("%1-%2")
        .arg(t.former->name.toSingleLiteral(),t.latter->name.toSingleLiteral())));
	it = new SI;
	it->setData(t.mile, Qt::EditRole);
	setItem(row, ColMile, it);
	setItem(row, ColEvent, new SI(qeutil::eventTypeString(t.type)));
	setItem(row, ColOther, new SI(t.another.get().trainName().full()));
	setItem(row, ColNote, new SI(t.note));
	etrcReport += QString::number(row + 1) + ". " + t.toString() + '\n';
}

TrainEventDialog::TrainEventDialog(Diagram& diagram_, std::shared_ptr<Train> train_, QWidget* parent) :
	QDialog(parent), diagram(diagram_), train(train_), 
	model(new TrainEventModel(train, diagram, this))
{
	setAttribute(Qt::WA_DeleteOnClose);
	resize(800, 800);
	initUI();
}

void TrainEventDialog::initUI()
{
	setWindowTitle(tr("%1 - 事件时刻表").arg(train->trainName().full()));
	
	auto* vlay = new QVBoxLayout;
	table = new QTableView;
	table->setEditTriggers(QTableView::NoEditTriggers);
	table->setModel(model);
	table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
	vlay->addWidget(table);
	connect(table->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
		table, SLOT(sortByColumn(int, Qt::SortOrder)));
	table->setItemDelegateForColumn(TrainEventModel::ColMile,
		new GeneralDoubleSpinDelegate(this));
	table->setItemDelegateForColumn(TrainEventModel::ColTime,
		new QETimeDelegate(this));
	table->resizeColumnsToContents();
	table->horizontalHeader()->setSortIndicatorShown(true);

	auto* act = new QAction(tr("定位到运行图"), table);
	table->addAction(act);
	connect(act, &QAction::triggered, this, &TrainEventDialog::actLocate);
	table->setContextMenuPolicy(Qt::ActionsContextMenu);

    auto* g = new ButtonGroup<5>({ "ETRC风格","导出Excel","导出CSV", "导出文本","关闭" });
	vlay->addLayout(g);
	g->connectAll(SIGNAL(clicked()), this, {
		SLOT(exportETRC()),SLOT(exportExcel()),SLOT(exportCsv()), SLOT(exportText()),SLOT(close())
		});

	setLayout(vlay);
}

void TrainEventDialog::exportText()
{
	QString f = QFileDialog::getSaveFileName(this, tr("导出ETRC风格事件表"), {},
		tr("文本文档 (*.txt)\n所有文件(*)"));
	if (f.isEmpty())
		return;
	QFile file(f);
	file.open(QFile::WriteOnly);
	if (!file.isOpen()) {
		qDebug() << "TrainEventDialog::exportText(): WARNING: open file " << f << " failed. ";
		QMessageBox::warning(this, tr("错误"), tr("打开文件失败"));
		return;
	}
	QTextStream s(&file);
	s << model->etrc();
	file.close();
}

void TrainEventDialog::exportExcel()
{
	QMessageBox::warning(this, tr("提示"), tr("导出Excel功能尚未实现。可以使用导出csv功能。"));
}

void TrainEventDialog::exportCsv()
{
	QString f0 = tr("%1列车事件表").arg(train->trainName().full());
	f0.replace('/', ',');
	QString fn = QFileDialog::getSaveFileName(this, tr("导出列车事件表"), f0,
		tr("逗号分隔文件 (*.csv)\n 所有文件 (*)"));
	if (fn.isEmpty())return;
	bool flag = model->exportToCsv(fn);
	if (flag) {
		QMessageBox::information(this, tr("提示"), tr("导出CSV文件成功"));
	}
	else {
		QMessageBox::warning(this, tr("提示"), tr("导出CSV文件失败"));
	}

}

void TrainEventDialog::actLocate()
{
	auto&& idx = table->currentIndex();
	if (!idx.isValid())return;
	auto time = model->timeForRow(idx.row());
	double mile = model->mileForRow(idx.row());
	auto rail = model->railForRow(idx.row());
	int pageIndex = PageComboForRail::dlgGetPageIndex(diagram, rail, this, tr("选择运行图"),
		tr("请选择要定位到的运行图页面名称："));
	emit locateToEvent(pageIndex, rail, mile, time);
}

void TrainEventDialog::exportETRC()
{
	auto* tb = new QTextBrowser;
	tb->setWindowTitle(tr("ETRC风格事件表"));
	tb->setText(model->etrc());
	tb->resize(500, 600);
	auto* dialog = new DialogAdapter(tb, this);
	dialog->open();
}


