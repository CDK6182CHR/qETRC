#include "traincomparedialog.h"
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <data/diagram/diadiff.h>
#include <data/train/trainstation.h>
#include <data/common/qesystem.h>
#include <data/train/trainname.h>

TrainCompareModel::TrainCompareModel(QObject* parent):
	QStandardItemModel(parent)
{
	setColumnCount(ColMAX);
	setHorizontalHeaderLabels({ tr("站名1"),tr("到点1"),tr("开点1"),tr("说明"),tr("站名2"),tr("到点2"),tr("开点2") });
}

void TrainCompareModel::refreshData()
{
	using SI = QStandardItem;
	setRowCount(diff->stations.size());
    for (size_t i = 0; i < diff->stations.size(); i++) {
		const auto& t = diff->stations.at(i);
		if (t.station1.has_value()) {
			setItem(i, ColName1, new SI(t.station1.value()->name.toSingleLiteral()));
			setItem(i, ColArrive1, new SI(t.station1.value()->arrive.toString(TrainTime::HMS)));
			setItem(i, ColDepart1, new SI(t.station1.value()->depart.toString(TrainTime::HMS)));
		}
		if (t.station2.has_value()) {
			setItem(i, ColName2, new SI(t.station2.value()->name.toSingleLiteral()));
			setItem(i, ColArrive2, new SI(t.station2.value()->arrive.toString(TrainTime::HMS)));
			setItem(i, ColDepart2, new SI(t.station2.value()->depart.toString(TrainTime::HMS)));
		}
		if (t.type == StationDiff::Unchanged) {
			// nothing todo ..
		}
		else if (t.type == StationDiff::NewAdded) {
			setItem(i, ColDiff, new SI(tr("新增")));
			for (int j = ColName2; j <= ColDepart2; j++) {
				item(i, j)->setForeground(Qt::blue);
			}
		}
		else if (t.type == StationDiff::Deleted) {
			setItem(i, ColDiff, new SI(tr("删除")));
			for (int j = ColName1; j <= ColDepart1; j++) {
				item(i, j)->setForeground(Qt::darkGray);
			}
		}
		else if (t.type == StationDiff::NameChanged) {
			setItem(i, ColDiff, new SI(tr("改名")));
			item(i, ColName1)->setForeground(Qt::red);
			item(i, ColName2)->setForeground(Qt::red);
		}
		else {
			setItem(i, ColDiff, new SI(tr("改点")));
			if (t.type == StationDiff::ArriveModified) {
				item(i, ColArrive1)->setForeground(Qt::red);
				item(i, ColArrive2)->setForeground(Qt::red);
			}
			else if (t.type == StationDiff::DepartModified) {
				item(i, ColDepart1)->setForeground(Qt::red);
				item(i, ColDepart2)->setForeground(Qt::red);
			}
			else {
				// Both ..
				item(i, ColArrive1)->setForeground(Qt::red);
				item(i, ColDepart1)->setForeground(Qt::red);
				item(i, ColArrive2)->setForeground(Qt::red);
				item(i, ColDepart2)->setForeground(Qt::red);
			}
		}
	}
}

void TrainCompareModel::resetData(std::shared_ptr<TrainDifference> diff)
{
	this->diff = diff;
	refreshData();
}

TrainCompareDialog::TrainCompareDialog(QWidget* parent):
	QDialog(parent),_model(new TrainCompareModel(this))
{
	setWindowTitle(tr("列车时刻对照"));
    setAttribute(Qt::WA_DeleteOnClose);
	initUI();
}

TrainCompareDialog::TrainCompareDialog(std::shared_ptr<TrainDifference> diff, QWidget* parent):
	QDialog(parent),_model(new TrainCompareModel(this))
{
	setWindowTitle(tr("列车时刻对照 - %1").arg(diff->trainName().full()));
	setAttribute(Qt::WA_DeleteOnClose);
	initUI();
	setData(diff);
}

void TrainCompareDialog::setData(std::shared_ptr<TrainDifference> diff)
{
	_model->resetData(diff);
	table->resizeColumnsToContents();
}

void TrainCompareDialog::refreshData()
{
	_model->refreshData();
}

void TrainCompareDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    table->setEditTriggers(QTableView::NoEditTriggers);
	table->setModel(_model);
    vlay->addWidget(table);

    auto* btn=new QPushButton(tr("关闭"));
    vlay->addWidget(btn);
    connect(btn,&QPushButton::clicked,this,&QDialog::close);
}
