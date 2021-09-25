#include "railnet.h"
#include "data/rail/railway.h"
#include "data/rail/railcategory.h"
#include "data/rail/forbid.h"


void RailNet::fromRailCategory(const RailCategory* cat)
{
	foreach(const auto & sub, cat->subCategories()) {
		fromRailCategory(sub.get());
	}
	foreach(const auto & rail, cat->railways()) {
		addRailway(rail.get());
	}
}

const RailNet::rail_ret_t RailNet::rail_ret_t::null{};

RailNet::rail_ret_t RailNet::sliceBySinglePath(QVector<QString> points,
	bool withRuler,
	QString* report, int rulerCount)const
{
	auto [rdown, pdown] = singleRailFromPath(points, withRuler, report);
	if (!rdown) {
		report->prepend(QObject::tr("寻找正向径路错误: "));
		return rail_ret_t::null;
	}

	std::reverse(points.begin(), points.end());

	auto [rup, pup] = singleRailFromPath(points, withRuler, report);
	if (!rup) {
		report->prepend(QObject::tr("寻找反向径路错误："));
		return rail_ret_t::null;
	}
	rdown->mergeCounter(*rup);
	rdown->filtRulerByCount(rulerCount);
	return { rdown,pdown,pup };
}

RailNet::rail_ret_t RailNet::sliceByDoublePath(
	const QVector<QString>& downPoints, const QVector<QString>& upPoints,
	bool withRuler, QString* report, int rulerCount)const
{
	auto [rdown, pdown] = singleRailFromPath(downPoints, withRuler, report);
	if (!rdown) {
		report->prepend(QObject::tr("寻找正向径路错误: "));
		return rail_ret_t::null;
	}

	auto [rup, pup] = singleRailFromPath(upPoints, withRuler, report);
	if (!rup) {
		report->prepend(QObject::tr("寻找反向径路错误："));
		return rail_ret_t::null;
	}
	rdown->mergeCounter(*rup);
	rdown->filtRulerByCount(rulerCount);
	return { rdown,pdown,pup };
}

RailNet::rail_ret_t RailNet::sliceBySymmetryPath(const QVector<QString>& points,
	bool withRuler,
	QString* report, int rulerCount)const
{
	auto [rdown, pdown] = singleRailFromPath(points, withRuler, report);
	if (!rdown) {
		report->prepend(QObject::tr("寻找径路错误: "));
		return rail_ret_t::null;
	}

	rdown->symmetrize();
	rdown->filtRulerByCount(rulerCount);
	return { rdown,pdown,{} };
}

QString RailNet::pathToString(const path_t& path) const
{
	if (path.empty())return {};
	QString res;
	double mile = 0;
	double lastMile = 0;
	std::shared_ptr<const vertex> lastVert{};
	QString lastRailName;

	// 循环中：如果线名不同，则总结上一步，开辟新的。
	for (const auto& ed : path) {
		if (ed->data.railName != lastRailName) {
			// 总结上一段径路
			if (!lastRailName.isEmpty()) {
				// 非第一次
				res.append(intervalPathToString(lastVert, ed->from.lock(),
					lastRailName, mile - lastMile));
				res.append('\n');
			}
			lastRailName = ed->data.railName;
			lastMile = mile;
			lastVert = ed->from.lock();
		}
		mile += ed->data.mile;
	}

	if (lastVert != path.back()->to.lock()) {
		// 最后一段的处理
		res.append(intervalPathToString(lastVert, path.back()->to.lock(),
			lastRailName, mile - lastMile));
	}
	return res;
}

std::pair<std::shared_ptr<Railway>, RailNet::path_t>
RailNet::singleRailFromPath(const QVector<QString>& points,
	bool withRuler,
	QString* report)const
{
	if (points.size() <= 1) {
		report->append(QObject::tr("至少需给出两个关键点"));
		return std::make_pair(nullptr, path_t{});
	}
	auto res = std::make_shared<Railway>(
		QString("%1-%2").arg(points.front(), points.back()));
	res->ensureForbids(Forbid::FORBID_COUNT);
	path_t path;

	QMap<QString, int> rulerMap;

	auto prev = points.begin();
	auto prst = find_vertex(*prev);
	if (!prst) {
		report->append(QObject::tr("径路首站%1不在图中").arg(*prev));
		return { nullptr,{} };
	}
	res->appendStation(prst->data.name, 0, prst->data.level, std::nullopt,
		PassedDirection::DownVia, true, prst->data.passenger,
		prst->data.freight);

	double mile = 0;
	for (auto p = std::next(prev); p != points.end(); prev = p, ++p) {
		auto curst = find_vertex(*p);
		if (!curst) {
			report->append(QObject::tr("径路中间站%1不在图中").arg(*p));
			return { nullptr,{} };
		}
		auto ret = sssp(prst, &GraphInterval::getMile);
		auto subpath = dump_path(prst, curst, ret);
		path.insert(path.end(), subpath.begin(), subpath.end());

		if (subpath.empty()) {
			report->append(QObject::tr("区间[%1->%2]不可达").arg(
				*prev, *p));
			return { nullptr,{} };
		}

		for (const auto& e : subpath) {
			const auto& d = e->to.lock()->data;
			mile += e->data.mile;
			res->appendStation(d.name, mile, d.level, std::nullopt,
				PassedDirection::DownVia, true,
				d.passenger, d.freight);
			auto railint = res->lastDownInterval();
			const auto& dint = e->data;
			// 天窗
			for (int i = 0; i < std::min(res->forbids().size(),
				dint.forbidNodes.size()); i++) {
				auto node = railint->getForbidNode(res->forbids().at(i));
				dint.forbidNodes.at(i).exportToNode(*node);
			}

			if (withRuler) {
				// 标尺
				foreach(const auto & rn, dint.rulerNodes) {
					int idx;
					if (auto itr = rulerMap.find(rn.name); itr != rulerMap.end()) {
						idx = itr.value();
					}
					else {
						idx = rulerMap.size();
						rulerMap.insert(rn.name, idx);
						res->addEmptyRuler(rn.name, true);
					}
					auto ruler = res->getRuler(idx);
					auto node = railint->getRulerNode(ruler);
					rn.exportToNode(*node);
				}
			}
		}

		prst = curst;
	}

	return std::make_pair(res, path);

}

QString RailNet::intervalPathToString(std::shared_ptr<const vertex> pre, 
	const std::shared_ptr<const vertex> post, const QString& railName, double mile) const
{
	return QString("%1-%2  %3  %4 km").arg(pre->data.name.toSingleLiteral(),
		post->data.name.toSingleLiteral(), railName, QString::number(mile, 'f', 3));
}

void RailNet::addRailway(const Railway* railway)
{
	std::shared_ptr<vertex> pre{};
	for (auto p = railway->firstDownInterval(); p; p = railway->nextIntervalCirc(p)) {
		if (!pre) {
			pre = emplace_vertex(p->fromStation()->name, *(p->fromStation()));
		}
		auto post = emplace_vertex(p->toStation()->name, *(p->toStation()));
		auto ed = emplace_edge(pre, post, railway->name(), *p);
		pre = std::move(post);
		if (p->mile() < 0) {
			qDebug() << "RailNet::addRailway: ERROR: negative mile, set to 0 " << p->mile() <<
				*p << " @ " << railway->name() << Qt::endl;
			ed->data.mile = 0;
		}
	}
}
