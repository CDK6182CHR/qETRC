#include "qecontrolledtable.h"
#include "buttongroup.hpp"

#include <QtWidgets>

QEControlledTable::QEControlledTable(QWidget *parent) : QWidget(parent)
{
	initUI();
}

void QEControlledTable::initUI()
{
    auto* vlay = new QVBoxLayout(this);
    _table = new QTableView();
	vlay->addWidget(_table);
    //qDebug()<<"QEControlledTable: parent" <<_table->parent()<<Qt::endl;

	auto* g = new ButtonGroup<5>({
		"前插","后插","删除","上移","下移"
		});
	g->setMinimumWidth(50);
	g->connectAll(SIGNAL(clicked()), this, {
		SLOT(insertBefore()),SLOT(insertAfter()),SLOT(removeRow()),
		SLOT(moveUp()),SLOT(moveDown())
		});

	vlay->addLayout(g);
    //qDebug()<<"Layout parent: "<<g->parent()<<Qt::endl;
    //qDebug()<<"QEControlledTable: parent" <<_table->parent()<<Qt::endl;
}


void QEControlledTable::insertBefore()
{
	auto idx = _table->currentIndex();
	_table->model()->insertRow(idx.row());
}

void QEControlledTable::insertAfter()
{
	auto idx = _table->currentIndex();
	_table->model()->insertRow(idx.row() + 1);
}

void QEControlledTable::removeRow()
{
	auto idx = _table->currentIndex();
	if (idx.isValid())
		_table->model()->removeRow(idx.row());
}

void QEControlledTable::moveUp()
{
	auto idx = _table->currentIndex();
	if (idx.isValid()&&idx.row()!=0) {
		_table->model()->moveRow(QModelIndex(), idx.row(), QModelIndex(), idx.row() - 1);
		_table->setCurrentIndex(_table->model()->index(idx.row() - 1, idx.column()));
	}
}

void QEControlledTable::moveDown()
{
	//下移这一行，等价于把它下面一行上移
	auto idx = _table->currentIndex();
	if (idx.isValid() && idx.row() != _table->model()->rowCount() - 1) {
		_table->model()->moveRow(QModelIndex(), idx.row() + 1, QModelIndex(), idx.row());
		_table->setCurrentIndex(_table->model()->index(idx.row() + 1, idx.column()));
	}
}
