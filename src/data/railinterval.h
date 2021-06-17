/*
 * 新增针对线路区间的抽象
 */
#pragma once
#include <memory>
#include <QList>

class RailStation;

class RailInterval
{
	std::shared_ptr<RailStation> from, to;
	bool down;
public:
	RailInterval(bool down_);

	//这些需要时再写
	RailInterval(const RailInterval&) = delete;
	RailInterval(RailInterval&&) = delete;
	RailInterval& operator=(const RailInterval&) = delete;
	RailInterval& operator=(RailInterval&&) = delete;

	inline std::shared_ptr<RailStation> fromStation() {
		return from;
	}
	inline const std::shared_ptr<RailStation> fromStation()const {
		return from;
	}
	inline std::shared_ptr<RailStation> toStation() {
		return to;
	}
	inline const std::shared_ptr<RailStation> toStation()const {
		return to;
	}

	/// <summary>
	/// 前、后区间，通过车站的信息进行索引
	/// </summary>
	/// <returns></returns>
	std::shared_ptr<RailInterval> prevInterval();

	std::shared_ptr<RailInterval> nextInterval();

};
