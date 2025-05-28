#include "trainitem.h"
//#include "data/diagram/trainadapter.h"
#include "data/diagram/trainline.h"
#include "data/train/routing.h"
#include "data/diagram/diagram.h"
#include "data/rail/railway.h"
#include "paintstationpointitem.h"
#include "data/common/qesystem.h"
#include "qemultilinepath.h"
#include "util/utilfunc.h"

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
    pen = line->train()->pen();
    if (config().inverse_color) {
        pen.setColor(qeutil::inversedColor(pen.color()));
    }

    // 如果这里报QtGui.dll的错误，考虑trainType()是不是空！
    setLine();
}

QRectF TrainItem::boundingRect() const
{
    return QRectF{};
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
    QBrush brush(pen.color());
    if (config().train_label_color == Config::LinkLineColorOption::TextColor) {
        brush.setColor(config().text_color_masked());
    }
    QPen rectPen(brush.color());

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
        startLabelText->setBrush(config().background_color_masked());   // 2024.06.27: use bg-color for highlighted train name
    }
    if (endLabelText && config().end_label_name) {
        endRect = new QGraphicsRectItem(endLabelText->boundingRect(), this);
        endRect->setPos(endLabelText->pos());
        endRect->setZValue(5);
        endRect->setPen(rectPen);
        endRect->setBrush(brush);
        endLabelText->setZValue(6);
        endLabelText->setBrush(config().background_color_masked());
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

    //显示铺画点
    if (SystemJson::instance.drag_time) {
        if (!stationMarks.empty()) {
            for (auto p : stationMarks) {
                p->setVisible(true);
            }
        }
        else {
            addStationPoints();
        }
    }

    //显示交路连线
    if (config().show_link_line == 1) {
        if (hasLinkLine) {
            if (linkItem1 || linkItem2) {
                if (linkItem1) {
                    linkItem1->setVisible(true);
                }
                if (linkItem2) {
                    linkItem2->setVisible(true);
                }
                if (linkLabelItem) {
                    linkLabelItem->setVisible(true);
                }
            }
            else {
                // 连线没有铺画过。尝试一次绘制
                hasLinkLine = addLinkLine(labelTrainName());
            }
        }
    }

    // 突出显示交路连线车次标记
    if (config().train_name_mark_style == Config::TrainNameMarkStyle::Link && linkLabelItem) {
        if (linkLabelRect) {
            linkLabelRect->show();
        }
        else {
            linkLabelRect = new QGraphicsRectItem(linkLabelItem->boundingRect(), this);
            linkLabelRect->setPos(linkLabelItem->pos());
            linkLabelRect->setZValue(5);
            linkLabelRect->setPen(rectPen);
            linkLabelRect->setBrush(brush);
        }
        linkLabelItem->setZValue(6);
        linkLabelItem->setBrush(Qt::white);
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
    QColor labelColor = pen.color();
    if (config().train_label_color == Config::LinkLineColorOption::TextColor) {
        labelColor = config().text_color_masked();
    }
    pen.setWidthF(pen.widthF() - 1);

    path->setPen(pen);
    path->setZValue(0);

    if (startLabelText) {
        //pen.setWidth(1);
        //startLabelItem->setPen(pen);
        startLabelItem->setZValue(0);
        startLabelText->setZValue(0);
        startLabelText->setBrush(labelColor);
        startLabelText->scene()->removeItem(startRect);
        delete startRect;
        startRect = nullptr;
    }
    if (endLabelText && config().end_label_name) {
        //pen.setWidth(1);
        //endLabelItem->setPen(pen);
        endLabelItem->setZValue(0);
        endLabelText->setZValue(0);
        endLabelText->setBrush(labelColor);
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

    hideStationPoints();
    hideLinkLine();

    if (linkLabelRect) {
        linkLabelRect->hide();
        linkLabelItem->setZValue(0);
        linkLabelItem->setBrush(labelColor);
    }

    _isHighlighted = false;
}

void TrainItem::highlightWithLink()
{
    highlight();
    if (!_linkHighlighted) {
        QPen pen = trainPen();
        if (config().link_line_color == Config::LinkLineColorOption::TextColor) {
            pen.setColor(config().text_color_masked());
        }
        pen.setWidthF(LINK_LINE_WIDTH + 1);
        pen.setStyle(linkLineStyle());
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
        if (config().link_line_color == Config::LinkLineColorOption::TextColor) {
            pen.setColor(config().text_color_masked());
        }
        pen.setWidth(LINK_LINE_WIDTH);
        pen.setStyle(linkLineStyle());
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

void TrainItem::repaintLinkLine()
{
    if (!hasLinkLine)
        return;
    clearLinkLines();
    hasLinkLine = addLinkLine(labelTrainName());
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
    DELETE_SUB(linkLabelItem);
    DELETE_SUB(linkLabelRect);

    for (auto p : spanItems) {
        delete p;
    }
    spanItems.clear();

    for (auto p : markLabels)
        delete p;
    markLabels.clear();  

    clearLabelInfo();
    clearLinkInfo();
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

void TrainItem::clearLinkInfo()
{
    if (!_page.hasLinkInfo())
        return;
    
    auto& linksStart = _page.dirLinks(_line->firstRailStation().get(), dir());

    // note: the layer >= 0 check is performed inside the function.
    linksStart.delOccupation(linkLayer.layer, train().get(), linkLayer.x_pre, linkLayer.x_cur, config().fullWidth());
    linksStart.delOccupation(startLayer.layer, train().get(), startLayer.x_pre, startLayer.x_cur, config().fullWidth());

    if (endLayer.layer >= 0) {
        auto& linksEnd = _page.dirLinks(_line->lastRailStation().get(), dir());
        linksEnd.delOccupation(endLayer.layer, train().get(), endLayer.x_pre, endLayer.x_cur, config().fullWidth());
    }
}

Direction TrainItem::dir() const
{
    return _line->dir();
}

#if 0
bool TrainItem::dragBegin(const QPointF& pos, PaintStationPointItem* point, bool ctrl, bool alt)
{
    _onDragging = false;
    _draggedStation = nullptr;

    if (train()->isOnPainting()) {
        // not process for paiting trains for now
        return false;
    }

    double ycoef = _railway.yCoeffFromAbsValue(pos.y()-start_y, config());
    auto itr = _line->stationFromYCoeffClosest(ycoef);
    if (itr == _line->stations().end())
        return false;
     
    constexpr const int MAX_DIFF = 20;
    if (std::abs(ycoef - itr->yCoeff()) > MAX_DIFF)
        return false;
    
    double dis_arr = std::abs(timeDistancePbc(pos.x(), itr->trainStation->arrive));
    double dis_dep = std::abs(timeDistancePbc(pos.x(), itr->trainStation->depart));

    double dis_min = std::min(dis_arr, dis_dep);
    if (dis_min > MAX_DIFF)
        return false;
    if (itr->trainStation->isStopped()) {
        if (dis_dep < dis_arr) {
            // is depart time
            _dragPoint = StationPoint::Depart;
        }
        else {
            _dragPoint = StationPoint::Arrive;
        }
    }
    else {
        if (ctrl) {
            _dragPoint = StationPoint::Arrive;
        }
        else if (alt) {
            _dragPoint = StationPoint::Depart;
        }
        else {
            _dragPoint = StationPoint::Pass;
        }
    }

    _draggedStation = &*itr;
    _onDragging = true;

    qDebug() << "dragBegin: on station " << _draggedStation->trainStation->name.toSingleLiteral() << Qt::endl;

    return true;
}
#else 

// 2023.06.04  new version using PaintStationPointItem
bool TrainItem::dragBegin(const QPointF& pos, PaintStationPointItem* point, bool ctrl, bool alt, bool shift)
{
    Q_UNUSED(pos);
    _onDragging = false;
    _draggedStation = nullptr;

    if (train()->isOnPainting()) {
        // not process for paiting trains for now
        return false;
    }

    _dragPoint = point->point();
    
    if (_dragPoint == StationPoint::Pass) {
        // 2024.03.26: for shift case, ctrl-alt-shift means drag backward from arrival time
        if ((ctrl && !shift) || (ctrl && alt &&shift)) {
            _dragPoint = StationPoint::Arrive;
        }
        else if (alt) {
            _dragPoint = StationPoint::Depart;
        }
    }

    if (shift) {
        _dragTrans = ctrl ? DragTranslationType::Backward : DragTranslationType::Forward;
    }
    else {
        _dragTrans = DragTranslationType::SinglePoint;
    }

    _draggedStation = point->station();
    _onDragging = true;

    //qDebug() << "station: " << point->station() << Qt::endl;

    //qDebug() << "dragBegin: on station " << _draggedStation->trainStation->name.toSingleLiteral() << Qt::endl;

    return true;
}

#endif

QTime TrainItem::posToTime(const QPointF& pos) const
{
    return calTimeByXFromStart(pos.x() - start_x);
}

const QString& TrainItem::dragPointString() const
{
    static const QString
        _arrive = QObject::tr("到达"),
        _depart = QObject::tr("出发"),
        _pass = QObject::tr("通过"),
        _invalid = QObject::tr("!INVALID!");
    switch (_dragPoint) {
    case StationPoint::Arrive: return _arrive;
    case StationPoint::Depart: return _depart;
    case StationPoint::Pass: return _pass;
    default: return _invalid;
    }
}

QTime TrainItem::draggedOldTime() const
{
    switch (_dragPoint)
    {
    case StationPoint::Arrive:
    case StationPoint::Pass:
        return _draggedStation->trainStation->arrive;
        break;
    case StationPoint::Depart:
        return _draggedStation->trainStation->depart;
        break;
    default:
        // INVALID !!
        return {};
        break;
    }
}

void TrainItem::doDrag(const QTime& tm, bool shift)
{
    if (!_onDragging) {
        qDebug() << "TrainItem::doDrag: ERROR: unexpected status" << Qt::endl;
        return;
    }
    if (shift) {
        // 2024.03.26: move the part of WHOLE LINE
        // first: process the selected point
        auto tm_old = draggedOldTime();
        int secs = tm_old.secsTo(tm);

        doDragSingle(tm);
        if (_dragTrans == DragTranslationType::Forward) {
            if (_dragPoint == StationPoint::Arrive) {
                _draggedStation->trainStation->depart =
                    _draggedStation->trainStation->depart.addSecs(secs);
            }
            auto itr = std::next(_draggedStation->trainStation);
            for (; itr != train()->timetable().end(); ++itr) {
                itr->arrive = itr->arrive.addSecs(secs);
                itr->depart = itr->depart.addSecs(secs);
            }
        }
        else if (_dragTrans == DragTranslationType::Backward) {
            if (_dragPoint == StationPoint::Depart) {
                _draggedStation->trainStation->arrive =
                    _draggedStation->trainStation->arrive.addSecs(secs);
            }
            auto itr = std::make_reverse_iterator(_draggedStation->trainStation);
            for (; itr != train()->timetable().rend(); ++itr) {
                itr->arrive = itr->arrive.addSecs(secs);
                itr->depart = itr->depart.addSecs(secs);
            }
        }
        
    }
    else {
        // original: just move single station
        doDragSingle(tm);
    }

    // finalize
    _onDragging = false;
    _draggedStation = nullptr;
    _dragPoint = StationPoint::NotValid;
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
    const QString& trainName = labelTrainName();

    setPathItem(trainName);

    QPen labelPen = trainPen();
    labelPen.setWidth(0.5);
    labelPen.setStyle(Qt::SolidLine);
    if (config().train_label_color == Config::LinkLineColorOption::TextColor) {
        labelPen.setColor(config().text_color_masked());
    }

    // 2024.02.09: add global config here
    bool glb_has_start_label = ((_startAtThis && !config().hide_start_label_starting) 
        || (!_startAtThis && !config().hide_start_label_non_starting));
    bool glb_has_end_label = ((_endAtThis && !config().hide_end_label_terminal)
        || (!_endAtThis && !config().hide_end_label_non_terminal));

    // 2024.02.28  for link line mode: we should determine the existence of linkLine BEFORE adding start item..
    if (config().show_link_line == 2)
        hasLinkLine = addLinkLine(trainName);
    else {
        // 2025.05.28: we must init hasLinkLine in this case because this is used in setStart/EndItem function
        // (at the guard statements at the beginning)
        const auto& t = train();
        hasLinkLine = t->hasRouting() && train()->routing().lock()->preLinkedOnRailway(*t, _railway);
    }

    if (glb_has_start_label && _line->startLabel() && startInRange) {
        setStartItem(trainName, labelPen);
    }
    if (glb_has_end_label && _line->endLabel() && endInRange) {
        setEndItem(config().end_label_name ? trainName : " ",
            labelPen);
    }
}

void TrainItem::setPathItem(const QString& trainName)
{
    //和图幅有关的数值
    double width = config().diagramWidth();

    bool started = false;    //是否已经开始铺画
    double ylast = -1, xlast = -1;
    bool inlast = false;   //上一个点是否在图幅内

    bool mark = (config().show_time_mark == 2);

    //QPainterPath path;
    QEMultiLinePath path;

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
            // else  // 2024.02.15: no-else; for the first paining point may not be the actual first?
            {
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
                //qDebug() << "Out of graph: " << train()->trainName().full() << " "
                //    << xlast << ", " << ylast << ", " << xarr << ", " << ycur;
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

    //QPainterPathStroker stroker;
    //stroker.setWidth(0.5);
    //auto outpath = stroker.createStroke(path);

    pathItem = new QGraphicsPathItem(path.path(), this);
    pathItem->setPen(pen);
    if (config().valid_width > 1) {
        QPen expen(Qt::transparent, pen.width() * config().valid_width);
        expandItem = new QGraphicsPathItem(path.path(), this);
        expandItem->setPen(expen);
        //_bounding = expandItem->boundingRect();
    }
    else {
        //_bounding = pathItem->boundingRect();
    }

    //跨界点标记
    QFont font = config().train_font;
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

QString TrainItem::labelTrainName() const
{
    return config().show_full_train_name ?
        train()->trainName().full() : train()->trainName().dirOrFull(_line->dir());
}

void TrainItem::setStartItem(const QString& text,const QPen& pen)
{
    // 2024.02.28: for link
    if (config().train_name_mark_style == Config::TrainNameMarkStyle::Link) {
        if (hasLinkLine)
            return;
    }

    QEMultiLinePath label(startPoint);
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
    //QPainterPathStroker s;
    //s.setWidth(0.01);
    //auto label_out = s.createStroke(label);
    startLabelItem = new QGraphicsPathItem(label.path(), this);
    startLabelItem->setPen(pen);
    _bounding |= startLabelItem->boundingRect();
    _bounding |= startLabelText->boundingRect();
}

void TrainItem::setEndItem(const QString& text, const QPen& pen)
{
    if (config().hide_end_label_link) {
        auto routw = train()->routing();
        if (!routw.expired()) {
            auto rout = routw.lock();
            if (rout->postLinkedOnRailway(*train(), _railway)) {
                return;
            }
        }
    }
    QEMultiLinePath label(endPoint);
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
                //{ x0 - beh / 3, y1 },
            }));
            label.closeSubPath();
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
    //QPainterPathStroker s;
    //s.setWidth(0.01);
    //auto label_out = s.createStroke(label);
    endLabelItem = new QGraphicsPathItem(label.path(), this);
    endLabelItem->setPen(pen);
    _bounding |= endLabelItem->boundingRect();
    _bounding |= endLabelText->boundingRect();
}

QGraphicsSimpleTextItem* TrainItem::setStartEndLabelText(const QString& text, const QColor& color)
{
    auto* item = new QGraphicsSimpleTextItem(text, this);
    item->setBrush(color);
    item->setFont(config().train_font);
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

QTime TrainItem::calTimeByXFromStart(double x_from_start) const
{
    int sec = (int)std::round(x_from_start * config().seconds_per_pix);
    return startTime.addSecs(sec);
}

double TrainItem::getOutGraph(double xin, double yin, double xout, double yout, 
    QEMultiLinePath& path)
{
    // Note, that xout may be out of graph or actually in graph (in this case, xout < xin)
    double fullwidth = config().fullWidth();
    double width = config().diagramWidth();
    double xright = xout < xin ? xout + fullwidth: xout;
    double yp = yin + (width - xin) * (yout - yin) / (xright - xin);
    QPointF pout(start_x + width, start_y + yp);
    path.lineTo(pout);
    return yp;
}

double TrainItem::getInGraph(double xout, double yout, double xin, double yin, QEMultiLinePath& path)
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
    return this->pen;
}

QColor TrainItem::trainColor() const
{
    return pen.color();
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

    if (config().train_name_mark_style == Config::TrainNameMarkStyle::Link) {
        // debug
        //if (train()->trainName().down() == "D4803") {
        //    qDebug() << "hit break point";
        //}

        double width = config().diagramWidth();
        // 2024.03.01: link-style train name marker
        double left_marg = config().totalLeftMargin();
        startLayer.x_pre = std::max(x - wl - left_marg, 0.);
        startLayer.x_cur = std::min(x + wr - left_marg, width);
        startLayer.layer = linkLineLayer(rst.get(), startLayer.x_pre, startLayer.x_cur);
        //qDebug() << "start layer: " << startLayer.layer << ", " << startLayer.x_pre << ", " << startLayer.x_cur;
        return config().base_link_height + startLayer.layer * config().step_link_height;
    }
    else {
        if (_line->dir() == Direction::Down) {
            startLabelInfo = determineLabelHeight(_page.overLabels(rst.get()), x, wl, wr);
        }
        else {
            startLabelInfo = determineLabelHeight(_page.belowLabels(rst.get()), x, wl, wr);
        }
        return startLabelInfo->second.height;
    }
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

    if (config().train_name_mark_style == Config::TrainNameMarkStyle::Link) {
        double width = config().diagramWidth();
        // 2024.03.01: link-style train name marker
        double left_marg = config().totalLeftMargin();
        endLayer.x_pre = std::max(x - wl - left_marg, 0.);
        endLayer.x_cur = std::min(x + wr - left_marg, width);
        endLayer.layer = linkLineLayerEnd(rst.get(), endLayer.x_pre, endLayer.x_cur);
        return config().base_link_height + endLayer.layer * config().step_link_height;
    }
    else {
        if (dir() == Direction::Down) {
            endLabelInfo = determineLabelHeight(_page.belowLabels(rst.get()), x, wl, wr);
        }
        else {
            endLabelInfo = determineLabelHeight(_page.overLabels(rst.get()), x, wl, wr);
        }
        return endLabelInfo->second.height;
    }

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
    auto lastIter = std::prev(_line->stations().end());
    auto width = config().diagramWidth();
    for (auto p = _line->stations().begin(); p != _line->stations().end(); ++p) {
        auto ts = p->trainStation;
        auto rs = p->railStation.lock();
        double ycur = _railway.yValueFromCoeff(rs->y_coeff.value(), config());
        double xarr = calXFromStart(ts->arrive), xdep = calXFromStart(ts->depart);

        //标注到点  
        //注意最后一站如果无停点是按照到达来标注的
        if (xarr <= width && (ts->isStopped() || p == lastIter)) {
            if (_line->startLabel() || p != _line->stations().begin()) {
                markArriveTime(xarr, ycur, ts->arrive);
            }
        }

        //标注开点
        //2022.05.18修正逻辑：无论有没有停点，都要标注开点；
        //除非是结束无标签的最后一站。
        //最后一站：仅当有停点且有结束标签时，才标记开点
        if (xdep <= width && (p != lastIter || (ts->isStopped() && _line->endLabel()))) {
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

void TrainItem::addStationPoints()
{
    auto width = config().diagramWidth();
    for (auto p = _line->stations().begin(); p != _line->stations().end(); ++p) {
        auto ts = p->trainStation;
        auto rs = p->railStation.lock();
        double ycur = _railway.yValueFromCoeff(rs->y_coeff.value(), config());
        double xarr = calXFromStart(ts->arrive), xdep = calXFromStart(ts->depart);

        //qDebug() << "addStationPoints: station, id " << p->trainStation->name.toSingleLiteral() << ", "
        //    << &*p << Qt::endl;

        // arrive point
        if (xarr <= width) {
            auto arr_point_type = (ts->isStopped() ? StationPoint::Arrive : StationPoint::Pass);
            auto* itarr = new PaintStationPointItem(arr_point_type, &*p, xarr + start_x, ycur + start_y, this);
            itarr->setZValue(6);
            itarr->setBrush(pen.color());
            itarr->setPen(pen);
            stationMarks.push_back(itarr);
        }

        // depart point
        if (xdep <= width && ts->isStopped()) {
            auto* itdep = new PaintStationPointItem(StationPoint::Depart, &*p, xdep + start_x, ycur + start_y, this);
            itdep->setBrush(pen.color());
            itdep->setPen(pen);
            itdep->setZValue(6);
            stationMarks.push_back(itdep);
        }
    }
}

void TrainItem::hideStationPoints()
{
    for (auto p : stationMarks) {
        p->hide();
    }
}

void TrainItem::hideLinkLine()
{
    // 取消交路连线显示
    if (config().show_link_line == 1) {
        if (linkItem1)
            linkItem1->setVisible(false);
        if (linkItem2)
            linkItem2->setVisible(false);
        if (linkLabelItem)
            linkLabelItem->setVisible(false);
    }
}

void TrainItem::markArriveTime(double x, double y, const QTime& tm)
{
    //到达时刻：下行标右上，下行标右下
    int m = tm.minute();
    // 2024.03.27: options for different round strategies
    if (config().second_round_option == Config::SecondRoundOption::Round &&
        tm.second() >= 30)
        m++;
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
    if (config().second_round_option == Config::SecondRoundOption::Round && tm.second() >= 30)
        m++;
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

bool TrainItem::addLinkLine(const QString& trainName)
{
    if (!train()->hasRouting())
        return false;
    std::shared_ptr<Routing> rout = train()->routing().lock();
    auto* pre = rout->preLinkedOnRailway(*train(), _railway);   // 这里已经保证first, last是同一个车站
    if (!pre)
        return false;
    auto last = pre->train()->lastStation();
    auto first = train()->firstStation();
    if (train()->isNullStation(first) || pre->train()->isNullStation(last))
        return false;
    if (first != _line->firstTrainStation()) {
        // 2021.09.02补正  必须是本线的才绘制
        return false;
    }
    // 到这里：已经判断好了绘制条件
    const QTime& last_tm = last->depart;
    const QTime& first_tm = first->arrive;
    //2021.10.09修改：这里必须限制Railway。有可能始发站绑定到了多条线路。
    auto rs = train()->boundStartingAtRail(_railway);
    double xcur = calXFromStart(first_tm);
    double xpre = calXFromStart(last_tm);
    double xpre_eff = xpre;     // For determining the starting of occupation. Mainly prepared for long text case.
    linkLayer.x_pre = xpre;
    linkLayer.x_cur = xcur;

    // 2024.02.26: add (optional) link label
    auto label_text = linkLineLabelText(trainName, rout.get());

    double width = config().diagramWidth();

    // 2025.02.22  add condition xpre <= width: do not create the label for the link lines out of the boundary.
    if (!label_text.isEmpty() && xpre <= width) {
        linkLabelItem = new QGraphicsSimpleTextItem(label_text, this);
        linkLabelItem->setFont(config().train_font);
        linkLabelItem->setBrush(labelColor());

        // 对于不跨界的连线，其有效范围由文字和连线中范围大的决定
        if (xpre <= xcur) {
            xpre_eff = std::min(xpre, xcur - linkLabelItem->boundingRect().width());
        }
    }

    // 2024.02.22: determine the height of the link line
    double height = linkLineHeight(rs.get(), xpre_eff, xcur);

    double y = _railway.yValueFromCoeff(rs->y_coeff.value(), config()) + start_y;
    QPen pen = trainPen();
    pen.setColor(linkLineColor());
    pen.setWidth(LINK_LINE_WIDTH);
    pen.setStyle(linkLineStyle()); 

    if (xcur >= xpre) {
        //无需跨界
        if (xpre <= width) {
            linkItem1 = drawLinkLine(xpre, std::min(width, xcur), y, height, true, xcur <= width, true);
            //linkItem1 = new QGraphicsLineItem(
            //    config().totalLeftMargin() + xpre, y, 
            //    config().totalLeftMargin() + std::min(width, xcur), y, this
            //);
            linkItem1->setPen(pen);
        }
    }
    else {
        //跨界
        //左边的 一定存在
        //linkItem1 = new QGraphicsLineItem(
        //    config().totalLeftMargin(), y, config().totalLeftMargin() + xcur, y, this
        //);
        linkItem1 = drawLinkLine(0, xcur, y, height, false, true, true);
        linkItem1->setPen(pen);
        //右边的
        if (xpre <= width) {
            //linkItem2 = new QGraphicsLineItem(
            //    config().totalLeftMargin() + xpre, y, 
            //    config().totalLeftMargin() + width, y, this
            //);
            linkItem2 = drawLinkLine(xpre, width, y, height, true, false, false);
            linkItem2->setPen(pen);
        }
    }
    return true;
}

void TrainItem::clearLinkLines()
{
    DELETE_SUB(linkItem1);
    DELETE_SUB(linkItem2);
    DELETE_SUB(linkLabelItem);
    clearLinkInfo();
}

typename Qt::PenStyle TrainItem::linkLineStyle() const
{
    switch (config().train_name_mark_style)
    {
    case Config::TrainNameMarkStyle::Label: return Qt::DashLine;
    case Config::TrainNameMarkStyle::Link: return Qt::SolidLine;
    }
    return Qt::SolidLine;
}

QString TrainItem::linkLineLabelText(const QString& trainName, const Routing* rout) const
{
    //if (config().train_name_mark_style == Config::TrainNameMarkStyle::Label) {
    switch (config().link_line_label_type) {
    case Config::LinkLineLabelType::PostTrainName: return trainName; break;
    case Config::LinkLineLabelType::RoutingName: return rout->name(); break;
    }
    return {};
}

int TrainItem::linkLineLayer(const RailStation* rs, int xleft, int xright)const
{
    const double tot_width = config().fullWidth();
    auto& labels = dir() == Direction::Down ? _page.overLinks(rs) : _page.belowLinks(rs);

    return labels.addOccupation(RouteLinkOccupy(this->train().get(), xleft, xright), tot_width);
}

int TrainItem::linkLineLayerEnd(const RailStation* rs, int xleft, int xright) const
{
    const double tot_width = config().fullWidth();
    auto& labels = dir() == Direction::Down ? _page.belowLinks(rs) : _page.overLinks(rs);

    return labels.addOccupation(RouteLinkOccupy(this->train().get(), xleft, xright), tot_width);
}

double TrainItem::linkLineHeight(const RailStation* rs, int xleft, int xright)
{
    if (!config().floating_link_line) return 0;
    this->linkLayer.layer = linkLineLayer(rs, xleft, xright);
    return config().base_link_height + this->linkLayer.layer * config().step_link_height;
}

QColor TrainItem::linkLineColor() const
{
    switch (config().link_line_color) {
    case Config::LinkLineColorOption::LineColor: return trainColor();
    case Config::LinkLineColorOption::TextColor: return config().text_color_masked();
    }
    return trainColor();
}

QColor TrainItem::labelColor() const
{
    switch (config().train_label_color) {
    case Config::LinkLineColorOption::LineColor: return trainColor();
    case Config::LinkLineColorOption::TextColor: return config().text_color_masked();
    }
    return trainColor();
}

QGraphicsPathItem* TrainItem::drawLinkLine(double x1, double x2, double y, double height, 
    bool left_start, bool right_end, bool hasLabel)
{
    
    int height_sig = dir() == Direction::Down ? -1 : 1;
    double left_mag = config().totalLeftMargin();
    double yh = y + height_sig * height;

    // for a trial version, only the line itself!

    QEMultiLinePath path(QPointF(x1 + left_mag, y));
    if (left_start) {
        path.lineTo(x1 + left_mag, yh);
    }
    else {
        path.moveTo(x1 + left_mag, yh);
    }
    path.lineTo(x2 + left_mag, yh);
    if (right_end) {
        path.lineTo(x2 + left_mag, y);
    }

    // The stroker does not work with dash line
    //QPainterPathStroker stroker;
    //stroker.setWidth(0.5);
    //QPainterPath path_s = stroker.createStroke(path);

    if (hasLabel && linkLabelItem) {
        // In this case, the funciton is responsible for placing the label item
        const auto& rect = linkLabelItem->boundingRect();
        double label_y = dir() == Direction::Down ? yh - rect.height() : yh;
        linkLabelItem->setPos(left_mag + x2 - rect.width(), label_y);
    }

    return new QGraphicsPathItem(path.path(), this);
}

double TrainItem::timeDistancePbc(double x, const QTime& tm) const
{
    double tm_x = calXFromStart(tm) + start_x;
    double width = config().diagramWidth();
    while (tm_x - x > width) {
        x += width;
    }
    while (x - tm_x > width) {
        tm_x += width;
    }
    // now tm_x and x should be in the same image under PBC
    return x - tm_x;
}

void TrainItem::doDragSingle(const QTime& tm)
{
    switch (_dragPoint)
    {
    case StationPoint::Arrive:
        _draggedStation->trainStation->arrive = tm;
        break;
    case StationPoint::Depart:
        _draggedStation->trainStation->depart = tm;
        break;
    case StationPoint::Pass:
        _draggedStation->trainStation->arrive = tm;
        _draggedStation->trainStation->depart = tm;
        break;
    default:
        qDebug() << "TrainItem::doDrag: INVALID _dragPoint" << Qt::endl;
        break;
    }
}
