#include "pathrulermodel.h"

#include "data/trainpath/trainpath.h"
#include "data/rail/railway.h"
#include "data/rail/ruler.h"
#include "util/utilfunc.h"

PathRulerModel::PathRulerModel(PathRuler ruler, QObject* parent):
	QAbstractTableModel(parent), m_ruler(std::move(ruler)), m_path(m_ruler.path())
{
	computeSegmentData();
}

int PathRulerModel::rowCount(const QModelIndex& parent) const
{
	return (int) m_ruler.segments().size();
}

int PathRulerModel::columnCount(const QModelIndex& parent) const
{
	return ColMAX;
}

QVariant PathRulerModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return {};
	int row = index.row();
	const auto& seg = m_ruler.segments().at(row);
	const auto& pathseg = m_path->segments().at(row);

	if (role == Qt::EditRole || role == Qt::DisplayRole) {
		switch (index.column())
		{
		case PathRulerModel::ColRailName: return pathseg.railwayName();
			break;
		case PathRulerModel::ColStartStation: return m_path->segStartStation(row).toSingleLiteral();
			break;
		case PathRulerModel::ColEndStation: return pathseg.end_station.toSingleLiteral();
			break;
		case PathRulerModel::ColDir: return DirFunc::dirToString(pathseg.dir);
			break;
		case PathRulerModel::ColMile: return QString::number(pathseg.mile, 'f', 3);
			break;
		case PathRulerModel::ColRulerName:
		{
			int rulerIndex = -1;
			if (!seg.ruler.expired()) {
				auto segruler = seg.ruler.lock();
				rulerIndex = segruler->index();
				return role == Qt::EditRole ? QVariant{ rulerIndex } : QVariant{ segruler->name() };
			}
			else {
				return role == Qt::EditRole ? QVariant{ -1 } : QVariant{};
			}
		}
			break;
		case PathRulerModel::ColPassTime: return qeutil::secsToStringHour(m_seg_data.at(row).interval_seconds);
			break;
		case PathRulerModel::ColRunSpeed: return QString::number(m_seg_data.at(row).run_peed, 'f', 3);
			break;
		default: return {};
			break;
		}
	}
    return {};
}

bool PathRulerModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid())
		return false;
	if (role == Qt::EditRole) {
		switch (index.column()) {
		case ColRulerName: 
		{
			int ruler_index = value.toInt();
			if (ruler_index < 0) {
				m_ruler.segmentsRef().at(index.row()).ruler.reset();
			}
			else {
				auto rail = m_path->segments().at(index.row()).railway.lock();
				m_ruler.segmentsRef().at(index.row()).ruler = rail->getRuler(ruler_index);
			}
			return true;
		}
		}
	}
	return QAbstractTableModel::setData(index, value, role);
}

QVariant PathRulerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole) {
			switch (section)
			{
			case PathRulerModel::ColRailName: return tr("线路名");
				break;
			case PathRulerModel::ColStartStation: return tr("起点");
				break;
			case PathRulerModel::ColEndStation: return tr("终点");
				break;
			case PathRulerModel::ColDir: return tr("行别");
				break;
			case PathRulerModel::ColMile: return tr("里程");
				break;
			case PathRulerModel::ColRulerName: return tr("标尺名");
				break;
			case PathRulerModel::ColPassTime: return tr("通通时分");
				break;
			case PathRulerModel::ColRunSpeed: return tr("运行速度");
				break;
			default: return {};
				break;
			}
		}
		else {
			return QAbstractItemModel::headerData(section, orientation, role);
		}
	}
	else {
		return QAbstractTableModel::headerData(section, orientation, role);
	}
}

Qt::ItemFlags PathRulerModel::flags(const QModelIndex& index) const
{
	auto flags = QAbstractTableModel::flags(index);
	if (index.column() == ColRulerName) {
		flags |= Qt::ItemIsEditable;
	}
	return flags;
}

void PathRulerModel::computeSegmentData()
{
	m_seg_data.clear();
	for (size_t i = 0; i < m_ruler.segments().size(); i++) {
		const auto& rulerseg = m_ruler.segments().at(i);
		const auto& pathseg = m_path->segments().at(i);

		if (!m_path->valid() || rulerseg.ruler.expired()) {
			m_seg_data.emplace_back(PathRulerSegmentData{ .valid = false });
		}
		else {
			auto segruler = rulerseg.ruler.lock();
			auto rail = pathseg.railway.lock();
			int tot_interval = segruler->totalInterval(
				rail->stationByName(m_path->segStartStation(i)), 
				rail->stationByName(pathseg.end_station), pathseg.dir);
			double speed = 0;
			if (tot_interval) {
				speed = pathseg.mile / tot_interval * 3600;   // km/h
			}
			m_seg_data.emplace_back(PathRulerSegmentData{ .valid = true, .interval_seconds = tot_interval, .run_peed = speed });
		}
	}
}

void PathRulerModel::setRuler(PathRuler ruler)
{
	beginResetModel();
	m_ruler = std::move(ruler);
	m_path = m_ruler.path();
	computeSegmentData();
	endResetModel();
}

