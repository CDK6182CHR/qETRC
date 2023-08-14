#pragma once

#include <QWidget>

class TrainPath;
class QTableView;
class QTextEdit;
class QLineEdit;
class PathModel;

class PathEdit :public QWidget
{
	TrainPath* _path;

	QLineEdit* edName;
	QTextEdit* edNote;
	QTableView* table;
	PathModel* const model;
public:
	PathEdit(QWidget* parent = nullptr);
	auto* path() { return _path; }

	void setPath(TrainPath* path);

	void refreshData();

private:
	void initUI();

private slots:
	void actAdd();
	void actRemove();
	void actApply();
	void actCancel();
	void actRefresh();
};
