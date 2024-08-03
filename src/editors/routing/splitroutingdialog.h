#pragma once

#include <QDialog>

#include <memory>
#include <vector>
#include <list>
#include <QStandardItemModel>

class Routing;
class TrainCollection;

class SplitRoutingModel :public QStandardItemModel
{
	std::shared_ptr<Routing> m_routing;
public:
	enum Columns {
		ColTrainName=0,
		ColVirtual,
		ColStarting,
		ColTerminal,
		ColLink,
		ColSplit,
		ColNewName,
		ColMAX
	};

	struct Result {
		int index;
		QString new_name;
	};

	SplitRoutingModel(std::shared_ptr<Routing> routing, QObject* parent = nullptr);
	void refreshData();

	std::vector<Result> appliedData()const;
};

class QTableView;
class QLineEdit;
class RoutingNode;

/**
 * Each one corresponds to a *new* routing object.
 */
struct SplitRoutingData {
	QString name;
	int idx_first, idx_last;
};

/**
 * Split routing into pieces. 
 * Mainly implemented by a table. Filling in the corresponding columns at some rows enables
 * spliting the routing *from* the row.
 * This class uses *modal* windows to ensure safety, and the data is not updated since created by design.
 */
class SplitRoutingDialog : public QDialog
{
	Q_OBJECT;
	TrainCollection& m_coll;
	std::shared_ptr<Routing> m_routing;
	SplitRoutingModel* m_model;
	QTableView* m_table;
	QLineEdit* m_edName;

public:
	SplitRoutingDialog(TrainCollection& coll, std::shared_ptr<Routing> routing, QWidget* parent = nullptr);

protected:
	void accept()override;

private:
	void initUI();
	void refreshData();

signals:
	void splitApplied(std::shared_ptr<Routing>, std::vector<SplitRoutingData>& res);
};