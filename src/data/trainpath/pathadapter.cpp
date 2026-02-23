#include "pathadapter.h"

#include <QObject>

#include "trainpath.h"
#include "data/train/train.h"
#include "data/rail/railway.h"
#include "data/diagram/trainadapter.h"
#include "data/diagram/trainline.h"
#include "log/IssueManager.h"

PathAdapter::PathAdapter(std::weak_ptr<Train> train, TrainPath* path, std::vector<PathSegAdapter> segments):
	m_train(train), m_path(path), m_segments(std::move(segments)), m_valid(true)
{
}

PathAdapter::PathAdapter(std::weak_ptr<Train> train, TrainPath* path):
	m_train(train), m_path(path), m_segments(), m_valid(false)
{
}

namespace {

	/**
	 * Return the relative position of the station comparing to the mile range (specified by segment).
	 * Specifically:
	 * -1  if the station is encountered BEFORE the range,
	 * 0   if the station is in the range, and
	 * +1  if the station is encountered AFTER the range.
	 * Here, before/after is in the view of the train running direction.
	 * Mind: how to process the counter mile?
	 */
	int stationInSegment(double mile_start, double mile_end, Direction dir, const RailStation& st)
	{
		if (dir == Direction::Down) {
			if (st.mile < mile_start)
				return -1;
			else if (st.mile <= mile_end)
				return 0;
			else
				return 1;
		}
		else if (dir == Direction::Up) {
			if (st.mile > mile_start)
				return -1;
			else if (st.mile >= mile_end)
				return 0;
			else
				return 1;
		}
		else {
			qCritical() << "Invalid direction " << static_cast<int>(dir);
			return 0;
		}
	}
}

