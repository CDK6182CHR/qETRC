#include "trainitem.h"
#include "data/diagram/trainadapter.h"
#include "data/diagram/trainline.h"
#include "data/train/routing.h"
#include "data/diagram/diagram.h"

#include <QPainterPath>
#include <QPointF>
#include <QPen>
#include <QGraphicsScene>

TrainItem::TrainItem(Diagram& diagram, std::shared_ptr<TrainLine> line,
    Railway& railway, DiagramPage& page, double startY, QGraphicsItem* parent):
    QGraphicsItem(parent),
    _line(line),_diagram(diagram),_page(page),_railway(railway),
    startTime(page.config().start_hour,0,0),
    start_x(page.config().totalLeftMargin()),start_y(startY)
{
    _startAtThis = train()->isStartingStation(_line->firstStationName());
    _endAtThis = train()->isTerminalStation(_line->lastStationName());
    startLabelInfo = _page.startingNullLabel(_line->firstRailStation().get(),_line->dir());
    endLabelInfo = _page.terminalNullLabel(_line->lastRailStation().get(), _line->dir());
    pen = trainPen();

    // 如果这里报QtGui.dll的错误，考虑trainType()是不是空！
    setLine();
}

QRectF TrainItem::boundingRect() const
{
    return QRectF();
    if (pathItem)
        return _bounding;
    else
        return QRectF();
}

void TrainItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

std::shared_ptr<Train> TrainItem::train()
{
    return _line->train();
}

std::shared_ptr<const Train> TrainItem::train() const
{
    return _line->train();
}

void TrainItem::highlight()
{
    if (_isHighlighted)
        return;
    auto* path = pathItem;
    if (!path)
        return;

    //运行线加粗显示
    QPen pen = path->pen();
    pen.setWidthF(pen.widthF() + 1);
    path->setPen(pen);
    path->setZValue(2);

    QPen rectPen(pen.color());
    QBrush brush(pen.color());
    rectPen.setWidthF(0.5);
    pen.setWidthF(2);
    if (startLabelText) {
        //起点标签
        startRect = new QGraphicsRectItem(startLabelText->boundingRect(), this);
        startRect->setPos(startLabelText->pos());
        startRect->setZValue(5);
        startRect->setPen(rectPen);
        startRect->setBrush(brush);
        startLabelText->setZValue(6);
        startLabelText->setBrush(Qt::white);
    }
    if (endLabelText && config().end_label_name) {
        endRect = new QGraphicsRectItem(endLabelText->boundingRect(), this);
        endRect->setPos(endLabelText->pos());
        endRect->setZValue(5);
        endRect->setPen(rectPen);
        endRect->setBrush(brush);
        endLabelText->setZValue(6);
        endLabelText->setBrush(Qt::white);
    }

    //跨界点
    if (!spanItems.empty()) {
        QFont font = spanItems.at(0)->font();
        font.setUnderline(true);
        for (auto p : spanItems) {
            p->setFont(font);
        }
    }

    //显示详细停点
    if (config().show_time_mark == 1) {
        if (!markLabels.empty()) {
            //优化：如果已经搞过，不再重复
            for (auto p : markLabels)
                p->setVisible(true);
        }
        else
            addTimeMarks();
    }
    setZValue(10);
    _isHighlighted = true;
}

void TrainItem::unhighlight()
{
    if (!_isHighlighted)
        return;
    auto* path = pathItem;
    if (!path)
        return;

    //取消运行线加粗
    QPen pen = path->pen();
    pen.setWidthF(pen.widthF() - 1);
    path->setPen(pen);
    path->setZValue(0);

    if (startLabelText) {
        //pen.setWidth(1);
        //startLabelItem->setPen(pen);
        startLabelItem->setZValue(0);
        startLabelText->setZValue(0);
        startLabelText->setBrush(pen.color());
        startLabelText->scene()->removeItem(startRect);
        delete startRect;
        startRect = nullptr;
    }
    if (endLabelText && config().end_label_name) {
        //pen.setWidth(1);
        //endLabelItem->setPen(pen);
        endLabelItem->setZValue(0);
        endLabelText->setZValue(0);
        endLabelText->setBrush(pen.color());
        endLabelText->scene()->removeItem(endRect);
        delete endRect;
        endRect = nullptr;
    }

    if (!spanItems.empty()) {
        QFont font = spanItems.at(0)->font();
        font.setUnderline(false);
        for (auto p : spanItems) {
            p->setFont(font);
        }
    }
    setZValue(5); 

    if (config().show_time_mark == 1)
        hideTimeMarks();

    _isHighlighted = false;
}

