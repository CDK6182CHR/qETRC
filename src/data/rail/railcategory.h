#pragma once

#include <QList>
#include <memory>
#include "railway.h"

/**
 * @brief The RailCategory class
 * 一组列车的集合。一张运行图Diagram中可以有一个本对象。
 * DiagramPage中暂定还是直接QList
 * 暂时简单的实现为一个容器
 * 主要动因是在Railway基本信息编辑的窗口里检测线名修改的合法性
 */
class RailCategory
{
    QList<std::shared_ptr<Railway>> _railways;
public:
    RailCategory()=default;
    RailCategory(RailCategory&&)=default;
    RailCategory(const RailCategory&)=delete;
    RailCategory& operator=(RailCategory&&)=default;

    auto& railways(){return _railways;}
    const auto& railways()const{return _railways;}

    bool railNameIsValid(const QString& name, std::shared_ptr<Railway> rail)const;

    int getRailwayIndex(std::shared_ptr<Railway> rail)const;
};

