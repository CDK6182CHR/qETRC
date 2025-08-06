#pragma once

#include <QGraphicsItem>
#include <Qt>

#include "data/common/traintime.h"
#include "data/diagram/diagrampage.h"
#include "data/train/stationpoint.h"

class DiagramPage;
class Diagram;
class TrainAdapter;
class TrainLine;
class Railway;
class QPointF;
class PaintStationPointItem;
class QEMultiLinePath;
class Routing;

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
    TrainTime startTime;

    QRectF _bounding;

    /**
     * 2024.03.01: this structure logs the information about one link line determined using the link-layer alg.
     */
    struct LinkLayerInfo {
        int layer = -1;
        double x_pre = -1, x_cur = -1;
    };
    LinkLayerInfo startLayer, endLayer;

    struct LinkLineItems {
        LinkLayerInfo layerInfo;
        QGraphicsPathItem* linkItem1 = nullptr, * linkItem2 = nullptr;
        QGraphicsSimpleTextItem* linkLabelItem = nullptr;
        QGraphicsRectItem* linkLabelRect = nullptr;

        void setVisibility(bool on);

        void deleteItems();
    };

    /**
     * 2025.08.06  items baggaged for pre/post link lines
     */
    LinkLineItems preLinkItems, postLinkItems;
    
    /**
     * 2024.02.26  用于仅选中车次显示连线的情况。仅在第一次选中时尝试绘制，后面就不用再试了。
     */
    bool hasPreLinkLine = true, hasPostLinkLine = true;

    static constexpr const double LINK_LINE_WIDTH = 0.5;

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

    /**
     * 2024.06.27 update: use this for the *actual* pen for drawing train lines.
     * This may be different from train->pen(), if inverse_color is configured.
     */
    QPen pen;

    const double start_x, start_y;

    /**
     * 进行标签高度判定时，最大的检测宽度的一半
     */
    static constexpr double MAX_COVER_WIDTH = 200;

    bool _onDragging=false;
    const AdapterStation* _draggedStation=nullptr;
    StationPoint _dragPoint = StationPoint::NotValid;

    enum class DragTranslationType {
        SinglePoint, Backward, Forward
    };
    DragTranslationType _dragTrans = DragTranslationType::SinglePoint;

public:
    enum { Type = UserType + 1 };
    TrainItem(Diagram& diagram, std::shared_ptr<TrainLine> line, Railway& railway, DiagramPage& page, double startY,
        QGraphicsItem* parent = nullptr);

    /**
     * 2025.02.06: This item itself contains nothing, so just return an empty rect.
     * The actual rect is childrenBoundingRect.
     */
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

    /**
     * 2024.03.19  repaint the link line.
     * This is used when the previous linked train is updated.
     * If current item contains no link line, returns directly. Otherwise re-paint it.
     */
    void repaintLinkLine();

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
    bool dragBegin(const QPointF& pos, PaintStationPointItem* point, bool ctrl, bool alt, bool shift);

    /**
     * 2023.05.30  for drag move event: return the time corresponding to the pos by simply calculation.
     * PBC is implicitly considered.
     */
    TrainTime posToTime(const QPointF& pos)const;

    const QString& dragPointString()const;

    auto* draggedStation()const { return _draggedStation; }

    bool isOnDragging()const { return _onDragging; }

    TrainTime draggedOldTime()const;

    /**
     * Commit the drag, then set to non-dragging mode.
     */
    void doDrag(const TrainTime& tm, bool shift);

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

    // 2024.02.26 split from setLine: 
    // returns the text to be shown in train name label
    QString labelTrainName()const;

    void setStartItem(const QString& text, const QPen& pen);

    void setEndItem(const QString& text, const QPen& pen);

    QGraphicsSimpleTextItem* setStartEndLabelText(const QString& text, const QColor& color);

    /**
     * @brief 计算时刻对应的横坐标数值（相对于起始点）
     * 注意：规定返回总是正值。
     */
    double calXFromStart(const TrainTime& time)const;

    /**
     * 2023.05.30  the inverse operation of calXFromStart: 
     * Compute the time corresponding to the given x (with start_x extracted)
     */
    TrainTime calTimeByXFromStart(double x_from_start)const;

    /**
     * @brief 出图操作  运行线右越界
     * 铺画越界边界，以及越界标签。注意所给参数都是直接算出的  i.e.正值
     * 返回跨界点纵坐标
     */
    double getOutGraph(double xin, double yin, double xout, double yout, QEMultiLinePath& path);

    double getInGraph(double xout, double yout, double xin, double yin, QEMultiLinePath& path);

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

    void hideLinkLine();

    /**
     * 详细停点的标记，分为到、开两种情况，一共四个位置，由行别进一步细分
     * 特别说明对折返的处理：无终止标签的最后一站只标记到点，无起始标签的第一站只标记开点
     * 如果折返站没有停点，行为未定义 （标出来的结果是不对的）
     */
    void markArriveTime(double x, double y, const TrainTime& tm);

    void markDepartTime(double x, double y, const TrainTime& tm);

    /**
     * @brief addLinkLine
     * 添加与交路前序车次之间的连线
     * 2024.02.26: returns whether the link line is actually added.
     */
    bool addPreLinkLine(const QString& trainName);

    /**
     * 2025.08.06  Experimental: add post link line to the train line.
     * This is used only in a (somewhat) rare case: the post train in the routing exists, but not starts from
     * current railway.
     */
    bool addPostLinkLine();
    
    /**
     * 2025.08.06  Extracted from previous version of addLinkLine()
     * Add link line from specified x, y data. Used for both starting and ending links.
     */
    LinkLineItems addLinkLine(std::shared_ptr<const RailStation> rst, const TrainTime& fromTime, const TrainTime& toTime,
        const QString& labelText, bool isPostLink);

    void clearLinkLines();

    typename Qt::PenStyle linkLineStyle()const;

    QString linkLineLabelText(const QString& trainName, const Routing* rout)const;

    /**
     * 2024.03.01  determine the layer number (0, 1, ...) of the link line.
     */
    int linkLineLayer(const RailStation* rs, int xleft, int xright, bool isPostLink)const;

    /**
     * 2024.03.01  similar to linkLineLayer, but for end point. Actually, used for end label
     * when link-mode is enabled.
     */
    int linkLineLayerEnd(const RailStation* rs, int xleft, int xright)const;

    /**
     * Determine the height of routing link line.
     * The direction of the link line is extracted from this->dir().
     * If floating link is not enabled. simply return 0.
     * Also, records the layer number of the current link line.
     */
    double linkLineHeight(const RailStation* rs, int xlelft, int xright, bool isPostLink);

    QColor linkLineColor()const;

    QColor labelColor()const;

    QGraphicsPathItem* drawLinkLine(double x1, double x2, double y, double height, 
        bool left_start, bool right_end, bool hasLabel, QGraphicsSimpleTextItem* labelItem, bool isPostLink);

    /**
     * Compute the distance between the time `tm` and the coord `x` along time axis.
     * PBC is considered. The returned value should be the minimum (in abs. value) under PBC.
     * The sign is reserved, which is defined as:  x - tm 
     * MIND: if the distance is quite large, then the result may be not reliable.
     */
    double timeDistancePbc(double x, const TrainTime& tm)const;

    /**
     * Do time drag for the operated single point
     */
    void doDragSingle(const TrainTime& tm);
};