void TrainItem::highlightWithLink()
{
    highlight();
    if (!_linkHighlighted) {
        QPen pen = trainPen();
        pen.setWidthF(pen.widthF() + 1);
        pen.setStyle(Qt::DashLine);
        if (linkItem1)
            linkItem1->setPen(pen);
        if (linkItem2) {
            linkItem2->setPen(pen);
        }
        _linkHighlighted = true;
    }
}

void TrainItem::unhighlightWithLink()
{
    unhighlight();
    if (_linkHighlighted) {
        QPen pen = trainPen();
        pen.setStyle(Qt::DashLine);
        if (linkItem1)
            linkItem1->setPen(pen);
        if (linkItem2)
            linkItem2->setPen(pen);
        _linkHighlighted = false;
    }
}

bool TrainItem::contains(const QPointF& f) const
{
    Q_UNUSED(f);
    return false;
}

#define DELETE_SUB(_item) \
if(_item){\
    delete _item;\
    _item=nullptr; \
}

TrainItem::~TrainItem() noexcept
{
    DELETE_SUB(pathItem);
    DELETE_SUB(expandItem);
    DELETE_SUB(startLabelItem);
    DELETE_SUB(startLabelText);
    DELETE_SUB(endLabelItem);
    DELETE_SUB(endLabelText);
    DELETE_SUB(startRect);
    DELETE_SUB(endRect);
    DELETE_SUB(linkItem1);
    DELETE_SUB(linkItem2);

    for (auto p : spanItems) {
        delete p;
    }
    spanItems.clear();

    for (auto p : markLabels)
        delete p;
    markLabels.clear();  

    clearLabelInfo();
}

void TrainItem::clearLabelInfo()
{
    if (!_page.hasLabelInfo())
        return;
    auto& sl = _page.startingLabels(_line->firstRailStation().get(), _line->dir());
    auto& se = _page.terminalLabels(_line->lastRailStation().get(), _line->dir());
    if (startLabelInfo != sl.end()) {
        sl.erase(startLabelInfo);
        startLabelInfo = sl.end();
    }
    if (endLabelInfo != se.end()) {
        se.erase(endLabelInfo);
        endLabelInfo = se.end();
    }
}

Direction TrainItem::dir() const
{
    return _line->dir();
}

const Config &TrainItem::config() const
{
    return _page.config();
}

const MarginConfig &TrainItem::margins() const
{
    return _page.margins();
}

void TrainItem::setLine()
{
    const QString& trainName = config().show_full_train_name ?
        train()->trainName().full() : train()->trainName().dirOrFull(_line->dir());

    setPathItem(trainName);

    QPen labelPen = trainPen();
    labelPen.setWidth(1);
    if (_line->startLabel() && startInRange) {
        setStartItem(trainName, labelPen);
    }
    if (_line->endLabel() && endInRange) {
        setEndItem(config().end_label_name ? trainName : " ",
            labelPen);
    }
    addLinkLine();
}

