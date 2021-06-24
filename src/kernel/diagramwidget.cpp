#include "diagramwidget.h"
#include <QPainter>
#include <Qt>
#include <QGraphicsScene>
#include <QScrollbar>

DiagramWidget::DiagramWidget(Diagram &diagram, QWidget* parent):
    QGraphicsView(parent),_diagram(diagram)
{
    //todo: menu...
    setRenderHint(QPainter::Antialiasing, true);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);

    setScene(new QGraphicsScene);
    //todo: label span
    paintGraph();

    setMouseTracking(true);
}

void DiagramWidget::autoPaintGraph()
{
    if (_diagram.config().auto_paint)
        paintGraph();
}

void DiagramWidget::paintGraph()
{
    scene()->clear();
    _selectedTrain.reset();
    emit showNewStatus(QString("正在铺画运行图"));

    const Config& cfg = _diagram.config();
    const auto& margins = cfg.margins;
    int hstart = cfg.start_hour, hend = cfg.end_hour;
    if (hend <= hstart)
        hend += 24;
    int hour_count = hend - hstart;    //区间数量
    //width = hour_count * (3600 / UIDict["seconds_per_pix"])
    double width = hour_count * (3600.0 / cfg.seconds_per_pix);

    //暂定上下边距只算一次
    double height = (_diagram.railwayCount()-1) * cfg.margins.gap_between_railways;
    for (const auto& p : _diagram.railways()) {
        height += p->calStationYValue(cfg);
    }
    const QColor& gridColor = cfg.grid_color;
    
    scene()->setSceneRect(0, 0, width + cfg.margins.left + cfg.margins.right,
        height + cfg.margins.up + cfg.margins.down);

    double ystart = margins.up;

    for (auto p : _diagram.railways()) {
        setHLines(p, ystart, width);
        setVLines(p, ystart, width, hour_count);
        scene()->addRect(margins.left, ystart, width, p->diagramHeight(), gridColor);
        ystart += p->diagramHeight() + margins.gap_between_railways;
    }

    //todo: labelSpan
    
    //todo: 绘制提示进度条
    for (auto p : _diagram.trainCollection().trains()) {
        paintTrain(p);
    }

    //todo: 天窗显示
    
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
        this, SLOT(updateTimeAxis()));
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
        this, SLOT(updateDistanceAxis()));

}

