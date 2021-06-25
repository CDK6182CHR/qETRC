#pragma once

#include <QGraphicsItem>
#include <QWidget>

#include "data/diagram/diagram.h"
#include "data/rail/railstation.h"

class TrainAdapter;
class TrainLine;
class Railway;

/**
 * @brief The TrainItem class  列车运行线类
 * 列车在特定线路上的一段运行线的抽象。
 * 生命周期不大于TrainLine以及Railway -- 采用引用
 * 注意构造本对象时，Train一定是要显示的（不做检查）
 */
class TrainItem : public QGraphicsItem
{
    TrainLine& _line;
    Diagram& _diagram;
    Railway& _railway;

    QGraphicsPathItem* pathItem = nullptr, * expandItem = nullptr;
    QGraphicsPathItem* startLabelItem = nullptr, * endLabelItem = nullptr;
    QGraphicsSimpleTextItem* startLabelText = nullptr, * endLabelText = nullptr;
    QGraphicsRectItem* startRect = nullptr, * endRect = nullptr;
    bool _isHighlighted = false;
    bool _startAtThis, _endAtThis;

    QPointF startPoint, endPoint;
    QTime startTime;

    /**
     * @brief linkItem1
     * 采用交路连线时的连线对象。最多两个（考虑跨日）
     */
    QGraphicsLineItem* linkItem1 = nullptr, * linkItem2 = nullptr;

    /**
     * @brief spanItems
     * 跨越运行图边界处的标签
     */
    QList<QGraphicsSimpleTextItem*> spanItems;

    /**
     * @brief markLabels
     * 标注通过站时刻的标签
     */
    QList<QGraphicsSimpleTextItem*> markLabels;

    std::multimap<double, LabelPositionInfo>::iterator startLabelInfo, endLabelInfo;

    /**
     * @brief 首末点是否在图幅内，用来判定是否要标注标签
     */
    bool startInRange = true, endInRange = true;

    double spanItemWidth = -1, spanItemHeight = -1;
    double startLabelHeight = -1, endLabelHeight = -1;

public:
    enum { Type = UserType + 1 };
    TrainItem(TrainLine& line, Railway& railway, Diagram& diagram,
        QGraphicsItem* parent = nullptr);

    virtual QRectF boundingRect()const override;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, 
        QWidget* widget = nullptr)override;

    inline int type()const override { return Type; }

    Train& train();

    void highlight();
    void unhighlight();

private:
    
    const Config& config()const { return _diagram.config(); }
    const auto& margins()const { return _diagram.config().margins; }

    void setLine();

    /**
     * @brief setPathItem
     * 绘制运行线主体部分  完全重写
     * 注意：合并主体和span的创建过程！
     */
    void setPathItem(const QString& trainName);

    void setStartItem(const QString& text, const QPen& pen);

    void setEndItem(const QString& text, const QPen& pen);

    QGraphicsSimpleTextItem* setStartEndLabelText(const QString& text, const QColor& color);

    /**
     * @brief 计算时刻对应的横坐标数值（相对于起始点）
     * 注意：规定返回总是正值。
     */
    double calXFromStart(const QTime& time)const;

    /**
     * @brief 出图操作  运行线右越界
     * 铺画越界边界，以及越界标签。注意所给参数都是直接算出的  i.e.正值
     * 返回跨界点纵坐标
     */
    double getOutGraph(double xin, double yin, double xout, double yout, QPainterPath& path);

    double getInGraph(double xout, double yout, double xin, double yin, QPainterPath& path);

    /**
     * @brief 封装查询列车绘制图形的方法
     * 暂定给个默认的
     */
    QPen trainPen()const;

    QColor trainColor()const;

    double determineStartLabelHeight();

    double determineEndLabelHeight();

    /**
     * 构造QFont对象，使所得的item宽度不大于指定宽度
     * 同时item已经被stretch过了
     */
    void setStretchedFont(QFont& base, QGraphicsSimpleTextItem* item, double width);

    void addTimeMarks();

    void hideTimeMarks();
};


