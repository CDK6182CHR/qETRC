﻿#pragma once

#include <memory>
#include <QGraphicsView>
#include <QString>
#include <QTime>
#include <deque>
#include "data/common/direction.h"
#include "data/diagram/trainline.h"
#include "data/common/qeglobal.h"

class Diagram;
class QGraphicsItemGroup;
class TrainItem;
class DiagramPage;
class Train;
class QMenu;
class TrainLine;
class TrainAdapter;
class Railway;
struct MarginConfig;
struct Config;
class Forbid;
class ForbidNode;
class Routing;
class DragTimeInfoWidget;
class PaintStationPointItem;
class PaintStationInfoWidget;
namespace qeutil {
    class QEBalloonTip;
}

/**
 * @brief The DiagramWidget class  运行图绘图窗口
 * pyETRC.GraphicsWidget
 * 关于ZValue的约定（pyETRC的注释）
 * 2019年4月27日批注，自2.0.2版本开始规范化图形空间的z_value。分配如下
 * 0：基本层。包含底图框线。
 * [1,5) 区间安排底图上的修饰内容。目前仅有天窗。天窗为1.
 * [5,10)区间安排列车运行线。目前统一安排为5.
 * （2023.06.04增加）6：PaintStationPointItem  铺画点标记/拖动点
 * （2021.08.29增加）9：weakItem  用于遮蔽其他运行线的。
 * 10：选中车次运行线层。
 * [10,15)预留。
 * [15,20)软件悬浮层。目前安排距离轴、时间轴15，选中车次名称16.
 * （2023.06.01增加）18：DragTimeInfoWidget   拖动时刻的信息窗口
 */
class DiagramWidget : public QGraphicsView
{
    Q_OBJECT;

    /**
     * @brief _diagram  运行图基础数据，全程采用同一个对象
     * 2021.07.02：将这里改成shared_ptr. 
     * 原因是_page中保存了一些图元的指针，特别是Label的信息，为了保证析构不出问题，
     * 必须保证Page的析构晚于DiagramWidget的析构。
     */
    const std::shared_ptr<DiagramPage> _page;
    Diagram& _diagram;

    /**
     * @brief _selectedTrain  当前选中的列车对象
     * 2021.07.06--改为shared
     */
    std::shared_ptr<Train> _selectedTrain = nullptr;

    struct {
        QGraphicsItemGroup* left, * right, * top, * bottom;
    } marginItems;
    //显示当前车次的Item
    QGraphicsSimpleTextItem* nowItem;
    QGraphicsRectItem* weakItem = nullptr;
    qeutil::QEBalloonTip* posTip = nullptr;

    bool updating = false;

    QTime startTime;

    QMenu* contextMenu = nullptr;

    // 2023.05.28  status for dragging time
    bool _onDragging = false;
    TrainItem* _draggedItem = nullptr;
    QPointF _dragStartPoint;
    bool _dragShift = false;
    Qt::KeyboardModifiers _dragMod;
    Direction _paintInfoDir;

    DragTimeInfoWidget* _dragInfoWidget = nullptr;
    QGraphicsProxyWidget* _dragInfoProxy = nullptr;
    std::map<PaintStationInfoWidget*, QGraphicsProxyWidget*> _paintInfoProxies;

public:
    struct SharedActions {
        QAction* refreshAll;
        QAction* search,* rulerRef, * trainRef, * eventList;
        QAction* timeAdjust, * batchCopy, * intervalExchange, * simpleInterp;
        QAction* addTrain, * rulerPaint, * greedyPaint;
        SharedActions() = default;
        SharedActions(const SharedActions&) = delete;
    };

    DiagramWidget(Diagram& daigram, std::shared_ptr<DiagramPage> page, QWidget* parent = nullptr);
    ~DiagramWidget()noexcept;

    /**
     * @brief autoPaintGraph
     * 由系统调用的自动铺画运行图过程。如果设置为不自动铺画，就不执行任何操作
     */
    void autoPaintGraph();

    /**
     * @brief paintGraph
     * 清空并重新绘制运行图
     */
    void paintGraph();

    /**
     * 暴力清空图元、所有映射关系。用在重新铺画之前。
     * 主要原因：如果发生了重新绑定，则TrainLine数据失效，原有的映射失效，
     * 如果简单的clear()则会造成析构异常。（原有的TrainLine地址无了，映射到奇怪的东西）
     */
    void clearGraph();

    auto selectedTrain() { return _selectedTrain; }
    void setSelectedTrain(std::shared_ptr<Train> train) { _selectedTrain = train; }

    bool toPdf(const QString& filename, const QString& title, const QString& note);

    /**
     * 2024.04.10: Change to async impl. The parent is used for constructing QProgressDialog.
     */
    void toPdfAsync(const QString& filename, const QString& title, const QString& note, QWidget* parent);

    void toPdfAsyncMultiPage(const QString& filename, const QString& title, const QString& note, QWidget* parent, 
        int hours_per_page);

    bool toPng(const QString& filename, const QString& title, const QString& note);

    
    void paintTrain(std::shared_ptr<Train> train);
    void paintTrain(Train& train);

