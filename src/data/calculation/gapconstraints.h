#pragma once
#include "data/diagram/traingap.h"
/**
 * @brief The GapConstraints class
 * 对列车间隔约束的描述，规定每一种间隔类型的最小值。
 * 以秒为单位。
 * 暂定：把单双线设置也放进来；这里其实包含了推线的所有约束条件
 */
class GapConstraints:
    public std::map<TrainGap::GapTypesV2,int>
{
    using MyBase = std::map<TrainGap::GapTypesV2, int>;
    //bool _singleLine=false;
public:

    /**
     * @brief GLOBAL_DEFAULT
     * 全局默认间隔。这个还不确定是否有用。
     */
    static constexpr const int GLOBAL_DEFAULT=3*60;

    using std::map<TrainGap::GapTypesV2,int>::map;

    /**
     * @brief correlationRange
     * @return 最大作用范围/关联长度。即所有的约束里面，最长的那个。
     * 在查找相关事件时，作为截断时长。
     */
    int correlationRange()const;

    //bool isSingleLine()const{return _singleLine;}

    //void setSingleLine(bool on){_singleLine=on;}

    QString toString()const;

    /**
     * 2023.06.16  API V2
     * 检查所给的间隔时间是否发生冲突
     * 这里不能简单地直接at()，因为所给的type可能不是“正则”的，
     * 也就是有，例如，“通到”这种情况，它不在key里，会触发bug。
     */
    bool checkConflict(TrainGap::GapTypesV2 type, int secs)const;

    /**
     * 2023.06.16  类似于 at()，但是提前正则化处理。
     */
    int maxConstraint(TrainGap::GapTypesV2 type)const;
private:
    // 2023.06.16:  API V2.
    // calling at() directly is NOT SAFE; use the wrapped APIs instead.
    using MyBase::at;
};

