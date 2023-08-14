#include "IssueManager.h"

#include "data/train/train.h"
#include "data/rail/railway.h"

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
	return _issues.size();
}

int IssueManager::columnCount(const QModelIndex& parent) const
{
	return ColMAX;
}

QVariant IssueManager::data(const QModelIndex& index, int role) const
{
	if (!index.isValid()) return {};
	const auto& s = _issues.at(index.row());
	if (role == Qt::DisplayRole) {
		switch (index.column())
		{
		case ColTrain: return s.info.train->trainName().full();
		case ColRailway: return s.info.rail->name();
		case ColPos: return s.info.posString();
		case ColType: return IssueInfo::issueTypeToString(s.info.type);
		case ColMsg: return s.info.msg;
		default:
			break;
		}
	}
	else if (role == Qt::ForegroundRole) {
		switch (s.level) {
		case QtInfoMsg: return QColor(Qt::black);
		case QtWarningMsg: return QColor(Qt::darkYellow);
		case QtCriticalMsg:
		case QtFatalMsg: return QColor(Qt::red);
		}
	}
	return {};
}

QVariant IssueManager::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section)
		{
		case IssueManager::ColTrain: return QObject::tr("列车");
			break;
		case IssueManager::ColRailway:return QObject::tr("线路");
			break;
		case IssueManager::ColPos:return QObject::tr("地点");
			break;
		case IssueManager::ColType:return QObject::tr("类型");
			break;
		case IssueManager::ColMsg:return QObject::tr("描述");
			break;
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}

void IssueManager::clear()
{
	beginResetModel();
	_issues.clear();
	endResetModel();
}

void IssueManager::emplaceIssue(PaintIssue&& issue)
{
	if (issue.level == QtDebugMsg)
		return;
	beginInsertRows({}, _issues.size(), _issues.size());
	_issues.emplace_back(std::move(issue));
	endInsertRows();
}

void IssueManager::emplaceIssue(QtMsgType type, const IssueInfo& info)
{
	emplaceIssue(PaintIssue(type, info));
}
