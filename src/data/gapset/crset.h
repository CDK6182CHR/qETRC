#pragma once

#include "gapsetabstract.h"

namespace gapset {
namespace cr {

/**
 * @brief The CRSet class
 * 中国铁路列车间隔规定系统的一部分，即仅包含同站同侧间隔的部分。
 * Ref. 倪少全.中国铁路列车运行图编制系统研究[D].西南交通大学,2013.
 */
class CRSet : public gapset::GapSetAbstract
{
public:
    CRSet();
    virtual void buildSet() override;
};

} // namespace cr
} // namespace gapset

