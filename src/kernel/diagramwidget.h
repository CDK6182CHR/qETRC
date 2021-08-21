#pragma once

#include <memory>
#include <QGraphicsView>
#include <QWidget>
#include <QString>
#include <QGraphicsItemGroup>

#include "data/diagram/diagram.h"

/**
 * @brief The DiagramWidget class  运行图绘图窗口
 * pyETRC.GraphicsWidget
 */
class DiagramWidget : public QGraphicsView
{
    Q_OBJECT

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

    bool updating = false;

    QTime startTime;

public:
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

    void paintTrain(std::shared_ptr<Train> train);
    void paintTrain(Train& train);

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
    void removeTrain(QList<std::shared_ptr<TrainAdapter>>&& adps);

    /**
     * 当指定列车时刻更新时调用。
     * 暂定为先删除再重新铺画
     * adps作为旧运行线的索引,xvalue语义
     */
    void updateTrain(std::shared_ptr<Train>, QList<std::shared_ptr<TrainAdapter>>&& adps);

    /**
     * 在列车没有重新执行绑定的情况下，重新铺画
     * 主要用于基本信息变化
     */
    void repaintTrain(std::shared_ptr<Train> train);

    /**
     * 显示或隐藏列车运行线
     * 如果没有铺画过，现场铺画
     */
    void setTrainShow(std::shared_ptr<Train> trains, bool show);

    void setTrainShow(std::shared_ptr<TrainAdapter> adp, bool show);

    void setTrainShow(std::shared_ptr<TrainLine> line, bool show);

    auto page()const { return _page; }

protected:
    virtual void mousePressEvent(QMouseEvent* e)override;

    virtual void mouseMoveEvent(QMouseEvent* e)override;

    virtual void mouseDoubleClickEvent(QMouseEvent* e)override;

    virtual void resizeEvent(QResizeEvent* e)override;

    virtual void focusInEvent(QFocusEvent* e)override;

private:

    /**
     * @brief pyETRC.GraphicsWidget._initHLines()
     * 绘制水平线
     */
    void setHLines(std::shared_ptr<Railway> rail, double start_y,
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
            double width, double height);

    QGraphicsSimpleTextItem*
        addLeftTableText(const QString& str, const QFont& testFont, double start_x, double start_y,
            double width, double height);

    /**
     * def _addStationTableText(self, text, textFont, textColor, start_x, center_y, width):
     */
    QGraphicsSimpleTextItem*
        addStationTableText(const QString& str, const QFont& textFont, double start_x, double center_y,
            double width);

    /*
    def _drawSingleHLine(self, textColor, textFont, y, name, pen, width, leftItems, rightItems, dir_,
                         label_start_x):
    */
    void drawSingleHLine(const QFont& textFont, double y, const QString& name, const QPen& pen,
        double width, QList<QGraphicsItem*>& leftItems,
        QList<QGraphicsItem*>& rightItems, double label_start_x);

    const auto& margins()const { return _diagram.config().margins; }
    const auto& config()const { return _diagram.config(); }

    /**
     * 两端对齐且符合指定宽度的字符串
     * 用于生成站名标签
     * @param use_stretch  如果false，则不使用分散对齐
     */
    QGraphicsSimpleTextItem*
        alignedTextItem(const QString& text, const QFont& baseFont, double label_width,
            double start_x, double center_y, bool use_stretch = true);

    //def _addTimeAxisMark(self, value: int, gridColor: QtGui.QColor,
    //    font : QtGui.QFont, x : int)
    QGraphicsSimpleTextItem*
        addTimeAxisMark(int value, const QFont& font, int x);

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


signals:
    void showNewStatus(QString);
    void trainSelected(std::shared_ptr<Train> train);
    void pageFocussedIn(std::shared_ptr<DiagramPage> page);

private slots:
    void updateTimeAxis();
    void updateDistanceAxis();

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
     * 当天窗数据变化时，更新
     */
    void updateForbid(std::shared_ptr<Forbid> forbid, Direction dir);
};

