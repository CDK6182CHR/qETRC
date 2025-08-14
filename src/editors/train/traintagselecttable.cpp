#include "traintagselecttable.h"

#include <QTableView>
#include <QHeaderView>
#include "model/general/qemoveablemodel.h"
#include "data/common/qesystem.h"
#include "model/delegate/textcompletiondelegate.h"

TrainTagSelectTable::TrainTagSelectTable(TrainTagManager& manager, QCompleter* completer, QWidget* parent):
	QEControlledTable(parent), m_manager(manager), m_completer(completer), m_model(new QEMoveableModel(this))
{
	initUI();
}

QVector<QString> TrainTagSelectTable::selectedTags() const
{
	QVector<QString> res;
	for(int i = 0; i < m_model->rowCount(); i++) {
		if(auto* it = m_model->item(i, 0)) {
			if(!it->text().isEmpty()) {
				res.push_back(it->text());
			}
		}
	}
	return res;
}

void TrainTagSelectTable::initUI()
{
	m_model->setColumnCount(1);
	m_model->setHorizontalHeaderLabels({tr("列车标签")});

	table()->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
	table()->setEditTriggers(QTableView::AllEditTriggers);
	table()->setModel(m_model);
	table()->setItemDelegateForColumn(0, new TextCompletionDelegate(m_completer, this));
	
	table()->setColumnWidth(0, 200);
}

void TrainTagSelectTable::refreshData(const QVector<QString>& tags)
{
	m_model->setRowCount(tags.size());
	for(int i = 0; i < m_model->rowCount(); i++) {
		m_model->setItem(i, new QStandardItem(tags.at(i)));
	}
}
