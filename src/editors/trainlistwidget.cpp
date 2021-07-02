#include "trainlistwidget.h"
#include "util/buttongroup.hpp"

#include "mainwindow/mainwindow.h"

#include <QtWidgets>
#include <QString>

TrainListWidget::TrainListWidget(TrainCollection& coll_, QWidget* parent):
	QWidget(parent), coll(coll_),model(new TrainListModel(coll_,this)),
	table(new QTableView),editSearch(new QLineEdit)
{
	initUI();
}

void TrainListWidget::refreshData()
{
	model->endResetModel();
	table->update();
	table->resizeColumnsToContents();
}

void TrainListWidget::initUI()
{
	auto* vlay = new QVBoxLayout;

	auto* hlay = new QHBoxLayout;
	hlay->addWidget(editSearch);   //搜索行为改为filt?
	connect(editSearch, SIGNAL(editingFinished()), this, SLOT(searchTrain()));
	auto* btn = new QPushButton(QObject::tr("搜索"));
	connect(btn, SIGNAL(clicked()), this, SLOT(searchTrain()));
	hlay->addWidget(btn);
	vlay->addLayout(hlay);

	table->verticalHeader()->setDefaultSectionSize(20);
	table->setModel(model);
	table->setSelectionBehavior(QTableView::SelectRows);
	table->horizontalHeader()->setSortIndicatorShown(true);
	connect(table->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
		table, SLOT(sortByColumn(int, Qt::SortOrder)));
	table->resizeColumnsToContents();
	vlay->addWidget(table);
	connect(table, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editButtonClicked()));
	connect(model, SIGNAL(trainSorted()), this, SIGNAL(trainReordered()));

	auto* h = new ButtonGroup<2>({ "上移","下移"});
	h->setMinimumWidth(50);
	vlay->addLayout(h);
	//todo: connect

	auto* g = new ButtonGroup<4>({ "编辑","批量调整","添加","删除" });
	g->setMinimumWidth(50);
	vlay->addLayout(g);
	g->connectAll(SIGNAL(clicked()), this, {
		SLOT(editButtonClicked()),SLOT(batchChange()),SIGNAL(addNewTrain()),SLOT(removeTrains())
		});

	setLayout(vlay);
}

void TrainListWidget::searchTrain()
{
	//todo...
}


void TrainListWidget::editButtonClicked()
{
	auto index = table->currentIndex();
	if (index.isValid()) {
		emit editTrain(coll.trainAt(index.row()));
		qDebug() << "edit train: " << coll.trainAt(index.row())->trainName().full() << Qt::endl;
	}
}

void TrainListWidget::batchChange()
{

}


void TrainListWidget::removeTrains()
{
	auto lst = table->selectionModel()->selectedRows(0);
	if (lst.empty())
		return;

	QList<int> rows;
	rows.reserve(lst.size());
	for (const auto& p : lst) {
		rows.append(p.row());
	}

	std::sort(rows.begin(), rows.end());
	QList<std::shared_ptr<Train>> trains;
	trains.reserve(rows.size());

	//注意：倒序遍历
	for (auto p = rows.rbegin(); p != rows.rend(); ++p) {
		std::shared_ptr<Train> train(coll.takeTrainAt(*p));   //move 
		trains.prepend(train);
	}

	emit trainsRemoved(trains, rows);
}

qecmd::RemoveTrains::RemoveTrains(const QList<std::shared_ptr<Train>>& trains,
	const QList<int>& indexes, TrainCollection& coll_, MainWindow* mw_, QUndoCommand* parent) :
	QUndoCommand(QObject::tr("删除") + QString::number(trains.size()) + QObject::tr("个车次"), parent),
	_trains(trains), _indexes(indexes), coll(coll_), mw(mw_)
{
}

void qecmd::RemoveTrains::undo()
{
	//把车次添加回表里面
	for (int i = 0; i < _trains.size(); i++) {
		coll.insertTrain(_indexes.at(i), _trains.at(i));
	}
	if (mw) {
		mw->undoRemoveTrains(_trains, _indexes);
	}
}

void qecmd::RemoveTrains::redo()
{
	if (first) {
		first = false;
		return;
	}
	//倒序，把表里面的车次再一个个删掉
	for (auto p = _indexes.rbegin(); p != _indexes.rend(); ++p) {
		coll.takeTrainAt(*p);
	}
	if (mw) {
		//不能这么干！递归调用了push()，然后又redo()了
		mw->redoRemoveTrains(_trains, _indexes);
	}
}
