#pragma once
#include <memory>
#include <QString>

struct RailStationEvent;
/**
 * 区间冲突检测报告
 */
struct IntervalConflictReport {
    enum ConflictType {
        NoConflict = 0,
        LeftConflict,    // 左冲突：与待排线位出发站左侧已有事件冲突，即追上了别人。
        RightConflict,   // 右冲突：与待排线位出发站右侧已有事件冲突，即被别人追上，或单线会车。
        CoverConflict,   // 共线冲突。没有更好的解决办法，只能往后平移1秒再重来
    } type;

    /**
     * 与冲突相关的事件。
     * 左冲突为到站事件；右冲突为发站事件；其他为空
     */
    std::shared_ptr<struct RailStationEvent> conflictEvent;

    static QString typeToString(ConflictType type);
};
