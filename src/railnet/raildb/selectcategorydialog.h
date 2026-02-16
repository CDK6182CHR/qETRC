#pragma once

#include <memory>
#include <deque>
#include <QDialog>

class RailDBModel;
class RailDB;
class QTreeView;
class QCheckBox;

namespace navi {
	class AbstractComponentItem;
}


/**
 * 2026.02.16  Dialog for selecting a category from rail-DB.
 * A tree-view for the categories is displayed.
 */
class SelectCategoryDialog : public QDialog
{
	Q_OBJECT;

	std::shared_ptr<RailDB> m_raildb;
	RailDBModel* m_model;

	QCheckBox* m_ckRoot;
	QTreeView* m_tree;

	/**
	 * The selected item path. Empty for root.
	 */
	std::deque<int> m_selectedPath;

public:
	explicit SelectCategoryDialog(std::shared_ptr<RailDB> raildb, const QString& prompt, QWidget* parent = nullptr);

	/**
	 * In current impl, we choose to store the selection result in the dialog itself.
	 * This will be convenient for the blocked get function.
	 */
	void accept() override;

	struct GetCategoryResult {
		bool accepted;
		std::deque<int> path;
	};

	/**
	 * Returns the selected category path. Empty for not selected or canceled.
	 */
	static GetCategoryResult getCategory(std::shared_ptr<RailDB> raildb, const QString& prompt, QWidget* parent = nullptr);

private:
	void initUI(const QString& prompt);
};