void DiagramWidget::setHLines(std::shared_ptr<Railway> rail, double start_y, double width)
{
    QList<QGraphicsItem*> leftItems, rightItems;

    const Config& cfg = _diagram.config();
    const auto& margins = cfg.margins;
    const QColor& textColor = cfg.text_color;
    QColor brushColor(Qt::white);
    brushColor.setAlpha(200);
    double label_start_x = margins.mile_label_width + margins.ruler_label_width;

    double height = rail->diagramHeight();   //注意这只是当前线路的高度
    double totheight = height + margins.up + margins.down;

    auto* rectLeft = scene()->addRect(0,
        start_y - margins.title_row_height - margins.first_row_append,
        margins.label_width + margins.ruler_label_width + margins.mile_label_width + margins.left_white,
        height + 2 * margins.first_row_append + margins.title_row_height
    );
    rectLeft->setBrush(QBrush(brushColor));
    rectLeft->setPen(QPen(Qt::transparent));
    rectLeft->setZValue(-1);
    leftItems.append(rectLeft);

    auto* rectRight = scene()->addRect(
        scene()->width() - margins.label_width - margins.right_white,
        margins.up - margins.title_row_height - margins.first_row_append,
        margins.label_width,
        height + 2 * margins.first_row_append + margins.title_row_height
    );
    rectRight->setBrush(brushColor);
    rectRight->setPen(QPen(Qt::transparent));
    rightItems.append(rectRight);

    const QColor& gridColor = cfg.grid_color;
    QPen defaultPen(gridColor, cfg.default_grid_width),
        boldPen(gridColor, cfg.bold_grid_width);

    double rect_start_y = start_y - margins.title_row_height - margins.first_row_append;
    auto* rulerRect = scene()->addRect(
        margins.left_white, rect_start_y,
        margins.mile_label_width + margins.ruler_label_width,
        height + margins.title_row_height + margins.first_row_append*2,
        defaultPen
    );
    rulerRect->setPen(defaultPen);
    leftItems.append(rulerRect);

    //标尺栏纵向界限
    auto* line = scene()->addLine(
        margins.ruler_label_width + margins.left_white,
        rect_start_y,
        margins.ruler_label_width + margins.left_white,
        height + start_y + margins.first_row_append,
        defaultPen
    );
    leftItems.append(line);

    //标尺栏横向界限
    line = scene()->addLine(
        margins.left_white,
        rect_start_y + margins.title_row_height / 2,
        margins.ruler_label_width + margins.left_white,
        rect_start_y + margins.title_row_height / 2,
        defaultPen
    );
    leftItems.append(line);

    //表头下横分界线
    line = scene()->addLine(
        margins.left_white,
        rect_start_y + margins.title_row_height,
        margins.ruler_label_width + margins.mile_label_width + margins.left_white,
        rect_start_y + margins.title_row_height,
        defaultPen
    );
    leftItems.append(line);

    nowItem = scene()->addSimpleText(" ");
    nowItem->setBrush(textColor);
    nowItem->setZValue(16);

    QFont textFont;
    //textFont.setBold(true);

    const char* s0 = rail->ordinate() ? "排图标尺" : "区间距离";
    leftItems.append(addLeftTableText(
        s0, textFont, 0, rect_start_y, margins.ruler_label_width, margins.title_row_height / 2
    ));

    leftItems.append(addLeftTableText(
        "下行", textFont, 0, rect_start_y + margins.title_row_height / 2.0,
        margins.ruler_label_width / 2.0, margins.title_row_height / 2.0
    ));

    leftItems.append(addLeftTableText(
        "上行", textFont, margins.ruler_label_width / 2.0,
        rect_start_y + margins.title_row_height / 2.0, margins.ruler_label_width / 2.0,
        margins.title_row_height / 2.0
    ));

    leftItems.append(addLeftTableText(
        "延长公里", textFont, margins.ruler_label_width, rect_start_y,
        margins.mile_label_width, margins.title_row_height
    ));

    //改为无条件上下行分开标注
    leftItems.append(scene()->addLine(
        margins.ruler_label_width / 2.0 + margins.left_white,
        rect_start_y + margins.title_row_height / 2.0,
        margins.ruler_label_width / 2.0 + margins.left_white,
        start_y + height + margins.first_row_append,
        defaultPen
    ));
    

    //铺画车站。以前已经完成了绑定，这里只需要简单地把所有有y坐标的全画出来就好
    for (auto p : rail->stations()) {
        if (p->y_value.has_value() && p->show) {
            const auto& pen = p->level <= cfg.bold_line_level ?
                boldPen : defaultPen;
            double h = start_y + p->y_value.value();
            drawSingleHLine(textFont, h, p->name.toDisplayLiteral(),
                pen, width, leftItems, rightItems, label_start_x);
            leftItems.append(addStationTableText(
                QString::number(p->mile, 'f', 1), textFont,
                margins.ruler_label_width, h, margins.mile_label_width
            ));
        }
    }

    auto ruler = rail->ordinate();
    auto upFirst = rail->firstUpInterval();

    //cum: cummulative 和前面的累计
    //必须这样做的原因是可能有车站没有显示，这时也不划线
    double lasty = start_y, cummile = 0.0;
    int cuminterval = 0;
    bool cumvalid = true;

    //标注区间数据  每个区间标注带上区间【终点】的界限
    for (auto p = rail->firstDownInterval(); p; p = rail->nextIntervalCirc(p)) {
        //标注区间终点分划线
        double x = margins.left_white;
        if (!p->isDown()) x += margins.ruler_label_width / 2.0;
        double y = p->toStation()->y_value.value() + start_y;
        if (p->toStation()->show)
            leftItems.append(scene()->addLine(
                x, y, x + margins.ruler_label_width / 2.0, y, defaultPen
            ));
        //标注数据
        if (p == upFirst) {
            //方向转换，清理旧数据
            lasty = start_y + height;
            cummile = 0.0;
            cuminterval = 0;
        }
        if (ruler) {
            
            if (p->getRulerNode(ruler)->isNull()) {
                if (p->direction() == Direction::Up && !ruler->different())
                    cuminterval += p->inverseInterval()->getRulerNode(ruler)->interval;
                else
                    cumvalid = false;
            }
            else {
                cuminterval += p->getRulerNode(ruler)->interval;
            }
                
        }
        else {
            cummile += p->mile();
        }
        if (p->toStation()->show) {
            const QString& text = ruler ?
                cumvalid ? QString::asprintf("%d:%02d", cuminterval / 60, cuminterval % 60) : "NA" :
                QString::number(cummile, 'f', 1);
            leftItems.append(alignedTextItem(
                text, textFont,
                margins.ruler_label_width / 2.0, x,
                (lasty + y) / 2, false
            ));
            lasty = y;
            cummile = 0.0;
            cuminterval = 0;
            cumvalid = true;
        }
        
    }
    //补充起点的下行和终点的上行边界
    leftItems.append(scene()->addLine(
        margins.left_white, start_y, margins.left_white + margins.ruler_label_width / 2.0, start_y,
        defaultPen
    ));
    leftItems.append(scene()->addLine(
        margins.left_white + margins.ruler_label_width / 2.0,
        start_y + height, margins.left_white + margins.ruler_label_width, start_y + height,defaultPen
    ));

    marginItems.left = scene()->createItemGroup(leftItems);
    marginItems.left->setZValue(15);
    marginItems.right = scene()->createItemGroup(rightItems);
    marginItems.right->setZValue(15);
}

