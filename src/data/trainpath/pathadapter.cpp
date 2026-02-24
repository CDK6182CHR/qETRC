#include "pathadapter.h"

#include <QObject>

#include "trainpath.h"
#include "data/train/train.h"
#include "data/rail/railway.h"
#include "data/diagram/trainadapter.h"
#include "data/diagram/trainline.h"
#include "log/IssueManager.h"
#include "util/utilfunc.h"

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

namespace {

	struct SkippedInterval {
		bool found = false;
		int seg_index = -1;
		int secs = -1;
		double mile = -1;
	};

	/**
	 * Find the next station that is bound AFTER the given segment.
	 * Return the information.
	 */
	SkippedInterval find_next_bind_station_after_segment(
		[[maybe_unused]] std::shared_ptr<Train> train, TrainPath* path, const std::vector<PathSegAdapter>& seg_adps,
		int start_seg_index, const AdapterStation& last_bind_st, int period_hours
	) {
		//const auto& start_seg_adp = seg_adps.at(start_seg_index);
		const auto& start_seg = path->segments().at(start_seg_index);
		auto start_seg_rail = start_seg.railway.lock();
		double mile_between = start_seg_rail->mileBetween(
			last_bind_st.railStation.lock(), start_seg_rail->stationByName(start_seg.end_station));

		StationName seg_start_st = start_seg.end_station;
		for (int i = start_seg_index + 1; i < (int)seg_adps.size(); i++) {
			auto& seg_adp = seg_adps.at(i);
			auto& seg = path->segments().at(seg_adp.segIndex());
			auto seg_rail = seg.railway.lock();
			if (seg_adp.type() != PathSegAdapter::EmptyBind) {
				const auto& adp = seg_adp.firstAdapterStation();
				mile_between += seg_rail->mileBetween(seg_rail->stationByName(seg_start_st), adp.railStation.lock());
				int secs = qeutil::secsTo(last_bind_st.trainStation->depart, adp.trainStation->arrive, period_hours);
				return SkippedInterval{
					.found = true, .seg_index = i,
					.secs = secs,
					.mile = mile_between,
				};
			}
			else {
				mile_between += seg.mile;  // <- This mile is valid after calling checkIsValid().
			}
			seg_start_st = seg.end_station;
		}
		return SkippedInterval{ .found = false };
	}
}

int PathAdapter::timetableInterpolationSimple(int period_hours)
{
	int tot_count = 0;
	auto train = m_train.lock();

	double seg_start_mile = 0;

	int start_seg_index = -1;
	Train::StationPtr last_bind_tst = train->timetable().begin();  // This is invalid BEFORE line_started
	Train::StationPtr interp_pos = train->timetable().begin();  // invalid before line_started

	// Current interval for extra polation
	double cur_interval_mile = -1;
	int cur_interval_secs = -1;

	double last_station_interval_mile = -1;
	int last_station_interval_secs = -1;

	StationName seg_start_rst_name = m_path->startStation();

	for (int iseg = 0; iseg < (int)m_segments.size(); iseg++) {
		auto& seg_adp = m_segments.at(iseg);
		auto& seg = m_path->segments().at(seg_adp.segIndex());
		auto rail = seg.railway.lock();

		auto seg_start_rst = rail->stationByName(seg_start_rst_name);
		auto seg_end_rst = rail->stationByName(seg.end_station);

		assert(seg_adp.segIndex() == iseg);

		if (start_seg_index < 0 && seg_adp.type() != PathSegAdapter::EmptyBind) {
			start_seg_index = 0;
		}

		if (start_seg_index < 0) {
			seg_start_rst_name = seg.end_station;
			continue;
		}
		else
			start_seg_index++;

		if (start_seg_index > 1) {
			auto extrap_end_rst = seg_adp.type() == PathSegAdapter::EmptyBind ? 
				seg_end_rst : seg_adp.firstAdapterStation().railStation.lock();
			if (extrap_end_rst != seg_start_rst) {
				// We need to do extrapolation here; the first station is guaranteed to be added by the last segment.
				auto rint = seg_start_rst->dirNextInterval(seg.dir);
				for (; rint; rint = rint->nextInterval()) {
					auto rst = rint->toStation();
					if (seg_adp.type() != PathSegAdapter::EmptyBind && rst == extrap_end_rst) {
						break;
					}

					last_station_interval_mile += rint->mile();
					last_station_interval_secs = static_cast<int>(std::round(
						last_station_interval_mile / cur_interval_mile * cur_interval_secs
					));
					TrainTime tm_interp = last_bind_tst->depart.addSecs(last_station_interval_secs, period_hours);
					interp_pos = train->timetable().insert(std::next(interp_pos), TrainStation(
						rst->name, tm_interp, tm_interp, false, "", QObject::tr("推定")
					));
					//qDebug() << "Extrapolation (head): " << rst->name.toSingleLiteral();
					tot_count++;

					if (seg_adp.type() == PathSegAdapter::EmptyBind && rst == extrap_end_rst) {
						break;
					}
				}
			}
		}

		if (seg_adp.line()) {
			// Has line; first do INTRA interpolation
			tot_count += seg_adp.line()->timetableInterpolationSimple(period_hours);
		}

		// For empty-bind case, this is processed at the beginning of the iteration. 
		// Here we only process the other two cases.
		if (seg_adp.type() != PathSegAdapter::EmptyBind) {
			auto& last_bind_st = seg_adp.lastAdapterStation();
			auto last_bind_rst = last_bind_st.railStation.lock();
			if (last_bind_rst != seg_end_rst) {
				// We need to do extrapolation for the last station
				auto skip_info = find_next_bind_station_after_segment(
					train, m_path, m_segments, iseg, last_bind_st, period_hours
				);
				if (!skip_info.found) {
					// No next unbound station, finish here!
					break;
				}
				cur_interval_mile = skip_info.mile;
				cur_interval_secs = skip_info.secs;
				last_station_interval_mile = 0;
				last_station_interval_secs = 0;
				last_bind_tst = last_bind_st.trainStation;

				if (cur_interval_mile == 0) {
					qWarning() << "zero interval mile encountered: last_bind_st " << last_bind_st.trainStation->name.toSingleLiteral();
					cur_interval_mile = 1;
				}

				auto rint = last_bind_rst->dirNextInterval(seg.dir);
				interp_pos = last_bind_tst;
				for (; rint; rint = rint->nextInterval()) {
					// Each loop process the LATTER station of the interval.
					auto rst = rint->toStation();
					last_station_interval_mile += rint->mile();
					last_station_interval_secs = static_cast<int>(std::round(
						last_station_interval_mile / cur_interval_mile * cur_interval_secs));
					TrainTime tm_interp = last_bind_tst->depart.addSecs(
						last_station_interval_secs, period_hours
					);
					interp_pos = train->timetable().insert(std::next(interp_pos), TrainStation(
						rst->name, tm_interp, tm_interp,
						false, "", QObject::tr("推定"))
					);
					//qDebug() << "Extrapolation (tail): " << rst->name.toSingleLiteral();
					tot_count++;
					if (rst == seg_end_rst) {
						break;
					}
				}
				if (!rint) {
					qDebug() << "Extrapolation-tail WARNING: exceed range. Segment end " << seg.end_station.toSingleLiteral();
				}
			}
		}

		seg_start_mile += seg.mile;
		seg_start_rst_name = seg.end_station;
	}
	return tot_count;
}
