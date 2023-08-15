#pragma once

#include <QWidget>

class TrainPath;
class QTableView;
class QTextEdit;
class QLineEdit;
class PathModel;
class RailCategory;

class PathEdit :public QWidget
{
	Q_OBJECT;
	RailCategory& railcat;
	TrainPath* _path;

	QLineEdit* edName;
	QTextEdit* edNote;
	QTableView* table;
	PathModel* const model;
public:
	PathEdit(RailCategory& railcat, QWidget* parent = nullptr);
	auto* path() { return _path; }

	void setPath(TrainPath* path);

	void refreshData();

private:
	void initUI();

private slots:
	void actAddBefore(); 
	void actAddAfter();
	void actRemove();
	void actApply();
	void actCancel();
	void actRefresh();
};
