#include "trainlistwidget.h"
#include "util/buttongroup.hpp"

#include "mainwindow/mainwindow.h"
#include "data/train/train.h"

#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>

#include <model/train/trainlistmodel.h>

TrainListWidget::TrainListWidget(TrainCollection& coll_, QUndoStack* undo, QWidget* parent):
    QWidget(parent), coll(coll_),_undo(undo),
    table(new QTableView),editSearch(new QLineEdit), model(new TrainListModel(coll_,undo,this))
{
	initUI();
}

void TrainListWidget::refreshData()
{
	model->beginResetModel();
	model->endResetModel();
	table->resizeColumnsToContents();
}

void TrainListWidget::initUI()
{
	auto* vlay = new QVBoxLayout;

	auto* hlay = new QHBoxLayout;
	hlay->addWidget(editSearch);   //搜索行为改为filt
	connect(editSearch, SIGNAL(editingFinished()), this, SLOT(searchTrain()));
	auto* btn = new QPushButton(QObject::tr("筛选"));
	btn->setMinimumWidth(50);
	connect(btn, SIGNAL(clicked()), this, SLOT(searchTrain()));
	hlay->addWidget(btn);
	btn = new QPushButton(tr("清空筛选"));
	btn->setMinimumWidth(70);
	connect(btn, SIGNAL(clicked()), this, SLOT(clearFilter()));
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
	
	connect(table->selectionModel(), &QItemSelectionModel::currentRowChanged,
		this, &TrainListWidget::onCurrentRowChanged);

	//auto* h = new ButtonGroup<2>({ "上移","下移"});
	//h->setMinimumWidth(50);
	//vlay->addLayout(h);
	//todo: connect

	auto* g = new ButtonGroup<4>({ "编辑","分类","添加","删除" });
	g->setMinimumWidth(50);
	vlay->addLayout(g);
	g->connectAll(SIGNAL(clicked()), this, {
		SLOT(editButtonClicked()),SLOT(batchChange()),SIGNAL(addNewTrain()),SLOT(actRemoveTrains())
		});

	setLayout(vlay);
}

void TrainListWidget::searchTrain()
{
	const QString& s = editSearch->text();
	if (s.isEmpty()) {
		clearFilter();
		return;
	}
	for (int i = 0; i < coll.trainCount(); i++) {
		const auto& t = coll.trainAt(i);
		table->setRowHidden(i, !t->trainName().contains(s));
	}
}


void TrainListWidget::clearFilter()
{
	for (int i = 0; i < coll.trainCount(); i++) {
		table->setRowHidden(i, false);
	}
}

void TrainListWidget::editButtonClicked()
{
	auto index = table->currentIndex();
	if (index.isValid()) {
		emit editTrain(coll.trainAt(index.row()));
	}
}

void TrainListWidget::batchChange()
{
	auto lst = table->selectionModel()->selectedRows();
	QVector<int> rows;
	foreach (const auto& t , lst) {
		rows.push_back(t.row());
	}
	if (rows.empty()) {
		QMessageBox::warning(this, tr("错误"), tr("批量设置列车类型：请先选择至少一个车次！"));
		return;
	}
	auto types = coll.typeNames();
	bool ok;
	auto nty_s = QInputDialog::getItem(this, tr("批量设置类型"), tr("将选中的[%1]个车次类型设置为：").arg(rows.count()),
		types, 0, true, &ok);
	auto nty = coll.typeManager().findOrCreate(nty_s);
	if (!ok)return;

	//操作压栈
	_undo->push(new qecmd::BatchChangeType(coll, rows, nty, model));
	QMessageBox::information(this, tr("提示"), tr("类型更新完毕。此功能不会自动重新铺画运行图，如果需要，"
		"请手动重新铺画或刷新运行图。"));
}


void TrainListWidget::actRemoveTrains()
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

	//注意实际的删除由Model执行。这里生成列表，发给Main来执行CMD压栈
	for (auto p = rows.rbegin(); p != rows.rend(); ++p) {
		std::shared_ptr<Train> train(coll.trainAt(*p));   //move 
		trains.prepend(train);
	}

	if (_undo) {
		_undo->push(new qecmd::RemoveTrains(trains, rows, coll, model));
	}
	else {
		//不支持撤销，就直接执行了
		model->redoRemoveTrains(trains, rows);
	}
}

void TrainListWidget::onCurrentRowChanged(const QModelIndex& idx)
{
	if (!idx.isValid())
		return;
	int row = idx.row();
	emit currentTrainChanged(coll.trainAt(row));
}



qecmd::RemoveTrains::RemoveTrains(const QList<std::shared_ptr<Train>>& trains,
	const QList<int>& indexes, TrainCollection& coll_, TrainListModel* model_,
	QUndoCommand* parent) :
	QUndoCommand(QObject::tr("删除") + QString::number(trains.size()) + QObject::tr("个车次"), parent),
	_trains(trains), _indexes(indexes), coll(coll_), model(model_)
{
}

void qecmd::RemoveTrains::undo()
{
	model->undoRemoveTrains(_trains, _indexes);
}

void qecmd::RemoveTrains::redo()
{
	model->redoRemoveTrains(_trains, _indexes);
}

qecmd::SortTrains::SortTrains(const QList<std::shared_ptr<Train>>& ord_, 
	TrainListModel* model_, QUndoCommand* parent):
	QUndoCommand(QObject::tr("列车排序"),parent),ord(ord_),model(model_)
{
}

void qecmd::SortTrains::undo()
{
	model->undoRedoSort(ord);
}

void qecmd::SortTrains::redo()
{
	if (first) {
		first = false;
		return;
	}
	model->undoRedoSort(ord);
}

bool qecmd::SortTrains::mergeWith(const QUndoCommand* another)
{
	if (id() != another->id())
		return false;
	auto cmd = static_cast<const qecmd::SortTrains*>(another);
	if (model == cmd->model) {
		//只针对同一个model的排序做合并
		//成功合并：抛弃中间状态
		return true;
	}
	return false;
}

qecmd::BatchChangeType::BatchChangeType(TrainCollection& coll_, const QVector<int>& indexes_, std::shared_ptr<TrainType> type,
	TrainListModel* model_, QUndoCommand* parent) :
	QUndoCommand(QObject::tr("批量更新%1个车次类型").arg(indexes_.size()), parent),
	coll(coll_), indexes(indexes_), types(indexes_.size(), type), model(model_)
{
}

void qecmd::BatchChangeType::commit()
{
	for (int i = 0; i < indexes.size(); i++) {
		int index = indexes.at(i);
		auto train = coll.trainAt(index);
		std::swap(train->typeRef(), types[i]);
		coll.updateTrainType(train, types[i]);
	}
	model->commitBatchChangeType(indexes);
}
