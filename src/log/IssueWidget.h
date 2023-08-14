#pragma once
#include <QWidget>

class IssueManager;
class QTableView;


/**
 * The Issue/Problem widget displacing (by default) at the bottom of the main window.
 */
class IssueWidget : public QWidget
{
	QTableView* table;
public:
	IssueWidget(QWidget* parent = nullptr);

private:
	void initUI();
	
};