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

};


}