void TrainItem::setPathItem(const QString& trainName)
{
    //和图幅有关的数值
    double width = config().diagramWidth();

    bool started = false;    //是否已经开始铺画
    double ylast = -1, xlast = -1;
    bool inlast = false;   //上一个点是否在图幅内

    bool mark = (config().show_time_mark == 2);

    QPainterPath path;

    QList<double> spanLeft, spanRight;   //跨界点纵坐标

    auto lastIter = _line->stations().end(); --lastIter;

    for (auto p = _line->stations().begin(); p != _line->stations().end(); ++p) {
        auto ts = p->trainStation;
        auto rs = p->railStation.lock();
        double ycur = _railway.yValueFromCoeff(rs->y_coeff.value(), config());   // 绝对坐标
        double xarr = calXFromStart(ts->arrive), xdep = calXFromStart(ts->depart);

        //首先处理到达点
        if (xarr <= width) {
            //到达点在范围内，铺画到达点
            QPointF parr(xarr + start_x, ycur + start_y);
            if (!started) {
                if (startInRange) {
                    //表示这就是第一个站，p==begin()
                    startPoint = parr;
                }
                path.moveTo(parr);
                started = true;
            }
            else {
                if (xarr < xlast) {
                    //横坐标数值减小，表明出现左入图情况（跨界）
                    if (inlast) {
                        //上一个点在界内，就还要补充右出图的情况
                        spanRight.append(getOutGraph(xlast, ylast, xarr, ycur, path));
                    }
                    //现在：左入图操作
                    spanLeft.append(getInGraph(xlast, ylast, xarr, ycur, path));
                    path.lineTo(parr);
                }
                else {
                    path.lineTo(parr);
                }
            }
            if (mark && ts->isStopped()) {
                if (_line->startLabel() || p != _line->stations().begin()) {
                    markArriveTime(xarr, ycur, ts->arrive);
                }
            }
        }
        else {  //xarr > width  在图外
            if (!started) {
                startInRange = false;
            }
            if (inlast) {
                //补充右出图情况
                spanRight.append(getOutGraph(xlast, ylast, xarr, ycur, path));
            }
        }

        //下面处理出发点
        if (ts->isStopped()) {
            //存在停点，到点和开点不同
            if (xdep <= width) {
                //界内
                if (!started)  // 此条件：解决界外到达、界内出发的首站没有设置started的问题
                    started = true;
                QPointF pdep(start_x + xdep, start_y + ycur);
                if (xdep < xarr) {
                    //站内越界
                    if (xarr <= width) {
                        //到达点也在界内，先补充右出界
                        spanRight.append(getOutGraph(xarr, ycur, xdep, ycur, path));
                    }
                    //左入界
                    spanLeft.append(getInGraph(xarr, ycur, xdep, ycur, path));
                }
                if (config().show_line_in_station)
                    path.lineTo(pdep);
                else
                    path.moveTo(pdep);
            }
            else {
                //界外
                if (xarr <= width) {
                    spanRight.append(getOutGraph(xarr, ycur, xdep, ycur, path));
                }
            }
        }
        //标记时刻 无论有没有停点，都要标注开点，除非是折返车的最后一站
        if (mark && xdep <= width) {
            if (p == lastIter) {
                if (_line->endLabel() && !ts->isStopped()) {
                    markArriveTime(xdep, ycur, ts->depart);
                }
            }
            else{
                markDepartTime(xdep, ycur, ts->depart);
            }
        }

        ylast = ycur;
        xlast = xdep;
        inlast = (xlast <= width);
    }

    //最后一个站，以及终止点
    endInRange = inlast;
    if (endInRange) {
        endPoint = path.currentPosition();
    }
    QPen pen = trainPen();

    QPainterPathStroker stroker;
    stroker.setWidth(0.5);
    auto outpath = stroker.createStroke(path);
    pathItem = new QGraphicsPathItem(outpath, this);
    pathItem->setPen(pen);
    if (config().valid_width > 1) {
        QPen expen(Qt::transparent, pen.width() * config().valid_width);
        expandItem = new QGraphicsPathItem(outpath, this);
        expandItem->setPen(expen);
        //_bounding = expandItem->boundingRect();
    }
    else {
        //_bounding = pathItem->boundingRect();
    }

    //跨界点标记
    QFont font;
    bool calculated = false;
    double sw = -1, sh = -1;    //item宽度
    for (auto p : spanLeft) {
        auto* item = setStartEndLabelText(trainName, pen.color());
        if (!calculated) {
            setStretchedFont(font, item, width);
            const auto& t = item->boundingRect();
            sw = t.width(); sh = t.height();
            calculated = true;
        }
        else
            item->setFont(font);
        item->setPos(start_x - sw, p - sh / 2 + start_y);
        spanItems.append(item);
        //_bounding |= item->boundingRect();
    }
    for (auto p : spanRight) {
        auto* item = setStartEndLabelText(trainName, pen.color());
        if (!calculated) {
            setStretchedFont(font, item, width);
            const auto& t = item->boundingRect();
            sw = t.width(); sh = t.height();
            calculated = true;
        }
        else
            item->setFont(font);
        item->setPos(start_x + width, p - sh / 2 + start_y);
        spanItems.append(item);
        //_bounding |= item->boundingRect();
    }
}

