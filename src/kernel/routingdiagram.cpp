#include "routingdiagram.h"
#include <QTime>
#include <QGraphicsLineItem>
#include <QMessageBox>
#include "data/train/routing.h"
#include "data/diagram/trainadapter.h"
#include "util/utilfunc.h"
#include <optional>
#include "data/train/train.h"
#include "mainwindow/version.h"

#ifdef QT_PRINTSUPPORT_LIB
#include <QPrinter>
#endif

RoutingDiagram::RoutingDiagram(std::shared_ptr<Routing> routing_, QWidget *parent):
    QGraphicsView(parent),routing(routing_)
{
    setScene(new QGraphicsScene(this));
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setWindowTitle(tr("交路图测试"));
    setRenderHint(QPainter::Antialiasing,true);

    transData();
    paintDiagram();
}

bool RoutingDiagram::outPixel(const QString &filename) const
{
    QImage image(scene()->width(),scene()->height()+200,
                 QImage::Format_ARGB32);
    image.fill(Qt::white);
    renderDiagram(image);
    bool flag=image.save(filename);
    return flag;
}

bool RoutingDiagram::outVector(const QString &filename)
{
#ifdef QT_PRINTSUPPORT_LIB
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename);
    QSize size(scene()->width(),scene()->height()+200);
    QPageSize pageSize(size);
    printer.setPageSize(pageSize);
    return renderDiagram(printer);
#else
    Q_UNUSED(filename)
    QMessageBox::warning(this, tr("错误"), tr("由于当前平台不支持QtPrintSupport，无法使用"
        "导出PDF功能。请考虑导出PNG格式。"));
    return false;
#endif
}


bool RoutingDiagram::transData()
{
    QString report;
    if(!routing->checkForDiagram(report)){
        QMessageBox::warning(this,tr("错误"),tr("更新交路图失败，当前交路数据无法绘制交路图，"
        "原因如下：\n")+report);
        return false;
    }
    data.clear();
    int currentDay=0;
    QTime last_time(0,0,0);
    for(auto p=routing->order().begin();p!=routing->order().end();++p){
        if(p->isVirtual()){
            qDebug()<<"RoutingDiagram::transData: WARNING: INVALID virtual node "<<
                      p->name()<<Qt::endl;
            continue;
        }
        auto train=p->train();
        auto starting=train->boundStarting(),terminal=train->boundTerminal();
        if(!starting || !terminal){
            qDebug()<<"RoutingDiagram::transData: WARNING: invalid null starting or terminal: "<<
                      train->trainName().full()<<Qt::endl;
            continue;
        }
        if(starting->trainStation->arrive < last_time){
            currentDay++;
        }
        int start_day=currentDay;
        currentDay += train->deltaDays(_multiDayByTimetable);
        int end_day=currentDay;
        last_time=terminal->trainStation->depart;
        data.push_back(RoutingLine{p,start_day,end_day});
    }
    maxDay=currentDay;
    return true;
}

