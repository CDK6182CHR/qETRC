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
	static IssueManager* get();

	//auto& issues() { return _issues; }
	auto& issues()const { return _issues; }

	// Í¨¹ý QAbstractTableModel ¼Ì³Ð
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	void clear();

	void emplaceIssue(PaintIssue&& issue);

private:
	IssueManager() = default;
	static std::unique_ptr<IssueManager> _instance;


};