void TrainItem::setStartItem(const QString& text,const QPen& pen)
{
    QPainterPath label(startPoint);
    startLabelText = setStartEndLabelText(text, pen.color());
    double height = determineStartLabelHeight();
    startLabelHeight = height;
    const auto& t = startLabelText->boundingRect();
    double w = t.width(), h = t.height();

    double x0 = startPoint.x(), y0 = startPoint.y();

    if (_startAtThis) {
        //本线始发
        if (_line->dir() == Direction::Down) {
            QPointF pn(startPoint.x(), startPoint.y() - height);
            label.lineTo(pn);
            label.moveTo(pn.x() - w / 2, pn.y());
            label.lineTo(pn.x() + w / 2, pn.y());
            startLabelText->setPos(pn.x() - w / 2, pn.y() - h);
        }
        else {
            QPointF pn(startPoint.x(), startPoint.y() + height);
            label.lineTo(pn);
            label.moveTo(pn.x() - w / 2, pn.y());
            label.lineTo(pn.x() + w / 2, pn.y());
            startLabelText->setPos(pn.x() - w / 2, pn.y());
        }	
    }
    else {   //not startAtThis
        if (_line->dir() == Direction::Down) {
            label.lineTo(x0, y0 - height);
            label.lineTo(x0 - w, y0 - height);
            label.lineTo(x0 - w - h, y0 - height - h);
            startLabelText->setPos(x0 - w, y0 - height - h);
        }
        else {
            label.lineTo(x0, y0 + height);
            label.lineTo(x0 - w, y0 + height);
            label.lineTo(x0 - w - h, y0 + height + h);
            startLabelText->setPos(x0 - w, y0 + height);
        }
    }
    startLabelItem = new QGraphicsPathItem(label, this);
    startLabelItem->setPen(pen);
    _bounding |= startLabelItem->boundingRect();
    _bounding |= startLabelText->boundingRect();
}

void TrainItem::setEndItem(const QString& text, const QPen& pen)
{
    QPainterPath label(endPoint);
    double x0 = endPoint.x(), y0 = endPoint.y();
    endLabelText = setStartEndLabelText(text, pen.color());
    const auto& t = endLabelText->boundingRect();
    double w = t.width(), h = t.height();
    if (!config().end_label_name) 
        w = 0;
    double height = determineEndLabelHeight();
    endLabelHeight = height;

    double beh = config().base_label_height;

    if (_line->dir() == Direction::Down) {
        if (_endAtThis) {
            //curPoint.setY(curPoint.y() + eh - beh / 2)
            double y1 = y0 + height - beh / 2;   //三角形底边中点
            label.lineTo(x0, y1);
            label.addPolygon(QPolygonF(QVector<QPointF>{
                {x0 - beh / 3, y1},
                { x0 + beh / 3,y1 },
                { x0,y0 + height },
                { x0 - beh / 3, y1 },
            }));
            label.moveTo(x0 - w / 2, (y0 += height));
            label.lineTo(x0 + w / 2, y0);
            endLabelText->setPos(x0 - w / 2, y0);
        }
        else {
            label.lineTo(x0, (y0 += height));
            label.lineTo(x0 + w + h, y0);
            label.lineTo(x0 + w, y0 + h);
            endLabelText->setPos(x0, y0);
        }
    }
    else {  //not down
        if (_endAtThis) {
            double y1 = y0 - height + beh / 2;   //三角形底边中点
            label.lineTo(x0, y1);
            label.addPolygon(QPolygonF(QVector<QPointF>{
                {x0 - beh / 3, y1},
                { x0 + beh / 3,y1 },
                { x0,y0 - height },
                { x0 - beh / 3, y1 }
            }));
            label.moveTo(x0 - w / 2, (y0 -= height));
            label.lineTo(x0 + w / 2, y0);
            endLabelText->setPos(x0 - w / 2, y0 - h);
        }
        else {
            label.lineTo(x0, (y0 -= height));
            label.lineTo(x0 + w + h, y0);
            label.lineTo(x0 + w, y0 - h);
            endLabelText->setPos(x0, y0 - h);
        }
    }
    endLabelItem = new QGraphicsPathItem(label, this);
    endLabelItem->setPen(pen);
    _bounding |= endLabelItem->boundingRect();
    _bounding |= endLabelText->boundingRect();
}

