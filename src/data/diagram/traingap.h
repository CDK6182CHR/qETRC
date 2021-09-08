#pragma once
#include "trainevents.h"
#include <QFlags>

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
     * 2021.09.08：
     */
    enum GapType {
        NoAppend=0b0,
        LeftAppend=0b01,
        RightAppend=0b10,
        BothAppend=LeftAppend |RightAppend,
        Avoid=0b100
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
     * trivial的构造函数
     */
    TrainGap(std::shared_ptr<const RailStationEvent> left_,
        std::shared_ptr<const RailStationEvent> right_,GapType type_,
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

    /**
     * 左车次这个事件是否为列车运行的站前事件。根据位置和行别判断。
     */
    bool isLeftFormer()const;

    bool isRightFormer()const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TrainGap::GapTypes)
