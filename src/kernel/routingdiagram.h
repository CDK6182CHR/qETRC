#pragma once

#include <QGraphicsView>
#include <memory>
#include <QMap>

#include <data/common/stationname.h>

class RoutingNode;


class Routing;
/**
 * @brief The RoutingDiagram class
 * pyETRC.CircuitDiagram   交路图
 * 代码基本照抄。但不同在：构造本类前检查是否能够画图；如果不能画图，直接不构造本类，
 * 而不是抛出异常。
 */
class RoutingDiagram : public QGraphicsView
{
    Q_OBJECT
    std::shared_ptr<Routing> routing;
    bool _multiDayByTimetable=false;
    QMap<StationName,double> userDefinedYValues,stationYValues;
    int maxDay=1;
    struct {
        int
        dayWidth=700,
        totalWidth=700,   // 这个数值后面再改
        height=500,
        left=140,
        right=20,
        top=40,
        bottom=40,
        stationDistance=40,
        extraHeight=20;
    } sizes;
    double topNext,bottomNext;   // 自动分配时，下一个位置
    struct RoutingLine{
        std::list<RoutingNode>::iterator node;
        int start_day,end_day;
    };
    QVector<RoutingLine> data;
public:
    RoutingDiagram(std::shared_ptr<Routing> routing_, QWidget* parent=nullptr);
    bool outPixel(const QString& filename)const;
    bool outVector(const QString& filename);
    const auto& getSizes()const{return sizes;}
    auto& getSizes(){return sizes;}
    const auto& getYValues()const{return stationYValues;}
    auto& getUserYValues(){return userDefinedYValues;}
private:
    bool transData();
    void paintDiagram();
    void _setVLines();
    void _setHLines();
    void _addStationLine(const StationName& name, double y);
    QPointF _posCalculate(const StationName& name, int day, const QTime& tm);
    void _adjustTrainNameItem(QGraphicsSimpleTextItem* textItem,
                              const QPointF& startPoint, const QPointF& endPoint);
    void _adjustTimeItem(QGraphicsSimpleTextItem* textItem, int scale);
    bool renderDiagram(QPaintDevice& img)const;
signals:
    void diagramRepainted();
public slots:
    void setDayWidth(int value);
    void setHeight(int value);
    void refreshDiagram();
    void setMultiDayByTimetable(bool on);

};