QGraphicsSimpleTextItem* TrainItem::setStartEndLabelText(const QString& text, const QColor& color)
{
    auto* item = new QGraphicsSimpleTextItem(text, this);
    item->setBrush(color);
    if (spanItemWidth < 0) {
        //没设置过
        const auto& t = item->boundingRect();
        spanItemWidth = t.width();
        spanItemHeight = t.height();
    }
    return item;
}

double TrainItem::calXFromStart(const QTime& time) const
{
    int sec = startTime.secsTo(time);
    if (sec < 0) 
        sec += 24 * 3600;
    return sec / config().seconds_per_pix;
}

double TrainItem::getOutGraph(double xin, double yin, double xout, double yout, 
    QPainterPath& path)
{
    double fullwidth = config().fullWidth();
    double width = config().diagramWidth();
    double xright = xout + fullwidth;
    double yp = yin + (width - xin) * (yout - yin) / (xright - xin);
    QPointF pout(start_x + width, start_y + yp);
    path.lineTo(pout);
    return yp;
}

double TrainItem::getInGraph(double xout, double yout, double xin, double yin, QPainterPath& path)
{
    double fullwidth = config().fullWidth();
    double xleft = xout - fullwidth;
    double yp = yout - xleft * (yin - yout) / (xin - xleft);  //入图点纵坐标
    QPointF pin(start_x, yp + start_y);
    path.moveTo(pin);
    return yp;
}

const QPen& TrainItem::trainPen() const
{
    return train()->pen();
}

QColor TrainItem::trainColor() const
{
    return QColor(Qt::red);
}

double TrainItem::determineStartLabelHeight()
{
    if(!config().avoid_cover)
        return config().start_label_height;
    double x = startPoint.x();    //既然调用了这个，那endPoint就一定有效
    double w = startLabelText->boundingRect().width();
    int wl, wr;
    if (_startAtThis) {
        wl = wr = w / 2;
    }
    else {
        wl = w; wr = 0;
    }
    auto rst = _line->firstRailStation();
    if (_line->dir() == Direction::Down) {
        startLabelInfo = determineLabelHeight(_page.overLabels(rst.get()), x, wl, wr);
    }
    else {
        startLabelInfo = determineLabelHeight(_page.belowLabels(rst.get()), x, wl, wr);
    }
    return startLabelInfo->second.height;
}

double TrainItem::determineEndLabelHeight()
{
    if (!config().avoid_cover)
        return config().end_label_height;
    else if (!config().end_label_name)
        return config().base_label_height;
    double x = endPoint.x();    //既然调用了这个，那endPoint就一定有效
    double w = endLabelText->boundingRect().width();
    int wl, wr;
    if (_endAtThis) {
        wl = wr = w / 2;
    }
    else {
        wl = 0; wr = w;
    }
    auto rst = _line->lastRailStation();
    if (dir() == Direction::Down) {
        endLabelInfo = determineLabelHeight(_page.belowLabels(rst.get()), x, wl, wr);
    }
    else {
        endLabelInfo = determineLabelHeight(_page.overLabels(rst.get()), x, wl, wr);
    }
    return endLabelInfo->second.height;
}

std::multimap<double, LabelPositionInfo>::iterator
TrainItem::determineLabelHeight(std::multimap<double, LabelPositionInfo>& spans,
    double xcenter, double left, double right)
{
    double start = xcenter - MAX_COVER_WIDTH;
    auto p = spans.lower_bound(start);
    QSet<double> occupied;   //已经占据了的高度表
    for (; p != spans.end() && p->first <= xcenter + MAX_COVER_WIDTH; ++p) {
        const auto& info = p->second;
        if (std::max(xcenter - left, p->first - info.left) <=
            std::min(xcenter + right, p->first + info.right)) {
            occupied.insert(info.height);
        }
    }
    double h = config().base_label_height;
    while (occupied.contains(h))
        h += config().step_label_height;
    return spans.insert({ xcenter,{h,left,right} });
}

void TrainItem::setStretchedFont(QFont& font, QGraphicsSimpleTextItem* item, double width)
{
    const auto& t = item->boundingRect();
    double w = t.width();
    if (w > width) {
        int stretch = 100 * width / w;
        font.setStretch(stretch);
        item->setFont(font);
    }
}

