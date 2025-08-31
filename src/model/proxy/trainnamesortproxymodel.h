#pragma once

#include <QSortFilterProxyModel>
#include <vector>

/**
 * 2025.08.31  EXPERIMENTAL
 * Proxy model for sorting the TrainName's according to configurations.
 * The selected columns in m_cols will be considered to be train names.
 */
class TrainNameSortProxyModel: public QSortFilterProxyModel
{
	Q_OBJECT;

	std::vector<int> m_cols;
public:
	TrainNameSortProxyModel(std::vector<int> cols, QObject* parent = nullptr);

protected:
	bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
};