    /**
     * 2024.02.12  Note: this is slightly different from the paintTrain(...).
     * Here, the on-painting train is automatically highlighted. Thus, the paiting methods
     * should call this function.
     */
    void paintTrainTmp(std::shared_ptr<Train> train);

    /**
     * 铺画列车运行线。注意paintTrain()不采用此方法，因为这里涉及查找Railway的序号等操作，
     * 效率低一点点
     */
    void paintTrainLine(std::shared_ptr<TrainLine> line);

    /**
     * 删除列车时调用
     * 移除和删除列车运行线  注意相关映射表的处理，对象的删除等
     */
    void removeTrain(const Train& train);

    /**
     * 删除由adps所指的原本属于某列车的运行线，用于更新列车运行线
     * 注意对adp只允许使用其中TrainLine的地址信息，用作删除索引。
     * 右值引用，强调xvalue语义
     */
    void removeTrain(QVector<std::shared_ptr<TrainAdapter>>&& adps);

    /**
     * 当指定列车时刻更新时调用。
     * 暂定为先删除再重新铺画
     * adps作为旧运行线的索引,xvalue语义
     */
    void updateTrain(std::shared_ptr<Train>, QVector<std::shared_ptr<TrainAdapter>>&& adps);

    /**
     * 在列车没有重新执行绑定的情况下，重新铺画
     * 主要用于基本信息变化
     */
    void repaintTrain(std::shared_ptr<Train> train);

    void repaintTrainLinkLine(std::shared_ptr<Train> train);

    /**
     * 显示或隐藏列车运行线
     * 如果没有铺画过，现场铺画
     */
    void setTrainShow(std::shared_ptr<Train> trains, bool show);

    void setTrainShow(std::shared_ptr<TrainAdapter> adp, bool show);

    void setTrainShow(std::shared_ptr<TrainLine> line, bool show);

    auto page()const { return _page; }

    void setupMenu(const SharedActions& actions);

protected:
    virtual void mousePressEvent(QMouseEvent* e)override;

    virtual void mouseMoveEvent(QMouseEvent* e)override;

    virtual void mouseReleaseEvent(QMouseEvent* e)override;

    virtual void mouseDoubleClickEvent(QMouseEvent* e)override;

    virtual void resizeEvent(QResizeEvent* e)override;

    virtual bool event(QEvent* e)override;

    virtual void contextMenuEvent(QContextMenuEvent* e)override;

private:

    /**
     * @brief pyETRC.GraphicsWidget._initHLines()
     * 绘制水平线
     */
    void setHLines(int idx, std::shared_ptr<Railway> rail, double start_y,
        double width, QList<QGraphicsItem*>& leftItems, QList<QGraphicsItem*>& rightItems);

    /**
     * @brief pyETRC.GraphicsWidget._initVLines()
     * 绘制上下的标题 时间轴
     */
    void setVLines(double width, int hour_count, const QList<QPair<double, double>> railYRanges);

    double minitesToPixels(int minutes)const;

    /**
     * @brief 绘制所给列车在所给线路的运行图
     * 注意调用之前，应当已经绑定好数据
     */
    void paintTrain(std::shared_ptr<Railway> railway, std::shared_ptr<Train> train);

    /**
     * pyETRC.GraphicsWidget._addLeftTableText(self, text: str, 
     *           textFont, textColor, start_x, start_y, width, height)
     * 添加左侧表格中的一项，封装调整位置、大小、字体、宽度等操作
     */
    QGraphicsSimpleTextItem*
        addLeftTableText(const char* str, const QFont& testFont, double start_x, double start_y,
            double width, double height, const QColor& textColor);

    QGraphicsSimpleTextItem*
        addLeftTableText(const QString& str, const QFont& testFont, double start_x, double start_y,
            double width, double height, const QColor& textColor);

    /**
     * def _addStationTableText(self, text, textFont, textColor, start_x, center_y, width):
     */
    QGraphicsSimpleTextItem*
        addStationTableText(const QString& str, const QFont& textFont, double start_x, double center_y,
            double width, const QColor& textColor);

    /*
    def _drawSingleHLine(self, textColor, textFont, y, name, pen, width, leftItems, rightItems, dir_,
                         label_start_x):
    */
    void drawSingleHLine(const QFont& textFont, double y, const QString& name, const QPen& pen, const QColor& textColor,
        double width, QList<QGraphicsItem*>& leftItems,
        QList<QGraphicsItem*>& rightItems, double label_start_x);

    const MarginConfig& margins()const;
    const Config& config()const;

    /**
     * 两端对齐且符合指定宽度的字符串
     * 用于生成站名标签
     * @param use_stretch  如果false，则不使用分散对齐
     * @param scale 2021.08.31新增  宽度中字体部分所占的比例
     */
    QGraphicsSimpleTextItem*
        alignedTextItem(const QString& text, const QFont& baseFont, double label_width,
            double start_x, double center_y, const QColor& textColor, bool use_stretch = true,
            double scale = 1.0);

