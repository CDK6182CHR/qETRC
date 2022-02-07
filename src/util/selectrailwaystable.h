#pragma once

#include <QTableView>
#include <memory>

class Railway;
class RailTableModel;
class Diagram;

/**
 * @brief The SelectRailwaysTable class
 * 2022.02.07
 * 支持多选的选择线路组件。用于从运行图导入线路到数据库。
 */
class SelectRailwaysTable : public QTableView
{
	Q_OBJECT
		Diagram& diagram;
	RailTableModel* const model;
public:
	explicit SelectRailwaysTable(Diagram& diagram, QWidget* parent = nullptr);

	QList<std::shared_ptr<Railway>> selectedRailways()const;

	static QList<std::shared_ptr<Railway>> dlgGetRailways(QWidget* parent,
		Diagram& diagram,
		const QString& title,
		const QString& prompt = "",
		bool* ok = nullptr);

signals:

};