void RoutingDiagram::paintDiagram()
{
    scene()->clear();
    _setVLines();
    _setHLines();
    scene()->setSceneRect(0,0,sizes.totalWidth+100,sizes.height);
    std::optional<bool> dir_down=std::nullopt;
    QPointF last_point;
    foreach(const auto& d,data){
        auto train=d.node->train();   // 非空
        auto&& startPoint=_posCalculate(train->starting(),d.start_day,
                                        train->timetable().front().arrive);
        auto&& endPoint=_posCalculate(train->terminal(),d.end_day,
                                      train->timetable().back().depart);
        if (dir_down.has_value()){
            if (last_point.y() != startPoint.y()){
                // 不是同一站点，画虚线
                QPen pen(Qt::DashLine);
                scene()->addLine(last_point.x(),last_point.y(),
                                 startPoint.x(),startPoint.y(),pen);
            }else{
                double y=last_point.y();
                double maxY;
                if(dir_down.value()){
                    maxY=y+sizes.extraHeight;
                }else{
                    maxY=y-sizes.extraHeight;
                }
                scene()->addLine(last_point.x(),y,last_point.x(),maxY);
                scene()->addLine(last_point.x(),maxY,startPoint.x(),maxY);
                scene()->addLine(startPoint.x(),maxY,startPoint.x(),y);
            }
        }
        scene()->addLine(startPoint.x(),startPoint.y(),endPoint.x(),endPoint.y());
        double cx=(startPoint.x()+endPoint.x())/2,
                cy=(startPoint.y()+endPoint.y())/2;
        auto* textItem=scene()->addSimpleText(train->trainName().full());
        auto rect=textItem->boundingRect();
        double w=rect.width(),h=rect.height();
        textItem->setPos(cx-w,cy-h/2);
        _adjustTrainNameItem(textItem,startPoint,endPoint);
        last_point=endPoint;
        dir_down = (endPoint.y()>startPoint.y());

        // 始发站时刻标签
        textItem=scene()->addSimpleText(train->timetable().front().arrive.toString("hh:mm"));
        h=textItem->boundingRect().height();
        // 始发时刻标在右侧，终到时刻标在左侧
        int scale=(dir_down.value()?1:0);
        textItem->setX(startPoint.x());
        textItem->setY(startPoint.y()-scale*h);
        _adjustTimeItem(textItem,-2*scale+1);
        //终到时刻
        textItem=scene()->addSimpleText(train->timetable().back().depart.toString("hh:mm"));
        rect=textItem->boundingRect();
        w=rect.width();h=rect.height();
        textItem->setX(endPoint.x()-w);
        textItem->setY(endPoint.y()-(1-scale)*h);
        _adjustTimeItem(textItem,-1+2*scale);
    }
    emit diagramRepainted();
}

void RoutingDiagram::_setVLines()
{
    auto* l=scene()->addLine(sizes.left,0,sizes.left,sizes.height);
    QPen pen;
    pen.setWidthF(1.5);
    l->setPen(pen);

    for(int day=0;day<=maxDay;day++){
        double x=(day+1)*sizes.dayWidth+sizes.left;
        l=scene()->addLine(x,sizes.top,x,sizes.height-sizes.bottom);
        l->setPen(pen);

        auto* text=scene()->addText(QString::number(day+1));
        auto&& rect=text->boundingRect();
        text->setPos(x-rect.width(),sizes.top-rect.height());
    }
    sizes.totalWidth = (maxDay+1) * sizes.dayWidth + sizes.left + sizes.right;
}

void RoutingDiagram::_setHLines()
{
    bool fromTop=true;
    stationYValues = userDefinedYValues;   // copy assign
    topNext=sizes.top;
    bottomNext=sizes.height-sizes.bottom;
    foreach(const auto& d,data){
        auto train=d.node->train();   // transData()过程 保证非空
        StationName topStation=train->starting(),bottomStation=train->terminal();
        if (!fromTop) std::swap(topStation,bottomStation);
        if (!stationYValues.contains(topStation)){
            stationYValues.insert(topStation,topNext);
            topNext += sizes.stationDistance;
        }
        if(!stationYValues.contains(bottomStation)){
            // 对于大于30分钟的，排列在异侧。否则认为是出库一类的小交路，排列在同侧（都是顶部）
            if (d.end_day-d.start_day > 1 || train->totalMinSecs()>=1800 ){
                //安排在异侧
                stationYValues.insert(bottomStation,bottomNext);
                bottomNext -= sizes.stationDistance;
            }else{
                //同侧
                stationYValues.insert(bottomStation,topNext);
                topNext += sizes.stationDistance;
                fromTop = !fromTop;    // 多反一次
            }
        }
        fromTop = !fromTop;
    }
    for(auto p=stationYValues.cbegin();p!=stationYValues.cend();++p){
        _addStationLine(p.key(),p.value());
    }
}

void RoutingDiagram::_addStationLine(const StationName &name, double y)
{
    double maxX=sizes.totalWidth-sizes.right;
    scene()->addLine(sizes.left,y,maxX,y);
    auto* textItem1=scene()->addSimpleText(name.toDisplayLiteral());
    auto&& rect=textItem1->boundingRect();
    textItem1->setPos(sizes.left-rect.width()-5, y-rect.height()/2);
}

