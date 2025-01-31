#include "trainliststdmodel.h"

#include "data/train/train.h"
#include "data/train/traintype.h"
#include "util/utilfunc.h"
#include "model/delegate/qedelegate.h"

TrainListStdModel::TrainListStdModel(QObject* parent):
	QEMoveableModel(parent)
{
	setColumnCount(ColMAX);
	setHorizontalHeaderLabels({
		tr("车次"), tr("始发"), tr("终到"), tr("类型"), tr("显示"),
		tr("铺画里程")
		});
}

void TrainListStdModel::addTrain(std::shared_ptr<Train> train)
{
	int row = rowCount();
	setRowCount(rowCount() + 1);

	using SI = QStandardItem;
	auto* it = new SI(train->trainName().full());
	QVariant v;
	v.setValue(train);
	it->setData(v, qeutil::TrainRole);
	setItem(row, ColTrainName, it);

	setItem(row, ColStarting, new SI(train->starting().toSingleLiteral()));
	setItem(row, ColTerminal, new SI(train->terminal().toSingleLiteral()));
	setItem(row, ColType, new SI(train->type()->name()));

	it = new SI;
	it->setCheckState(qeutil::boolToCheckState(train->isShow()));
	setItem(row, ColShow, it);

	setItem(row, ColMile, new SI(QString::number(train->localMile(), 'f', 3)));
}

std::vector<std::shared_ptr<Train>> TrainListStdModel::trains() const
{
	std::vector<std::shared_ptr<Train>>res{};
	for (int r = 0; r < rowCount(); r++) {
		res.emplace_back(qvariant_cast<std::shared_ptr<Train>>(item(r, ColTrainName)->data(qeutil::TrainRole)));
	}
	return res;
}
