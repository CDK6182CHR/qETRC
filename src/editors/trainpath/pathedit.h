#pragma once

#include <QWidget>

class TrainPath;
class QTableView;
class QTextEdit;
class QLineEdit;
class PathModel;
class RailCategory;
class TrainPathCollection;


class PathEdit :public QWidget
{
	Q_OBJECT;
	RailCategory& railcat;
	TrainPathCollection& pathcoll;
	TrainPath* _path;

	QLineEdit* edName;
	QTextEdit* edNote;
	QTableView* table;
	PathModel* const model;
public:
	PathEdit(RailCategory& railcat, TrainPathCollection& pathcoll, QWidget* parent = nullptr);
	auto* path() { return _path; }

	void setPath(TrainPath* path);

	void refreshData();

private:
	void initUI();

signals:
	void pathApplied(TrainPath* path, std::unique_ptr<TrainPath>& data);

private slots:
	void actAddBefore(); 
	void actAddAfter();
	void actRemove();
	void actApply();
	//void actCancel();
	void actRefresh();
};
