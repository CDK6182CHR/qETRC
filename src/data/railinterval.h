/*
 * 新增针对线路区间的抽象
 */
#pragma once
#include <memory>
#include <QList>



class RailStation;
class RulerNode;

class RailInterval:
	std::enable_shared_from_this<RailInterval>
{
	friend class Railway;
	//注意一律按照有向边约定
	std::shared_ptr<RailStation> from, to;
	bool down;

    QList<std::shared_ptr<RulerNode>> _rulerNodes;

	RailInterval(bool down_);

    /// <summary>
    /// 注意不能为RailStation添加指针
    /// </summary>
    RailInterval(bool down_, std::shared_ptr<RailStation> from_,
        std::shared_ptr<RailStation> to_);

    static std::shared_ptr<RailInterval> construct(bool down,
                                                   std::shared_ptr<RailStation> from,
                                                   std::shared_ptr<RailStation> to);

public:

	//这些需要时再写
	RailInterval(const RailInterval&) = delete;
	RailInterval(RailInterval&&) = default;
	RailInterval& operator=(const RailInterval&) = delete;
	RailInterval& operator=(RailInterval&&) = default;

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
	/// 按照运行方向定义
	/// </summary>
	std::shared_ptr<RailInterval> prevInterval()const;

	std::shared_ptr<RailInterval> nextInterval()const;

	/// <summary>
	/// 合并两连续区间的数据
	/// 两区间应当是连续并且同向的
	/// 包括指针处理
	/// </summary>
	/// <param name="next">下一区间（按行别定义的，运行方向）</param>
	/// <returns>新的对象</returns>
	RailInterval mergeWith(const RailInterval& next)const;

    double mile()const;

    inline std::shared_ptr<const RulerNode> rulerNodeAt(int i)const{
        return _rulerNodes.at(i);
    }

    inline std::shared_ptr<RulerNode> rulerNodeAt(int i){
        return _rulerNodes.at(i);
    }

    inline bool isDown()const{return down;}

    template <typename Node>
    inline std::shared_ptr<const Node> getDataAt(int i)const;

    template <typename Node>
    inline std::shared_ptr<Node> getDataAt(int i);

    template <>
    inline std::shared_ptr<const RulerNode> getDataAt(int i)const{
        return _rulerNodes.at(i);
    }

    template <>
    inline std::shared_ptr<RulerNode> getDataAt(int i){
        return _rulerNodes[i];
    }

};
