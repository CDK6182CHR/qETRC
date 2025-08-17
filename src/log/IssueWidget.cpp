#include "IssueWidget.h"
#include "log/IssueManager.h"
#include "data/common/qesystem.h"
#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>


IssueWidget::IssueWidget(QWidget* parent):
	QWidget(parent)
{
	initUI();
}

void IssueWidget::initUI()
{
	auto* vlay = new QVBoxLayout(this);

	table = new QTableView;
	table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
	table->setModel(IssueManager::get());

	{
		int c = 0;
		for (int w : {100, 100, 120, 80, 1000}) {
			table->setColumnWidth(c++, w);
		}
	}
	vlay->addWidget(table);
}
