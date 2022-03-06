#pragma once
#include "data/diagram/traingap.h"
/**
 * @brief The GapConstraints class
 * 对列车间隔约束的描述，规定每一种间隔类型的最小值。
 * 以秒为单位。
 * 暂定：把单双线设置也放进来；这里其实包含了推线的所有约束条件
 */
class GapConstraints:
        public std::map<TrainGapTypePair,int>
{
    bool _singleLine=false;
public:

    /**
     * @brief GLOBAL_DEFAULT
     * 全局默认间隔。这个还不确定是否有用。
     */
    static constexpr const int GLOBAL_DEFAULT=3*60;

    using std::map<TrainGapTypePair,int>::map;

    /**
     * @brief correlationRange
     * @return 最大作用范围/关联长度。即所有的约束里面，最长的那个。
     * 在查找相关事件时，作为截断时长。
     */
    int correlationRange()const;

    bool isSingleLine()const{return _singleLine;}

    void setSingleLine(bool on){_singleLine=on;}
};