void TrainItem::addTimeMarks()
{
    auto lastIter = _line->stations().end(); --lastIter;
    for (auto p = _line->stations().begin(); p != _line->stations().end(); ++p) {
        auto ts = p->trainStation;
        auto rs = p->railStation.lock();
        double ycur = _railway.yValueFromCoeff(rs->y_coeff.value(), config());
        double xarr = calXFromStart(ts->arrive), xdep = calXFromStart(ts->depart);

        //标注到点  
        //注意最后一站如果无停点是按照到达来标注的
        if (ts->isStopped() || p == lastIter) {
            if (_line->startLabel() || p != _line->stations().begin()) {
                markArriveTime(xarr, ycur, ts->arrive);
            }
        }

        //标注开点
        //2022.05.18修正逻辑：无论有没有停点，都要标注开点；
        //除非是结束无标签的最后一站。
        //最后一站：仅当有停点且有结束标签时，才标记开点
        if (p != lastIter || (ts->isStopped() && _line->endLabel())) {
            markDepartTime(xdep, ycur, ts->depart);
        }
    }
}

void TrainItem::hideTimeMarks()
{
    for (auto p : markLabels) {
        p->setVisible(false);
    }
}

void TrainItem::markArriveTime(double x, double y, const QTime& tm)
{
    //到达时刻：下行标右上，下行标右下
    int m = tm.minute();
    if (tm.second() >= 30)m++;
    auto* item = new QGraphicsSimpleTextItem(QString::number(m % 10), this);
    item->setBrush(pen.color());
    double h = item->boundingRect().height();
    if (dir() == Direction::Down) {
        item->setPos(x + start_x, y - h + start_y);
    }
    else {
        item->setPos(x + start_x, y + start_y);
    }
    markLabels.append(item);
}

void TrainItem::markDepartTime(double x, double y, const QTime& tm)
{
    //出发时刻 下行标左下，上行标左上
    int m = tm.minute();
    if (tm.second() >= 30)m++;
    auto* item = new QGraphicsSimpleTextItem(QString::number(m % 10), this);
    item->setBrush(pen.color());
    const auto& t = item->boundingRect();
    double w = t.width(), h = t.height();
    if (dir() == Direction::Down) {
        item->setPos(x - w + start_x, y + start_y);
    }
    else {
        item->setPos(x - w + start_x, y - h + start_y);
    }
    markLabels.append(item);
}

void TrainItem::addLinkLine()
{
    if (!train()->hasRouting())
        return;
    std::shared_ptr<Routing> rout = train()->routing().lock();
    auto* pre = rout->preLinked(*train());   // 这里已经保证first, last是同一个车站
    if (!pre)
        return;
    auto last = pre->train()->lastStation();
    auto first = train()->firstStation();
    if (train()->isNullStation(first) || pre->train()->isNullStation(last))
        return;
    if (first != _line->firstTrainStation()) {
        // 2021.09.02补正  必须是本线的才绘制
        return;
    }
    // 到这里：已经判断好了绘制条件
    const QTime& last_tm = last->depart;
    const QTime& first_tm = first->arrive;
    //2021.10.09修改：这里必须限制Railway。有可能始发站绑定到了多条线路。
    auto rs = train()->boundStartingAtRail(_railway);
    double xcur = calXFromStart(first_tm);
    double xpre = calXFromStart(last_tm);

    double width = config().diagramWidth();
    double y = _railway.yValueFromCoeff(rs->y_coeff.value(), config()) + start_y;
    QPen pen = trainPen();
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);

    if (xcur >= xpre) {
        //无需跨界
        if (xpre <= width) {
            linkItem1 = new QGraphicsLineItem(
                config().totalLeftMargin() + xpre, y, 
                config().totalLeftMargin() + std::min(width, xcur), y, this
            );
            linkItem1->setPen(pen);
        }
    }
    else {
        //跨界
        //左边的 一定存在
        linkItem1 = new QGraphicsLineItem(
            config().totalLeftMargin(), y, config().totalLeftMargin() + xcur, y, this
        );
        linkItem1->setPen(pen);
        //右边的
        if (xpre <= width) {
            linkItem2 = new QGraphicsLineItem(
                config().totalLeftMargin() + xpre, y, 
                config().totalLeftMargin() + width, y, this
            );
            linkItem2->setPen(pen);
        }
    }
}
