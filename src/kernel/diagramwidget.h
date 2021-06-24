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
     * 类似单例
     */
    Diagram& _diagram;
    std::shared_ptr<Train> _selectedTrain{};

    struct {
        QGraphicsItemGroup* left, * right, * top, * bottom;
    } marginItems;
    //显示当前车次的Item
    QGraphicsSimpleTextItem* nowItem;

public:
    DiagramWidget(Diagram& diagram, QWidget* parent = nullptr);

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

    auto selectedTrain() { return _selectedTrain; }
    void setSelectedTrain(std::shared_ptr<Train> train) { _selectedTrain = train; }

private:
    /**
     * @brief setAxes
     * 绘制坐标轴
     */
    void setAxes();

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
    void setVLines(double start_y,
        double width, int hour_count, const QList<QPair<double, double>> railYRanges);

    double minitesToPixels(int minutes)const;

    /**
     * @brief 绘制所给列车在所给线路的运行图
     * 注意调用之前，应当已经绑定好数据
     */
    void paintTrain(std::shared_ptr<Railway> railway, std::shared_ptr<Train> train);

    void paintTrain(std::shared_ptr<Train> train);

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

signals:
    void showNewStatus(QString);

private slots:
    void updateTimeAxis();
    void updateDistanceAxis();
};

