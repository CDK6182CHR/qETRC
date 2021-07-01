#pragma once

#include <memory>
#include <QList>

#include "config.h"

class Railway;
class Diagram;
struct Config;
struct MarginConfig;

/**
 * @brief The DiagramPage class
 * Diagram的一个/页面/视图  显示一组Railway的运行图
 * DiagramWidget针对这个类绘图
 * 注意不保存实际数据
 */
class DiagramPage
{
    Diagram& _diagram;
    QList<std::shared_ptr<Railway>> _railways;
    QList<double> _startYs;
    QString _name;
public:
    DiagramPage(Diagram& diagram, const QList<std::shared_ptr<Railway>>& railways,
        const QString& name);
    auto& diagram(){return _diagram;}
    const auto& diagram()const{return _diagram;}
    auto& railways(){return _railways;}
    const auto& railways()const{return _railways;}
    auto& startYs(){return _startYs;}
    auto railwayAt(int i)const { return _railways.at(i); }
    const QString& name()const { return _name; }
    void setName(const QString& s) { _name = s; }

    QString railNameString()const;

    /**
     * 暂时实现为直接返回Diagram的数据 （不保存副本）
     */
    const Config& config()const;
    const MarginConfig& margins()const;

    inline int railwayCount()const { return _railways.size(); }

    /**
     * 所给线路的下标。通过地址比较。线性算法。
     * 如果找不到，返回-1
     */
    int railwayIndex(const Railway& rail)const;

    double railwayStartY(const Railway& rail)const;
};


