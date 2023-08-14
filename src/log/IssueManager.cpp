#include "IssueManager.h"

std::unique_ptr<IssueManager> IssueManager::_instance;

IssueManager* IssueManager::get()
{
	if (!_instance) {
		_instance.reset(new IssueManager);
	}
	return _instance.get();
}

int IssueManager::rowCount(const QModelIndex& parent) const
{
	return int();
}

int IssueManager::columnCount(const QModelIndex& parent) const
{
	return  int();
}

Q_INVOKABLE QVariant IssueManager::data(const QModelIndex& index, int role) const
{
	return  QVariant();
}

void IssueManager::clear()
{
	beginResetModel();
	_issues.clear();
	endResetModel();
}

void IssueManager::emplaceIssue(PaintIssue&& issue)
{
	beginInsertRows({}, _issues.size(), _issues.size());
	_issues.emplace_back(std::move(issue));
	endInsertRows();
}
