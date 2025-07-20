#pragma once
#include <QGraphicsView>

#include "data/rail/trackdiagramdata.h"

/**
 * pyETRC.StationGraphWidget  图形部分实现
 */
class TrackDiagram : public QGraphicsView 
{
    Q_OBJECT
    TrackDiagramData& _data;
    struct {
        int
            left=60,
            right=20,
            top=50,
            bottom=50;
    }margins;

    double seconds_per_pix=15;
    double row_height=40;

public:
    TrackDiagram(TrackDiagramData& data, QWidget* parent=nullptr);
    void refreshData();
    int xScale()const{return seconds_per_pix;}

private:
    void paintDiagram();
    void _initXAxis(double width,double height,const QColor& gridColor);
    void _initYAxis(double width,const QPen& defaultPen,const QPen& boldPen);
    void _addTrains();
    void _addTrainRect(const TrackOccupy& to, double start_y);
    double _calXValue(const TrainTime& tm)const;
public slots:
    void setXScale(int value);
    void repaintDiagram();
};