PathAdapter PathAdapter::bind(std::shared_ptr<Train> train, TrainPath* path)
{
	std::vector<PathSegAdapter> segments;

	if (!path->valid()) {
		qeIssueCritical(IssueInfo(IssueInfo::InvalidPath, train, {}, {}, QObject::tr("列车径路%1不可用，"
			"此径路将被忽略").arg(path->name())));
		return PathAdapter(train, path);
	}

	std::map<Railway*, std::shared_ptr<TrainAdapter>> adp_map;

	bool first_matched_encountered = false;
	bool has_any_match_before_this_line = false;
	StationName start_station = path->startStation();
	auto tit_last_bind = train->timetable().begin();


	for (int iseg = 0; iseg < (int) path->segments().size(); iseg++) {
		auto& seg = path->segments().at(iseg);
		auto rail = seg.railway.lock();
		auto rst_start = rail->stationByName(start_station);
		auto rst_end = rail->stationByName(seg.end_station);
		double mile_start = rst_start->mile;
		double mile_end = rst_end->mile;

		std::shared_ptr<TrainAdapter> adp{};
		if (auto itr = adp_map.find(rail.get()); itr != adp_map.end()) {
			adp = itr->second;
		}
		else {
			adp = std::shared_ptr<TrainAdapter>(new TrainAdapter(train, rail));
			adp_map.emplace(rail.get(), adp);
		}

		auto line = std::make_shared<TrainLine>(*adp);
		line->_dir = seg.dir;

		// 2024.03.20: for one-station line case, the deletion of such line causes the restoring of tit_last_bind
		auto tit_last_bind_before_loop = tit_last_bind;
		// loop over train stations
		for (auto tit = tit_last_bind; tit != train->timetable().end(); ++tit) {
			auto rst = rail->stationByGeneralName(tit->name);
			if (!rst) {
				// station not in railway, just pass
				continue;
			}

			// now, rst is not empty
			auto ipos = stationInSegment(mile_start, mile_end, seg.dir, *rst);
			if (ipos < 0) {
				// before range, nothing to do
			}
			else if (ipos > 0) {
				// out of range, break current loop
				break;
			}
			else {  // ipos == 0, to be bound
				line->addStation(tit, rst);
				tit_last_bind = tit;
				first_matched_encountered = true;
			}

			if (rst->name == seg.end_station) {
				// The last station in the segment is found
				break;
			}
		}

		if (line->count() > 1) {
			// For TrainPath-guided binding, currently, one-station line is allowed
			// (if this is not, the deletion of last line should change tit_last_bind)
			// 2024.03.20  CHANGE THIS: one-station line is now NOT allowed
			adp->lines().append(line);

			// now, a special case, for the same-railway-turn-back case, set the label validity.
			if (adp->lines().size() > 1) {
				auto last_line = adp->lines().at(adp->lines().size() - 2);
				if (last_line->railway() == rail && last_line->stations().back() == line->stations().front() &&
					last_line->dir() != line->dir()) {
					last_line->_endLabel = false;
					line->_startLabel = false;
				}
			}

			// 2026.02.22: detect the possible lost of information at the endings
			if (has_any_match_before_this_line) {
				// Possible lost at the BEGINNING of this segment
				auto line_start_rst = line->stations().front().railStation.lock();
				if (line_start_rst != rst_start) {
					qeIssueWarning(IssueInfo(IssueInfo::PathBindLateStart, train, rail, line_start_rst,
						QObject::tr("根据列车径路铺画运行线时，本段列车径路[%1-%2]的可用时刻表首站为[%3]，"
							"可能造成运行线部分缺失。请考虑补充节点站[%1]时刻信息。")
						.arg(start_station.toSingleLiteral(), seg.end_station.toSingleLiteral(),
							line_start_rst->name.toSingleLiteral())));
				}
			}

			if (line->stations().back().trainStation != std::prev(train->timetable().end())) {
				// Possible lost at the ENDING of this segment
				auto line_end_rst = line->stations().back().railStation.lock();
				if (line_end_rst != rst_end) {
					qeIssueWarning(IssueInfo(IssueInfo::PathBindEarlyStop, train, rail, line_end_rst,
						QObject::tr("根据列车径路铺画运行线时，本段列车径路[%1-%2]的可用时刻表末站为[%3]，"
							"可能造成运行线部分缺失。请考虑补充节点站[%2]时刻信息。")
						.arg(start_station.toSingleLiteral(), seg.end_station.toSingleLiteral(),
							line_end_rst->name.toSingleLiteral())));
				}
			}
			segments.emplace_back(iseg, line);
		}
		else if (line->count() == 1) {
			// 2024.03.20: for this case, restore tit_last_bind, since the previous bind is withdrawn
			tit_last_bind = tit_last_bind_before_loop;
			qeIssueWarning(IssueInfo(IssueInfo::PathBindSingleStation, train, rail,
				line->stations().front().railStation.lock(),
				QObject::tr("根据列车径路铺画运行线时，本段列车径路[%1-%2]仅有一个铺画站，无法铺画本段运行线，可能造成运行线缺失")
				.arg(start_station.toSingleLiteral(), seg.end_station.toSingleLiteral())));
			segments.emplace_back(iseg, line->stations().front());
		}
		else {
			// 2026.02.22: for this case, no binding at all.
			if (first_matched_encountered) {
				qeIssueWarning(IssueInfo(IssueInfo::PathBindSkipped, train, rail,
					rst_start,
					QObject::tr("根据列车径路铺画运行线时，本段列车径路[%1-%2]无铺画站，无法铺画本段运行线，可能造成运行线缺失")
					.arg(start_station.toSingleLiteral(), seg.end_station.toSingleLiteral())));
			}
			segments.emplace_back(iseg);
		}

		start_station = seg.end_station;
		if (first_matched_encountered) {
			// We set this key for the later checking of starting station of the next segment.
			// Currently, we consider the path starts after any matching (even a discarded one in the single-matching case).
			has_any_match_before_this_line = true;
		}
	}

	for (auto itr = adp_map.begin(); itr != adp_map.end(); ++itr) {
		auto adp = itr->second;
		if (!adp->isNull()) {
			train->adapters().append(adp);
		}
	}

	return PathAdapter(train, path, std::move(segments));
}

int PathAdapter::timetableInterpolationSimple(int period_hours)
{
	int tot_count = 0;
	auto train = m_train.lock();

	double seg_start_mile = 0;

	int start_seg_index = -1;
	Train::StationPtr last_station = train->timetable().begin();  // This is invalid BEFORE line_started

	// Current interval for extra polation
	double cur_interval_mile = -1;
	int cur_interval_secs = -1;

	double last_station_interval_mile = -1;
	int last_station_interval_secs = -1;

	for (int iseg = 0; iseg < (int)m_segments.size(); iseg++) {
		auto& seg_adp = m_segments.at(iseg);
		auto& seg = m_path->segments().at(seg_adp.segIndex());
		assert(seg_adp.segIndex() == iseg);

		if (start_seg_index < 0 && seg_adp.type() != PathSegAdapter::EmptyBind) {
			start_seg_index = 0;
		}

		if (start_seg_index < 0)
			continue;
		else
			start_seg_index++;

		if (start_seg_index > 1) {
			// TODO: EXTRApolation at the beginning of the segment, if required
		}

		if (seg_adp.line()) {
			// Has line; first do INTRA interpolation
			tot_count += seg_adp.line()->timetableInterpolationSimple(period_hours);
		}

		// TODO: find next station after this line to do extrapolation
		// remember to update last_station and related data

		seg_start_mile += seg.mile;
	}
	return tot_count;
}
