#include "trainnamesortproxymodel.h"

#include <algorithm>
#include "util/utilfunc.h"

TrainNameSortProxyModel::TrainNameSortProxyModel(std::vector<int> cols, QObject* parent):
	QSortFilterProxyModel(parent), m_cols(std::move(cols))
{
	
}

bool TrainNameSortProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
	if (auto itr = std::ranges::find(m_cols, left.column()); itr != m_cols.end()) {
		// TrainName sorting
		return qeutil::trainNameLess(left.data(Qt::DisplayRole).toString(), 
			right.data(Qt::DisplayRole).toString());
	}
	else {
		return QSortFilterProxyModel::lessThan(left, right);
	}
}
