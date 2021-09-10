#pragma once
#include "trainevents.h"
#include <QFlags>
#include <set>
#include <map>

/**
 * @brief The TrainGap struct
 * 两列车事件之间的间隔（追踪间隔）。暂时只考虑同向。
 * 采用指针：使得类型容易进行拷贝、赋值等操作。
 * 
 * 此语境下约定：former/latter 指运行方向先到和后到的站 （相对方向）
 * pre/post 指里程小端和里程大端 （绝对方向）
 * left/right 指相关的前一个和后一个车次
 */
struct TrainGap{

    /**
     * 从LSB开始：前事件是否附加，后事件是否附加，特殊 （暂定一个待避时长）
     * 2021.09.08：添加方向Flag：按照左右车次是否为下行编码。
     * GapTypes结合Positions，两个Flag构成完整的间隔描述，可以用来统计最小值。
     */
    enum GapType {
        LeftDown = 0b00100,
        RightDown = 0b01000,
        NoAppend = 0b00000,
        LeftAppend = 0b00001,
        RightAppend = 0b00010,
        BothAppend = LeftAppend | RightAppend,
        Avoid = 0b10000
    };
    Q_DECLARE_FLAGS(GapTypes, GapType)

    std::shared_ptr<const RailStationEvent> left, right;
    GapTypes type;   

    int num;   //数字特征：被踩次数。其他情况直接不使用。

    /**
     * 此版本会根据附加情况计算类型
     */
    TrainGap(std::shared_ptr<const RailStationEvent> left_,
        std::shared_ptr<const RailStationEvent> right_);

    /**
     * trivial的构造函数；直接指定完整的Type，主要用于待避类的构造
     */
    TrainGap(std::shared_ptr<const RailStationEvent> left_,
        std::shared_ptr<const RailStationEvent> right_,GapTypes type_,
        int num_):
        left(left_),right(right_),type(type_),num(num_){}

    RailStationEvent::Positions position()const;
    int secs()const;
    Direction leftDir()const;
    Direction rightDir()const;

    /**
     * 将类型转换为字符串，需考虑上下行、位置等。一次性转换完成。
     */
    QString typeString()const;

    static QString typeToString(const GapTypes& type, const RailStationEvent::Positions& pos);

    /**
     * 给出pos-type的联合字符串表示形式；中间换行
     */
    static QString posTypeToString(const GapTypes& type, const RailStationEvent::Positions& pos);

    /**
     * 左车次这个事件是否为列车运行的站前事件。根据位置和行别判断。
     */
    bool isLeftFormer()const;

    bool isRightFormer()const;

    /**
     * 针对Type的判定：不依赖具体事件。
     */
    static bool isTypeLeftFormer(const GapTypes& type, const RailStationEvent::Positions& pos);
    static bool isTypeRightFormer(const GapTypes& type, const RailStationEvent::Positions& pos);

    bool operator<(const TrainGap& gap)const;

    static bool ltSecs(const std::shared_ptr<TrainGap>& gap1,
        const std::shared_ptr<TrainGap>& gap2);
};

Q_DECLARE_METATYPE(std::shared_ptr<TrainGap>);

struct TrainGapPtrComparator {
    bool operator()(const std::shared_ptr<TrainGap>& gap1,
        const std::shared_ptr<TrainGap>& gap2)const {
        return TrainGap::ltSecs(gap1, gap2);
    }
};

using TrainGapList = std::vector<std::shared_ptr<TrainGap>>;

/**
 * 对列车间隔的完整描述
 */
using TrainGapTypePair = std::pair<RailStationEvent::Positions, TrainGap::GapTypes>;

using TrainGapStatistics = std::map<std::pair<RailStationEvent::Positions, TrainGap::GapTypes>,
    std::set<std::shared_ptr<TrainGap>, TrainGapPtrComparator>>;

Q_DECLARE_OPERATORS_FOR_FLAGS(TrainGap::GapTypes)