QPointF RoutingDiagram::_posCalculate(const StationName &name, int day, const QTime &tm)
{
    double y=stationYValues.value(name);
    double x=sizes.left;
    x+=day*sizes.dayWidth;
    int sec=tm.msecsSinceStartOfDay()/1000;
    x+=sec/(3600.0*24)*sizes.dayWidth;
    return QPointF(x,y);
}

void RoutingDiagram::_adjustTrainNameItem(QGraphicsSimpleTextItem *textItem, const QPointF &startPoint, const QPointF &endPoint)
{
    // 沿着斜线向endPoint方向调整车次标签位置。必须保证垂直方向不越界
    double minY=std::min(startPoint.y(),endPoint.y());
    double maxY=std::max(startPoint.y(),endPoint.y());
    auto&& rect=textItem->boundingRect();
    double w=rect.width(),h=rect.height();
    double dx,dy;
    if (endPoint.x() == startPoint.x()){
        // 斜率不存在的特殊情况
        dx=0,dy=h;
    }else if(endPoint.y()==startPoint.y()){
        // 斜率为0的特殊情况
        dy=0,dx=w;
    }else{
        double k=(endPoint.y()-startPoint.y())/(endPoint.x()-startPoint.x());
        dy=h,dx=dy/k;
    }
    if(endPoint.y()<startPoint.y()){
        dy=-dy;dx=-dx;
    }
    double x=textItem->x(),y=textItem->y();
    while (minY<=y && y<=maxY){
        const auto& items=textItem->collidingItems();
        bool flag=true;
        foreach(auto* item, items){
            if(item->type() == QGraphicsSimpleTextItem::Type){
                x+=dx;
                y+=dy;
                textItem->setPos(x,y);
                flag=false;
                break;
            }
        }
        if(flag)
            break;
    }
}

void RoutingDiagram::_adjustTimeItem(QGraphicsSimpleTextItem *textItem, int scale)
{
    // 向远离运行线的方向调整冲突的始发终到时刻标签的位置。scale为正负1，表示移动方向。
    auto&& rect=textItem->boundingRect();
    double h=rect.height();
    double y=textItem->y();
    double dy=h*scale;
    while (0<=y && y<=sizes.height-h){
        const auto& items=textItem->collidingItems();
        bool flag=true;
        foreach(auto item,items){
            if (item->type() == QGraphicsSimpleTextItem::Type){
                y+=dy;
                textItem->setY(y);
                flag=false;
                break;
            }
        }
        if (flag) break;
    }
}

bool RoutingDiagram::renderDiagram(QPaintDevice &img) const
{
    QPainter painter;
    painter.begin(&img);
    if (!painter.isActive())
        return false;
    painter.scale(img.width()/scene()->width(),img.width()/scene()->width());
    QFont font;
    font.setPixelSize(20);
    painter.setFont(font);
    painter.setRenderHint(QPainter::Antialiasing,true);
    scene()->render(&painter,QRectF(0,100,scene()->width()+30,scene()->height()));
    painter.drawText(sizes.left,40,tr("%1交路示意图").arg(routing->name()));
    //painter.drawText(self.sizes["left"],70,f"在运行图{self.graph.lineName()}上")
    double h=scene()->height()+200;
    painter.drawText(sizes.left,h-70,tr("%1型 %2担当 备注：%3").arg(routing->model(),
                                                              routing->owner(),
                                                              routing->note()));
    painter.drawText(sizes.left,h-40,routing->orderString());
    painter.drawText(scene()->width()-400,h-20,tr("由%1_%2导出")
                     .arg(qespec::TITLE.data(), qespec::VERSION.data()));
    return true;
}

void RoutingDiagram::setDayWidth(int value)
{
    sizes.dayWidth=value;
    paintDiagram();
}

void RoutingDiagram::setHeight(int value)
{
    sizes.height=value;
    paintDiagram();
}

void RoutingDiagram::refreshDiagram()
{
    paintDiagram();
}

void RoutingDiagram::setMultiDayByTimetable(bool on)
{
    _multiDayByTimetable=on;
    if(!transData())return;
    paintDiagram();
}





