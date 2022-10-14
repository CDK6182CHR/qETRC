#include "qecontrolledtable.h"
#include "buttongroup.hpp"

#include <QTableView>
#include <QVBoxLayout>

QEControlledTable::QEControlledTable(QWidget *parent, bool doubleLine) :
    QWidget(parent), doubleLine(doubleLine)
{
	initUI();
}

void QEControlledTable::toggleSelection()
{
    const auto& sel = table()->selectionModel()->selectedIndexes();
    foreach(const auto & idx, sel) {
        if (idx.flags() & Qt::ItemIsUserCheckable) {
            if (qvariant_cast<Qt::CheckState>(idx.data(Qt::CheckStateRole))
                == Qt::Checked) {
                _table->model()->setData(idx, Qt::Unchecked, Qt::CheckStateRole);
            }
            else {
                _table->model()->setData(idx, Qt::Checked, Qt::CheckStateRole);
            }
        }
    }
}

void QEControlledTable::checkSelection()
{
    const auto& sel = table()->selectionModel()->selectedIndexes();
    foreach(const auto & idx, sel) {
        if (idx.flags() & Qt::ItemIsUserCheckable) {
            _table->model()->setData(idx, Qt::Checked, Qt::CheckStateRole);
        }
    }
}

void QEControlledTable::uncheckSelection()
{
    const auto& sel = table()->selectionModel()->selectedIndexes();
    foreach(const auto & idx, sel) {
        if (idx.flags() & Qt::ItemIsUserCheckable) {
            _table->model()->setData(idx, Qt::Unchecked, Qt::CheckStateRole);
        }
    }
}

void QEControlledTable::initUI()
{
    auto* vlay = new QVBoxLayout(this);
    _table = new QTableView();
	vlay->addWidget(_table);
    vlay->setContentsMargins(0,0,0,0);
    //qDebug()<<"QEControlledTable: parent" <<_table->parent()<<Qt::endl;

    if(doubleLine){
        auto* g=new ButtonGroup<3>({"前插","后插","删除"});
        g->setMinimumWidth(50);
        g->connectAll(SIGNAL(clicked()),this,{SLOT(insertBefore()),
                      SLOT(insertAfter()),SLOT(removeRow())});
        vlay->addLayout(g);
        auto* h=new ButtonGroup<2>({"上移","下移"});
        h->setMinimumWidth(50);
        vlay->addLayout(h);
        h->connectAll(SIGNAL(clicked()),this,{SLOT(moveUp()),SLOT(moveDown())});
    }else{
        auto* g = new ButtonGroup<5>({
            "前插","后插","删除","上移","下移"
            });
        g->setMinimumWidth(50);
        g->connectAll(SIGNAL(clicked()), this, {
            SLOT(insertBefore()),SLOT(insertAfter()),SLOT(removeRow()),
            SLOT(moveUp()),SLOT(moveDown())
            });

        vlay->addLayout(g);
    }

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
