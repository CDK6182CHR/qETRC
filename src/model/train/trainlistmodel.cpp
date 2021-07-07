#include "trainlistmodel.h"
#include "editors/trainlistwidget.h"


TrainListModel::TrainListModel(TrainCollection& collection, QUndoStack* undo, QObject* parent):
	QAbstractTableModel(parent), coll(collection), _undo(undo)
{
}

int TrainListModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return coll.size();
}

int TrainListModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return MAX_COLUMNS;
}

QVariant TrainListModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return {};
	std::shared_ptr<Train> t = coll.trainAt(index.row());
	if (role == Qt::DisplayRole) {
		switch (index.column()) {
		case ColTrainName:return t->trainName().full();
		case ColStarting:return t->starting().toSingleLiteral();
		case ColTerminal:return t->terminal().toSingleLiteral();
		case ColType:return t->type()->name();
		case ColMile:return QString::number(t->localMile(), 'f', 3);
		case ColSpeed:return QString::number(t->localTraverseSpeed(), 'f', 3);
		}
	}
	else if (role == Qt::CheckStateRole) {
		if (index.column() == ColShow)
			return t->isShow() ? Qt::Checked : Qt::Unchecked;
	}
	return {};
}

bool TrainListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid())
		return false;
	auto t = coll.trainAt(index.row());
	if (role == Qt::CheckStateRole) {
		if (index.column() == ColShow) {
			Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
			bool show = (state == Qt::Checked);
			if (show != t->isShow()) {
				t->setIsShow(show);
				emit trainShowChanged(t, show);
				return true;
			}
		}
	}
	return false;
}

Qt::ItemFlags TrainListModel::flags(const QModelIndex& index) const
{
	switch (index.column()) {
	case ColShow:return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;
	}
	return QAbstractTableModel::flags(index);
}

void TrainListModel::sort(int column, Qt::SortOrder order)
{
	//把旧版的列表复制一份
	QList<std::shared_ptr<Train>> oldList(coll.trains());   //copy construct!!
	beginResetModel();
	auto& lst = coll.trains();
	if (order == Qt::AscendingOrder) {
		switch (column) {
		case ColTrainName:std::sort(lst.begin(), lst.end(), &Train::ltName); break;
		case ColStarting:std::sort(lst.begin(), lst.end(), &Train::ltStarting); break;
		case ColTerminal:std::sort(lst.begin(), lst.end(), &Train::ltTerminal); break;
		case ColType:std::sort(lst.begin(), lst.end(), &Train::ltType); break;
		case ColShow:std::sort(lst.begin(), lst.end(), &Train::ltShow); break;
		case ColMile:std::sort(lst.begin(), lst.end(), &Train::ltMile); break;
		case ColSpeed:std::sort(lst.begin(), lst.end(), &Train::ltTravSpeed); break;
		default:break;
		}
	}
	else {
		switch (column) {
		case ColTrainName:std::sort(lst.begin(), lst.end(), &Train::gtName); break;
		case ColStarting:std::sort(lst.begin(), lst.end(), &Train::gtStarting); break;
		case ColTerminal:std::sort(lst.begin(), lst.end(), &Train::gtTerminal); break;
		case ColType:std::sort(lst.begin(), lst.end(), &Train::gtType); break;
		case ColShow:std::sort(lst.begin(), lst.end(), &Train::gtShow); break;
		case ColMile:std::sort(lst.begin(), lst.end(), &Train::gtMile); break;
		case ColSpeed:std::sort(lst.begin(), lst.end(), &Train::gtTravSpeed); break;
		default:break;
		}
	}
	
	endResetModel();

	if (oldList != lst) {
		if (_undo) {
			//注意压栈后的第一个Redo是不执行的
			_undo->push(new qecmd::SortTrains(oldList, this));
		}
	}
}

QVariant TrainListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole&&orientation==Qt::Horizontal) {
		switch (section) {
		case ColTrainName:return QObject::tr("车次");
		case ColStarting:return QObject::tr("始发站");
		case ColTerminal:return QObject::tr("终到站");
		case ColType:return QObject::tr("类型");
		case ColShow:return QObject::tr("显示");
		case ColMile:return QObject::tr("铺画里程");
		case ColSpeed:return QObject::tr("铺画旅速");
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}

void TrainListModel::undoRedoSort(QList<std::shared_ptr<Train>>& lst)
{
	beginResetModel();
	std::swap(coll.trains(), lst);
	endResetModel();
}

void TrainListModel::redoRemoveTrains(const QList<std::shared_ptr<Train>>& trains,
	const QList<int>& indexes)
{
	beginResetModel();
	//注意：倒序遍历
	for (auto p = indexes.rbegin(); p != indexes.rend(); ++p) {
		std::shared_ptr<Train> train(coll.takeTrainAt(*p));   //move 
	}
	endResetModel();
	emit trainsRemovedRedone(trains);
}

void TrainListModel::undoRemoveTrains(const QList<std::shared_ptr<Train>>& trains, 
	const QList<int>& indexes)
{
	beginResetModel();
	for (int i = 0; i < trains.size(); i++) {
		coll.insertTrainForUndo(indexes.at(i), trains.at(i));
	}
	endResetModel();
	emit trainsRemovedUndone(trains);
}

