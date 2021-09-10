#pragma once

#include <optional>
#include <map>
#include <QString>

#include "data/common/stationname.h"
#include "railinterval.h"
#include <memory>

/**
 * @brief The PassedDirection enum
 * 车站通过情况（单向站情况）
 */
enum class PassedDirection {
    NoVia = 0b00,
    DownVia = 0b01,
    UpVia = 0b10,
    BothVia = 0b11
};

namespace qeutil {
    inline QString passDirStr(PassedDirection dir) {
        switch (dir)
        {
        case PassedDirection::NoVia: return QObject::tr("不通过");
            break;
        case PassedDirection::DownVia:return QObject::tr("下行");
            break;
        case PassedDirection::UpVia:return QObject::tr("上行");
            break;
        case PassedDirection::BothVia:return QObject::tr("上下行");
            break;
        default:return "";
            break;
        }
    }
    inline QString passDirStr(int dir) {
        return passDirStr(static_cast<PassedDirection>(dir));
    }

    inline Qt::CheckState boolToCheckState(bool d) {
        return d ? Qt::Checked : Qt::Unchecked;
    }
}


class QJsonObject;

/**
 * 原pyETRC项目的LineStation类
 * 原pyETRC中直接用dict做的，这里都采用struct的模式简单实现
 */
class RailStation
{
    friend class Railway;
    friend class RailInterval;
    //前后区间指针，暂定由Railway类负责维护
    //todo: 详细维护算法
    std::shared_ptr<RailInterval> downPrev, downNext;
    std::shared_ptr<RailInterval> upPrev, upNext;
    
public:
    StationName name;
    double mile;
    int level;
    std::optional<double> counter;

    /**
     * 车站位置的纵坐标（像素），此数据不写入文件。
     * 采用相对位置，即本线第一个站为0.
     */
    std::optional<double> y_value;
    PassedDirection direction;
    bool _show;
    bool passenger,freight;
    QList<QString> tracks;
    RailStation(const StationName& name_,
        double mile_,
        int level_ = 4,
        std::optional<double> counter_ = std::nullopt,
        PassedDirection direction_ = PassedDirection::BothVia, bool show = true,
        bool passenger = false, bool freight = false
    );
    RailStation(const QJsonObject& obj);

    /// <summary>
    /// 注意：shared_ptr的默认复制语义是不正确的
    /// 重新实现为：构造空shared_ptr
    /// </summary>
    RailStation(const RailStation&);

    RailStation(RailStation&& rs) = default;

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    QString counterStr()const;

    inline double counterMile()const{
        return counter.value_or(mile);
    }

    inline bool isDownVia()const {
        return direction == PassedDirection::DownVia ||
            direction == PassedDirection::BothVia;
    }

    inline bool isUpVia()const {
        return direction == PassedDirection::UpVia ||
            direction == PassedDirection::BothVia;
    }

    inline constexpr bool isDirectionVia(Direction _dir)const {
        switch (_dir) {
        case Direction::Down:return isDownVia();
        case Direction::Up:return isUpVia();
        default:
            return false;
        }
    }

    std::shared_ptr<RailStation> downAdjacent();
    std::shared_ptr<RailStation> upAdjacent();

    std::shared_ptr<RailStation> dirAdjacent(Direction _dir) {
        switch (_dir) {
        case Direction::Down: return downAdjacent();
        case Direction::Up: return upAdjacent();
        default:return nullptr;
        }
    }

    std::shared_ptr<const RailStation> dirAdjacent(Direction _dir)const {
        return const_cast<RailStation*>(this)->dirAdjacent(_dir);
    }

    std::shared_ptr<const RailStation> downAdjacent()const {
        return const_cast<RailStation*>(this)->downAdjacent();
    }
    std::shared_ptr<const RailStation> upAdjacent()const {
        return const_cast<RailStation*>(this)->upAdjacent();
    }

    bool isAdjacentWith(const std::shared_ptr<const RailStation>& another) const {
        return downAdjacent() == another || upAdjacent() == another;
    }

    /**
     * Returns `RailInterval` between `this` and `another` if this is adjacent with another,
     * otherwise null.
     * Seealso `isAdjacentWith()`.
     */
    std::shared_ptr<RailInterval>
        adjacentIntervalTo(std::shared_ptr<const RailStation> another);

    inline bool hasDownAdjacent()const{return downNext.get();}
    inline bool hasUpAdjacent()const{return upNext.get();}

    inline std::shared_ptr<RailInterval> dirNextInterval(Direction _dir) {
        switch (_dir) {
        case Direction::Down:
            return downNext;
        case Direction::Up:
            return upNext;
        default:
            return nullptr;
        }
    }

    inline std::shared_ptr<RailInterval> dirPrevInterval(Direction _dir) {
        switch (_dir) {
        case Direction::Down:
            return downPrev;
        case Direction::Up:
            return upPrev;
        default:
            return nullptr;
        }
    }

    inline std::shared_ptr<const RailInterval> dirNextInterval(Direction _dir)const {
        return const_cast<RailStation*>(this)->dirNextInterval(_dir);
    }

    inline std::shared_ptr<const RailInterval> dirPrevInterval(Direction _dir)const {
        return const_cast<RailStation*>(this)->dirPrevInterval(_dir);
    }

    inline bool operator<(double mile_)const {
        return this->mile < mile_;
    }
};

inline bool operator<(double mile, const RailStation& rs) {
    return mile < rs.mile;
}

class RailStationMileLess {
public:
    bool operator() (const std::shared_ptr<RailStation>& st, double mile)const {
        return st->mile < mile;
    }
    bool operator() (double mile, const std::shared_ptr<RailStation>& st)const {
        return mile < st->mile;
    }
    bool operator() (const std::shared_ptr<RailStation>& st1,
        const std::shared_ptr<RailStation>& st2)const {
        return st1->mile < st2->mile;
    }
};

Q_DECLARE_METATYPE(std::shared_ptr<RailStation>)
Q_DECLARE_METATYPE(std::shared_ptr<const RailStation>)
