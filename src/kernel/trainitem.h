#pragma once

#include <QGraphicsItem>
#include <QTime>

#include "data/diagram/diagrampage.h"
#include "data/train/stationpoint.h"

class DiagramPage;
class Diagram;
class TrainAdapter;
class TrainLine;
class Railway;
class QPointF;
class PaintStationPointItem;


/**
 * @brief The TrainItem class  列车运行线类
 * 列车在特定线路上的一段运行线的抽象。
 * 生命周期不大于TrainLine以及Railway -- 采用引用
 * 注意构造本对象时，Train一定是要显示的（不做检查）
 * 2021.07.12提出新问题：系统更新数据时，可能先更新了数据，此时TrainLine对象被删除，
 * 再清理运行图时在析构函数发生异常。
 * 暂定解决方案：将TrainLine改为shared_ptr
 * 但注意这个并没有解决问题。TrainLine中间接采用ref引用到了Railway，此时Railway对象已经无了
 * 现在暂时通过先删除page的信息，再集中析构Item。
 */
class TrainItem : public QGraphicsItem
{
    std::shared_ptr<TrainLine> _line;
    Diagram& _diagram;
    DiagramPage& _page;
    Railway& _railway;

    QGraphicsPathItem* pathItem = nullptr, * expandItem = nullptr;
    QGraphicsPathItem* startLabelItem = nullptr, * endLabelItem = nullptr;
    QGraphicsSimpleTextItem* startLabelText = nullptr, * endLabelText = nullptr;
    QGraphicsRectItem* startRect = nullptr, * endRect = nullptr;
    bool _isHighlighted = false, _linkHighlighted = false;
    bool _startAtThis, _endAtThis;

    QPointF startPoint, endPoint;
    QTime startTime;

    QRectF _bounding;

    /**
     * @brief linkItem1
     * 采用交路连线时的连线对象。最多两个（考虑跨日）
     */
    QGraphicsPathItem* linkItem1 = nullptr, * linkItem2 = nullptr;
    int linkLayer = -1;
    double link_x_pre = -1, link_x_cur = -1;

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

    /**
     * 2023.06.04  图定铺画点标记/拖动点
     */
    QVector<PaintStationPointItem*> stationMarks;

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

    bool _onDragging=false;
    const AdapterStation* _draggedStation=nullptr;
    StationPoint _dragPoint = StationPoint::NotValid;

public:
    enum { Type = UserType + 1 };
    TrainItem(Diagram& diagram, std::shared_ptr<TrainLine> line, Railway& railway, DiagramPage& page, double startY,
        QGraphicsItem* parent = nullptr);

    virtual QRectF boundingRect()const override;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, 
        QWidget* widget = nullptr)override;

    inline int type()const override { return Type; }

    /**
     * 这是所在线路的起始纵坐标，绝对坐标
     */
    double getStartY()const { return start_y; }

    std::shared_ptr<Train> train();
    std::shared_ptr<const Train> train()const;

    auto& trainLine() { return _line; }
    const auto& trainLine()const { return _line; }

    void highlight();
    void unhighlight();
    void highlightWithLink();
    void unhighlightWithLink();

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

    /**
     * 2024.02.22 similar to clearLabelInfo()
     */
    void clearLinkInfo();

    Direction dir()const;

    /**
     * 2023.05.28  Begin of dragging, called by MousePressEvent.
     * Find and store the station under drag.
     */
    bool dragBegin(const QPointF& pos, PaintStationPointItem* point, bool ctrl, bool alt);

    /**
     * 2023.05.30  for drag move event: return the time corresponding to the pos by simply calculation.
     * PBC is implicitly considered.
     */
    QTime posToTime(const QPointF& pos)const;

    const QString& dragPointString()const;

    auto* draggedStation()const { return _draggedStation; }

    bool isOnDragging()const { return _onDragging; }

    QTime draggedOldTime()const;

    /**
     * Commit the drag, then set to non-dragging mode.
     */
    void doDrag(const QTime& tm);

private:
    
    const Config& config()const;
    const MarginConfig& margins()const;

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
     * 2023.05.30  the inverse operation of calXFromStart: 
     * Compute the time corresponding to the given x (with start_x extracted)
     */
    QTime calTimeByXFromStart(double x_from_start)const;

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
     * 2023.06.04  添加图定点标记
     */
    void addStationPoints();

    void hideStationPoints();

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

    /**
     * Determine the height of routing link line.
     * The direction of the link line is extracted from this->dir().
     * If floating link is not enabled. simply return 0.
     * Also, records the layer number of the current link line.
     */
    double linkLineHeght(const RailStation* rs, int xlelft, int xright);

    QGraphicsPathItem* drawLinkLine(double x1, double x2, double y, double height);

    /**
     * Compute the distance between the time `tm` and the coord `x` along time axis.
     * PBC is considered. The returned value should be the minimum (in abs. value) under PBC.
     * The sign is reserved, which is defined as:  x - tm 
     * MIND: if the distance is quite large, then the result may be not reliable.
     */
    double timeDistancePbc(double x, const QTime& tm)const;

};


