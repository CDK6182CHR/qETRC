#pragma once
#include <QString>
#include <vector>
#include <memory>
#include "gapgroupabstract.h"

namespace gapset{

/**
 * @brief The GapSetAbstract class
 * 表示一套约束下的所有间隔类型分组的集合，需包含所有可能的类型。
 * 需要与编辑页面互通。
 */
class GapSetAbstract:
        public std::vector<std::unique_ptr<GapGroupAbstract>>
{
protected:
    bool _singleLine=false;
    std::vector<TrainGapTypePair> _remainTypes;

public:
    GapSetAbstract()=default;

    bool singleLine()const {return _singleLine;}
    void setSingleLine(bool on){_singleLine=on;}

    /**
     * 设置是否单线；如果改变了，则重新构建set。
     */
    void setSingleLineAndBuild(bool on);


    /**
     * @brief remainTypes
     * 剩余的类型。在本分类模式下，不需要考虑，最后直接设成0就行了。
     * @return
     */
    auto& remainTypes(){return _remainTypes;}

    /**
     * @brief buildSet
     * 构建所有分组。由子类负责实现。
     */
    virtual void buildSet()=0;

    /**
     * @brief minimalGapByGroup
     * 2022.04.30  将全局的最小间隔重组为各个Group的最小。
     * @param mingap  按TrainGapTypePair分类的最小间隔
     * @param minSecs  最小的间隔，用于填充无数据的。通常用截断时长代替。
     * @param maxSecs  最大数值，防止出现很离谱的结果
     * @return 按Group分类的最小间隔，会包含所有的Group，但不管_remainTypes；
     * 无数据的实现为minSecs
     */
    std::map<const GapGroupAbstract*, int>
        minimalGapByGroup(const std::map<TrainGapTypePair,int>& mingap,
                          int minSecs, int maxSecs)const;

    void setConstraintFromMinimal(const std::map<TrainGapTypePair,int>& mingap,
                                  int minSecs, int maxSecs);

};


}

