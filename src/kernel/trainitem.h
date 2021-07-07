#pragma once

#include <QGraphicsItem>
#include <QWidget>
#include <QPen>

#include "data/diagram/diagram.h"
#include "data/diagram/diagrampage.h"
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
    DiagramPage& _page;
    Railway& _railway;

    QGraphicsPathItem* pathItem = nullptr, * expandItem = nullptr;
    QGraphicsPathItem* startLabelItem = nullptr, * endLabelItem = nullptr;
    QGraphicsSimpleTextItem* startLabelText = nullptr, * endLabelText = nullptr;
    QGraphicsRectItem* startRect = nullptr, * endRect = nullptr;
    bool _isHighlighted = false;
    bool _startAtThis, _endAtThis;

    QPointF startPoint, endPoint;
    QTime startTime;

    QRectF _bounding;

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

    QPen pen;

    const double start_x, start_y;

    /**
     * 进行标签高度判定时，最大的检测宽度的一半
     */
    static constexpr double MAX_COVER_WIDTH = 200;

public:
    enum { Type = UserType + 1 };
    TrainItem(Diagram& diagram, TrainLine& line, Railway& railway, DiagramPage& page, double startY,
        QGraphicsItem* parent = nullptr);

    virtual QRectF boundingRect()const override;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, 
        QWidget* widget = nullptr)override;

    inline int type()const override { return Type; }

    double getStartY()const { return start_y; }

    std::shared_ptr<Train> train();
    std::shared_ptr<const Train> train()const;

    auto& trainLine() { return _line; }
    const auto& trainLine()const { return _line; }

    void highlight();
    void unhighlight();

    virtual bool contains(const QPointF& f)const override;

    ~TrainItem()noexcept;

    /**
     * 清除Railway中保存的标签高度信息。
     * 逻辑上这个好像该析构函数去做。但如果是整张图重新铺画的情况，铺画前高度信息应该已经没有了，
     * 此时迭代器失效，再去删除会搞出问题来。
     * 因此只有单独重新铺画一趟车的运行线时，才有必要手动删除信息。
     * 
     * 以后写刷新操作时，应当把线路绑定信息也刷新一下。
     * ---
     * 2021.07.02  将LabelInfo迁移到TrainPage之后，应该可以在析构中执行这个了
     */
    void clearLabelInfo();

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
    const QPen& trainPen()const;

    QColor trainColor()const;

    /**
     * @brief 确定标签高度，以避免重叠
     * 同时维护数据
     */
    double determineStartLabelHeight();

    double determineEndLabelHeight();

    /**
     * 上下行判定标签高度的统一操作
     */
    std::multimap<double,LabelPositionInfo>::iterator
        determineLabelHeight(std::multimap<double, LabelPositionInfo>& spans,
        double xcenter, double left, double right);

    /**
     * 构造QFont对象，使所得的item宽度不大于指定宽度
     * 同时item已经被stretch过了
     */
    void setStretchedFont(QFont& base, QGraphicsSimpleTextItem* item, double width);

    void addTimeMarks();

    void hideTimeMarks();

    /**
     * 详细停点的标记，分为到、开两种情况，一共四个位置，由行别进一步细分
     * 特别说明对折返的处理：无终止标签的最后一站只标记到点，无起始标签的第一站只标记开点
     * 如果折返站没有停点，行为未定义 （标出来的结果是不对的）
     */
    void markArriveTime(double x, double y, const QTime& tm);

    void markDepartTime(double x, double y, const QTime& tm);

    /**
     * @brief addLinkLine
     * 添加与交路前序车次之间的连线
     */
    void addLinkLine();

};


