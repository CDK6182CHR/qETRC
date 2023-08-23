#pragma once
#include <deque>

#include <QAbstractTableModel>

#include "PaintIssue.h"

/**
 * 2023.08.14  Global singeleton for managing issues (mainly generated from painting).
 * The manager is basically a deque of Issue's. The adding operation automatically generates
 * a piece of log (see GlobalLogger).
 * For current implementation, this class serves as a model.
 */
class IssueManager: public QAbstractTableModel
{
	std::deque<PaintIssue> _issues;

public:
	enum Columns {
		ColTrain=0,
		ColRailway,
		ColPos,
		ColType,
		ColMsg,
		ColMAX
	};

	static IssueManager* get();

	//auto& issues() { return _issues; }
	auto& issues()const { return _issues; }

	// 通过 QAbstractTableModel 继承
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role)const override;

	void clear();

	void emplaceIssue(PaintIssue&& issue);

	void emplaceIssue(QtMsgType type, const IssueInfo& info);

	void clearIssuesForTrain(const Train* train);

private:
	IssueManager() = default;
	static std::unique_ptr<IssueManager> _instance;

	void removeIssueAt(int index);
};


#define qeIssueInfo(_issueInfo) do {\
IssueManager::get()->emplaceIssue(QtInfoMsg, _issueInfo); \
qInfo() << _issueInfo.toString(); \
}while(false)

#define qeIssueWarning(_issueInfo) do {\
IssueManager::get()->emplaceIssue(QtWarningMsg, _issueInfo); \
qWarning() << _issueInfo.toString(); \
}while(false)

#define qeIssueCritical(_issueInfo) do {\
IssueManager::get()->emplaceIssue(QtCriticalMsg, _issueInfo); \
qCritical() << _issueInfo.toString(); \
}while(false)

#define qeIssueFatal(_issueInfo) do {\
IssueManager::get()->emplaceIssue(QtFatalMsg, _issueInfo); \
qFatal() << _issueInfo.toString(); \
}while(false)

#define qeIssueDebug(_issueInfo) do {\
IssueManager::get()->emplaceIssue(QtDebugMsg, _issueInfo); \
qDebug() << _issueInfo.toString(); \
}while(false)