    //def _addTimeAxisMark(self, value: int, gridColor: QtGui.QColor,
    //    font : QtGui.QFont, x : int)
    QGraphicsSimpleTextItem*
        addTimeAxisMark(int value, const QFont& font, const QColor& grd_color, int x);

    void selectTrain(TrainItem* item);

    void unselectTrain();

    /**
     * pyETRC.GrpahicsWidget._resetForbidShow()
     * 铺画时，直接重新画所有标尺
     */
    void showAllForbids();

    /**
     * @brief pyETRC.GraphicsWidget.show_forbid()  绘制指定方向的指定天窗
     * 暂定private，对外接口等写到时再设计
     */
    void showForbid(std::shared_ptr<Forbid> forbid, Direction dir);

    /**
     * 删除旧天窗数据（总是），显示新的天窗数据（如果要求显示）。
     * 实际上起到了更新的作用。
     */
    void showForbid(std::shared_ptr<Forbid> forbid, Direction dir, int index);

    void removeForbid(std::shared_ptr<Forbid> forbid, Direction dir);

    void addForbidNode(std::shared_ptr<Forbid> forbid, std::shared_ptr<ForbidNode> node,
        const QBrush& brush, const QPen& pen, double startY);

    double calXFromStart(const QTime& time)const;

    /**
     * 指定位置的TrainItem，如果没有，返回空、
     * 注意pos应当是mapToScene后的位置
     */
    TrainItem* posTrainItem(const QPointF& pos);

    void stationToolTip(std::deque<AdapterStation>::const_iterator st, const TrainLine& line);

    void intervalToolTip(std::deque<AdapterStation>::const_iterator former,
        std::deque<AdapterStation>::const_iterator latter, const TrainLine& line);

    /**
     * 输出PDF和PNG的公共操作
     * 2024.04.10: refactor to public slots (previous private function) for multi-threaded impl.
     */
    //void paintToFile(QPainter& painter, const QString& title, const QString& note);

    /**
     * 显示用于虚化非选择车次的蒙板
     */
    void showWeakenItem();

    void hideWeakenItem();

    void showPosTip(const QPoint& pos, const QString& msg, 
        const QString& title = tr("运行图定位"));

    /**
     * 2023.05.30  moved from MouseMoveEvent()
     * The process about mouse-over information
     */
    void showTimeTooltip(const QPoint& pos_glb);

    void dragTimeBegin(const QPointF& pos, TrainItem* item, PaintStationPointItem* point,
        bool ctrl_pressed, bool alt_pressed, bool shift_pressed, Qt::KeyboardModifiers mod);

    void dragTimeMove(const QPointF& pos);

    void dragTimeFinish(const QPointF& pos);

    /**
     * 2024.02.12 Show painting info widget for painting train.
     */
    void showPaintingInfoWidget(const QPointF& pos, TrainItem* item, PaintStationPointItem* point);

signals:
    void showNewStatus(QString);
    void trainSelected(std::shared_ptr<Train> train);
    void pageFocussedIn(std::shared_ptr<DiagramPage> page);
    void railFocussedIn(std::shared_ptr<Railway> railway);

    void timeDraggedSingle(std::shared_ptr<Train> train, int station_id, const TrainStation& data, 
        Qt::KeyboardModifiers mod);
    void timeDraggedNonLocal(std::shared_ptr<Train> trian, int station_id, std::shared_ptr<Train> data,
        Qt::KeyboardModifiers mod);
    void paintingPointClicked(DiagramWidget* d, std::shared_ptr<Train> train, AdapterStation* st);

private slots:
    void updateTimeAxis();
    void updateDistanceAxis();
    void closePaintInfoWidget();

public slots:

    /**
     * @brief showTrainEventText  临时使用 ETRC风格的文本形式输出事件表
     */
    void showTrainEventText();

    /**
     * 选中一趟列车，高亮显示，但不发送信号。
     * 由MainWindow那边调用，避免递归的信号。
     */
    void highlightTrain(std::shared_ptr<Train> train);

    /**
     * 高亮显示交路，但不设置列车为当前列车，也不发送信号。
     */
    void highlightRouting(std::shared_ptr<Routing> routing);

    void unhighlightRouting(std::shared_ptr<Routing> routing);

    /**
     * 当天窗数据变化时，更新
     */
    void updateForbid(std::shared_ptr<Forbid> forbid, Direction dir);

    void zoomIn();

    void zoomOut();

    /**
     * 定位到指定线路的指定车站
     */
    void locateToStation(std::shared_ptr<const Railway> railway,
        std::shared_ptr<const RailStation> station, const QTime& tm);

    void locateToMile(std::shared_ptr<const Railway> railway, double mile, const QTime& tm);

    /**
     * 2024.02.12  add the info widget into scene.
     * Here, the widget's parent is this.
     */
    void addPaintStationInfoWidget(PaintStationInfoWidget* w);

    /**
     * 输出PDF和PNG的公共操作
     * 2024.04.10: refactor to public slots (previous private function) for multi-threaded impl.
     */
    void paintToFile(QPainter& painter, const QString& title, const QString& note, const QString& page_mark = {});

};