void DiagramWidget::setVLines(std::shared_ptr<Railway> rail, double start_y, double width, int hour_count)
{
    //todo...
}




void DiagramWidget::paintTrain(std::shared_ptr<Train> train)
{
    //todo...
}

QGraphicsSimpleTextItem* DiagramWidget::addLeftTableText(const char* str, 
    const QFont& textFont, double start_x, double start_y, double width, double height)
{
    return addLeftTableText(QObject::tr(str), textFont, start_x, start_y, width, height);
}

QGraphicsSimpleTextItem* DiagramWidget::addLeftTableText(const QString& str, 
    const QFont& textFont, double start_x, double start_y, double width, double height)
{
    const QColor& textColor = _diagram.config().text_color;
    const auto& margins = _diagram.config().margins;
    start_x += margins.left_white;

    auto* text = scene()->addSimpleText(str);
    QFont font(textFont);   //copy
    double width1 = text->boundingRect().width();
    if (width1 > width) {
        int stretch = 100 * width / width1;   //int div
        font.setStretch(stretch);
    }
    text->setFont(font);
    text->setBrush(textColor);
    const auto& r = text->boundingRect();
    text->setX(start_x + (width - r.width()) / 2);
    text->setY(start_y + (height - r.height()) / 2);
    return text;
}

QGraphicsSimpleTextItem* DiagramWidget::addStationTableText(const QString& str,
    const QFont& textFont, double start_x, double center_y, double width)
{
    const QColor& textColor = _diagram.config().text_color;
    const auto& margins = _diagram.config().margins;
    start_x += margins.left_white;

    auto* text = scene()->addSimpleText(str);
    QFont font(textFont);   //copy
    double width1 = text->boundingRect().width();
    if (width1 > width) {
        int stretch = 100 * width / width1;   //int div
        font.setStretch(stretch);
    }
    text->setFont(font);
    text->setBrush(textColor);
    const auto& r = text->boundingRect();
    text->setX(start_x + (width - r.width()) / 2);
    text->setY(center_y-r.height()/2);
    return text;
}

void DiagramWidget::drawSingleHLine(const QFont& textFont, double y, 
    const QString& name, const QPen& pen, double width, 
    QList<QGraphicsItem*>& leftItems, 
    QList<QGraphicsItem*>& rightItems, double label_start_x)
{
    scene()->addLine(margins().left, y, width + margins().left, y, pen);

    leftItems.append(alignedTextItem(name, textFont, margins().label_width - 5,
        label_start_x + margins().left_white + 5, y));
    
    rightItems.append(alignedTextItem(name, textFont, margins().label_width,
        scene()->width() - margins().label_width - margins().right_white, y));
}

QGraphicsSimpleTextItem* DiagramWidget::alignedTextItem(const QString& text, 
    const QFont& baseFont, double label_width, double start_x, double center_y, bool use_stretch)
{
    auto* textItem = scene()->addSimpleText(text, baseFont);
    textItem->setBrush(config().text_color);
    textItem->setY(center_y - textItem->boundingRect().height() / 2);
    double textWidth = textItem->boundingRect().width();
    QFont font(baseFont);
    if (textWidth > label_width) {
        int stretch = 100 * label_width / textWidth;
        font.setStretch(stretch);
        textItem->setFont(font);
    }
    textWidth = textItem->boundingRect().width();

    int cnt = text.size();
    if (use_stretch ) {
        if (textWidth < label_width && cnt > 1) {
            //两端对齐
            font.setLetterSpacing(QFont::AbsoluteSpacing, (label_width - textWidth) / (cnt - 1));
            textItem->setFont(font);
        }
        textItem->setX(start_x);
    }
    else {
        textItem->setX(start_x + (label_width - textWidth) / 2);
    }
    
    return textItem;
}


void DiagramWidget::updateTimeAxis()
{
    //todo...
}

void DiagramWidget::updateDistanceAxis()
{
    QPoint point(0, 0);
    auto scenepoint = mapToScene(point);
    marginItems.left->setX(scenepoint.x());
    nowItem->setX(scenepoint.x());
    QPoint p2(width(), 0);
    auto sp2 = mapToScene(p2);
    marginItems.right->setX(sp2.x() - scene()->width() - 20);
    /*
        point = QtCore.QPoint(0, 0)
        scenepoint = self.mapToScene(point)
        self.marginItemGroups["left"].setX(scenepoint.x())
        try:
            self.nowItem.setX(scenepoint.x())
            point = QtCore.QPoint(self.width(), 0)
            scenepoint = self.mapToScene(point)
            self.marginItemGroups["right"].setX(scenepoint.x() - self.scene.width() - 20)
        except RuntimeError:
            pass
    */
}
