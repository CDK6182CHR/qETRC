#pragma once
#include "trainevents.h"
#include <QFlags>
#include <set>
#include <map>

#include "data/common/direction.h"

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
     * -------------------------------
     * 2023.06.15: 添加pre/post标记。自此，GapTypes类即可独立描述间隔类型，无需另配Position。
     * 保证：Position标记在LSB起的第二个字节。
     * 此修订在注释中记作API V2。
     */
    enum GapType {
        LeftDown = 0b00100,
        RightDown = 0b01000,
        NoAppend = 0b00000,
        LeftAppend = 0b00001,
        RightAppend = 0b00010,
        BothAppend = LeftAppend | RightAppend,
        Avoid = 0b10000,
        LeftPre = 0x0100,
        LeftPost = 0x0200,
        RightPre = 0x0400,
        RightPost = 0x0800,

        // masks for part comparison
        BasicMask = 0x00FF,
        LeftPositionMask = 0x0300,
        RightPositionMask = 0x0C00,
        PreMask = LeftPre | RightPre,
        PostMask = LeftPost | RightPost,
    };
    Q_DECLARE_FLAGS(GapTypesV2, GapType)

    std::shared_ptr<const RailStationEvent> left, right;
    GapTypesV2 type;   

    int num;   //数字特征：被踩次数。其他情况直接不使用。
    int period_hours;

    /**
     * 此版本会根据附加情况计算类型
     * 2023.06.15  API V2  修改实现。
     * 注意，由于增加了对侧的情况，通过的事件现在有一定的歧义性。既可以是同侧，又可以是对侧。
     * 但是由于其数值是一样的，似乎没必要特别处理。
     * 在分析时，暂定仍然按照旧的模式进行。
     * 在贪心推线设置约束时可能需要注意一下判定逻辑。（例如）通发间隔，既包含同侧的，也包含对侧的。
     */
    TrainGap(std::shared_ptr<const RailStationEvent> left_,
        std::shared_ptr<const RailStationEvent> right_, int period_hours);

    /**
     * trivial的构造函数；直接指定完整的Type，主要用于待避类的构造
     */
    TrainGap(std::shared_ptr<const RailStationEvent> left_,
        std::shared_ptr<const RailStationEvent> right_,GapTypesV2 type_,
        int num_, int period_hours_):
        left(left_),right(right_),type(type_),num(num_),period_hours(period_hours_){}

    RailStationEvent::Positions position()const;
    int secs()const;
    Direction leftDir()const;
    Direction rightDir()const;

    /**
     * 将类型转换为字符串，需考虑上下行、位置等。一次性转换完成。
     */
    QString typeString()const;

    /**
     * 2023.06.15  For new GapTypes, convert event pos to GapType pos.
     * Return the flag contains only the corresponding bits, with all other bits zero.
     * Benefitted from the fact that the position flags locates in the second byte in GapTypes from LSB, 
     * the implementation is simply bit shift.
     */
    static GapTypesV2 posToGapPosLeft(RailStationEvent::Positions pos);
    static GapTypesV2 posToGapPosRight(RailStationEvent::Positions pos);
    
    static RailStationEvent::Positions gapTypeToPosLeft(GapTypesV2 type);
    static RailStationEvent::Positions gapTypeToPosRight(GapTypesV2 type);

    static GapTypesV2 setLeftPos(GapTypesV2 type, RailStationEvent::Positions pos);
    static GapTypesV2 setRightPos(GapTypesV2 type, RailStationEvent::Positions pos);

    /**
     * 2023.06.16  判断是否为同侧间隔（站前站后）。
     * 如果两边的位置有交集，一律优先判为同侧。（即：通发可算作同侧）
     */
    static bool isSamePositionType(GapTypesV2 type);

    //static QString typeToString(const GapTypes& type, const RailStationEvent::Positions& pos);

    /**
     * 2023.06.15 for API V2, similar to the function in V1, 
     * returns only the single-line result.
     */
    static QString typeToString(const GapTypesV2& type);

    /**
     * 2023.06.15  
     * similar to RailStationEvent::posToString, but different processing for 0x0
     */
    static QString prefixPosToString(const GapTypesV2& type);

    /**
     * 给出pos-type的联合字符串表示形式；中间换行
     * 2023.06.15: update for V2 API
     */
    static QString posTypeToString(const GapTypesV2& type);

    /**
     * 左车次这个事件是否为列车运行的站前事件。根据位置和行别判断。
     */
    bool isLeftFormer()const;

    bool isRightFormer()const;

    /**
     * 针对Type的判定：不依赖具体事件。
     * 下行站前或上行站后；就是先经过的那个；也即到或者通
     */
    static bool isTypeLeftFormer(const GapTypesV2& type);

    /**
     * 下行站后或者上行站前；发或者通
     */
    static bool isTypeRightFormer(const GapTypesV2& type);

    bool operator<(const TrainGap& gap)const;

    static bool ltSecs(const std::shared_ptr<TrainGap>& gap1,
        const std::shared_ptr<TrainGap>& gap2);

    /**
     * 2022.03.06
     * 根据前后两事件，判断两个事件之间的间隔类型。
     * 如果两事件之间不构成间隔，则返回空。
     * ----------------
     * 2023.06.15  update to API V2
     * @param singleLine  单线标记。此标记下判定反向间隔，否则忽略。
     */
    static std::optional<TrainGap::GapTypesV2>
        gapTypeBetween(const RailStationEventBase& left, const RailStationEventBase& right, 
            bool singleLine);

    /**
     * 2022.03.11
     * 生成间隔类型（不考虑站前站后）
     */
    static GapTypesV2 generateType(Direction left_dir, Direction right_dir,
        bool left_append, bool right_append);

    static GapTypesV2 genDirGapType(Direction left_dir, Direction right_dir, GapTypesV2 base);

    /**
     * 2022.03.11
     * 生成（用于排图约束的）所有可能间隔类型。
     * 按单双线区分
     */
    static std::vector<TrainGap::GapTypesV2>
        allPossibleGaps(bool singleLine);
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
 * 2023.06.15: removed for API V2
 */
//using TrainGapTypePair = std::pair<RailStationEvent::Positions, TrainGap::GapTypes>;

using TrainGapStatistics = std::map<TrainGap::GapTypesV2,
    std::set<std::shared_ptr<TrainGap>, TrainGapPtrComparator>>;

Q_DECLARE_OPERATORS_FOR_FLAGS(TrainGap::GapTypesV2)
