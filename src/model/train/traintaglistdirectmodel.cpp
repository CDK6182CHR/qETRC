#include "traintaglistdirectmodel.h"

#include "data/train/traintagmanager.h"

void TrainTagListDirectModel::refreshData(const TrainTagManager& manager)
{
	QStringList lst;
	for (auto p : manager.tags()) {
		lst.append(p.first);
	}
	setTags(std::move(lst));
}

void TrainTagListDirectModel::clear()
{
	beginResetModel();
	m_tags.clear();
	endResetModel();
}

int TrainTagListDirectModel::rowCount([[maybe_unused]] const QModelIndex& parent) const
{
	return m_tags.size();
}

QVariant TrainTagListDirectModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return {};
	switch (role) {
	case Qt::DisplayRole:
	case Qt::EditRole:
		return m_tags.at(index.row());
	default:
		return {};
	}
}

void TrainTagListDirectModel::setTags(QStringList lst)
{
	beginResetModel();
	m_tags = std::move(lst);
	endResetModel();
}