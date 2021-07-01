#include "trainlistwidget.h"
#include "util/buttongroup.hpp"

#include <QtWidgets>

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

	auto* h = new ButtonGroup<3>({ "上移","下移","保存顺序" });
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

}
