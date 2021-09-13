#pragma once

#include <QList>
#include <memory>

class Railway;

/**
 * @brief The RailCategory class
 * 一组列车的集合。一张运行图Diagram中可以有一个本对象。
 * DiagramPage中暂定还是直接QList
 * 暂时简单的实现为一个容器
 * 主要动因是在Railway基本信息编辑的窗口里检测线名修改的合法性
 * 
 * 2021.09.13：进一步支持带子类的情况。为线路数据库铺路。
 */
class RailCategory
{
    std::weak_ptr<RailCategory> _parent;
    QList<std::shared_ptr<RailCategory>> _subcats;
    QList<std::shared_ptr<Railway>> _railways;
public:
    RailCategory()=default;
    RailCategory(std::weak_ptr<RailCategory> parent);
    RailCategory(RailCategory&&)=default;
    RailCategory(const RailCategory&)=delete;
    RailCategory& operator=(RailCategory&&)=default;

    auto parent() { return _parent; }

    auto& railways(){return _railways;}
    const auto& railways()const{return _railways;}

    bool railNameIsValid(const QString& name, std::shared_ptr<Railway> rail)const;

    int getRailwayIndex(std::shared_ptr<const Railway> rail)const;

    int getRailwayIndex(const Railway& rail)const;
};

