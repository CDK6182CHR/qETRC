#include "diagramwidget.h"
#include "data/diagram/trainadapter.h"
#include "data/train/routing.h"
#include "trainitem.h"
#include "util/utilfunc.h"
#include <QPainter>
#include <Qt>
#include <QGraphicsScene>
#include <QScrollBar>
#include <QList>
#include <QPair>
#include <QMouseEvent>
#include <cmath>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QScroller>
#include <QMenu>

#if defined(QT_PRINTSUPPORT_LIB)
#include <QPrinter>
#endif

#include "data/diagram/diagram.h"
#include "data/common/qesystem.h"
#include "mainwindow/version.h"
#include "util/qeballoomtip.h"
#include "data/rail/rulernode.h"
#include "data/rail/forbid.h"

DiagramWidget::DiagramWidget(Diagram& diagram, std::shared_ptr<DiagramPage> page, QWidget* parent):
    QGraphicsView(parent), _page(page),_diagram(diagram),startTime(page->config().start_hour,0,0)
{
    QScroller::grabGesture(this, QScroller::TouchGesture);
    setRenderHint(QPainter::Antialiasing, true);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);

    setScene(new QGraphicsScene(this));
    paintGraph();

    setMouseTracking(true);
    setAttribute(Qt::WA_AcceptTouchEvents);
}

DiagramWidget::~DiagramWidget() noexcept
{
    //把这个放前面，是为了避免在TrainItem集体析构时，再次触发删除LabelInfo，引发异常
    //通过这里大量析构Item，通常是运行图数据发生不可控的剧烈变化，
    //此时TrainItem对数据的引用通常已经进入很危险的状态，应避免使用
    _page->clearGraphics();
    scene()->clear();   //据说这样快一些？
}

void DiagramWidget::autoPaintGraph()
{
    if (config().auto_paint)
        paintGraph();
}

void DiagramWidget::paintGraph()
{
    auto clock_start = std::chrono::system_clock::now();
    updating = true;
    clearGraph();
    // 2022.02.07：更改页面设置后，这个可能会变
    startTime = QTime(config().start_hour, 0, 0);
    _selectedTrain = nullptr;
    emit showNewStatus(QString("正在铺画运行图"));

    const Config& cfg = config();
    const auto& margins = cfg.margins;
    int hstart = cfg.start_hour, hend = cfg.end_hour;
    if (hend <= hstart)
        hend += 24;
    int hour_count = hend - hstart;    //区间数量
    //width = hour_count * (3600 / UIDict["seconds_per_pix"])
    double width = hour_count * (3600.0 / cfg.seconds_per_pix);

    //暂定上下边距只算一次
    double height = (_page->railwayCount()-1) * cfg.margins.gap_between_railways;
    foreach (const auto& p , _page->railways()) {
        //2022.09.22: This should be valid throughout the lives of Railway
        //p->calStationYCoeff();
        height += p->diagramHeight(cfg);
    }
    const QColor& gridColor = cfg.grid_color;
    
    scene()->setSceneRect(0, 0, width + cfg.totalLeftMargin() + cfg.totalRightMargin(),
        height + cfg.margins.up + cfg.margins.down);

    double ystart = margins.up;

    QList<QPair<double, double>> railYRanges;   //每条线路的时间线纵坐标起止点
    QList<QGraphicsItem*> leftItems, rightItems;

    _page->startYs().clear();
    for (int i = 0;i<_page->railwayCount();i++) {
        auto p = _page->railways().at(i);
        _page->startYs().append(ystart);
        setHLines(p, ystart, width, leftItems, rightItems);
        scene()->addRect(cfg.totalLeftMargin(), ystart, width, p->diagramHeight(cfg), gridColor);
        railYRanges.append(qMakePair(ystart, ystart + p->diagramHeight(cfg)));
        ystart += p->diagramHeight(cfg) + margins.gap_between_railways;
    }

    setVLines(width, hour_count, railYRanges);

    //foreach(auto it, leftItems) {
    //    auto r = it->boundingRect();
    //    auto itt = scene()->addRect(r);
    //    itt->setPos(it->pos());
    //}

    marginItems.left = scene()->createItemGroup(leftItems);
    marginItems.left->setZValue(15);
    marginItems.right = scene()->createItemGroup(rightItems);
    marginItems.right->setZValue(15);
    
    //todo: 绘制提示进度条
    for (auto p : _diagram.trainCollection().trains()) {
        paintTrain(p);
    }

    showAllForbids();
    
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
        this, SLOT(updateTimeAxis()));
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
        this, SLOT(updateDistanceAxis()));

    updating = false;

    updateTimeAxis();
    updateDistanceAxis();
    auto clock_end = std::chrono::system_clock::now();
    emit showNewStatus(QObject::tr("运行图 [%1] 铺画完毕  用时%2毫秒").arg(_page->name())
        .arg((clock_end - clock_start) / std::chrono::milliseconds(1)));
}

void DiagramWidget::clearGraph()
{
    weakItem = nullptr;
    _page->clearGraphics();
    scene()->clear();
}

bool DiagramWidget::toPdf(const QString& filename, const QString& title, const QString& note)
{
#if ! defined(QT_PRINTSUPPORT_LIB)
    Q_UNUSED(filename);
    Q_UNUSED(title);
    Q_UNUSED(note)
    QMessageBox::warning(this,tr("错误"),tr("由于当前平台不支持QtPrintSupport, "
        "无法使用导出PDF功能。请考虑使用导出PNG功能。"));
    return false;
#else
    using namespace std::chrono_literals;
    auto start = std::chrono::system_clock::now();
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename);
    constexpr double note_apdx = 80;

    QSize size(scene()->width(), scene()->height() + 100 + note_apdx);
    QPageSize pageSize(size);
    printer.setPageSize(pageSize);

    QPainter painter;
    painter.begin(&printer);

    if (!painter.isActive()) {
        QMessageBox::warning(this, QObject::tr("错误"), QObject::tr("保存PDF失败，可能是文件占用。"));
        return false;
    }
    painter.scale(printer.width() / scene()->width(), printer.width() / scene()->width());

    paintToFile(painter, title, note);
    auto end = std::chrono::system_clock::now();
    emit showNewStatus(tr("导出PDF  用时%1毫秒").arg((end - start) / 1ms));
    return true;
#endif
}

void DiagramWidget::paintToFile(QPainter& painter, const QString& title, const QString& note)
{
    marginItems.left->setX(0);
    marginItems.right->setX(0);
    marginItems.top->setY(0);
    marginItems.bottom->setY(0);
    nowItem->setPos(0, 0);
    painter.setPen(QPen(config().text_color));
    QFont font;
    font.setPixelSize(40);
    font.setBold(true);
    painter.setFont(font);

    painter.drawText(config().totalLeftMargin(), 80, title);

    font.setPixelSize(20);
    font.setBold(false);
    painter.setFont(font);

    if (!note.isEmpty()) {
        QString s(note);
        s.replace("\n", " ");
        s = QString("备注：") + s;
        painter.drawText(config().totalLeftMargin(), scene()->height() + 100 + 40, s);
    }

    QString mark = tr("由 %1_%2 导出").arg(qespec::TITLE.data()).arg(qespec::VERSION.data());
    painter.drawText(scene()->width() - 400, scene()->height() + 100 + 40, mark);
    painter.setRenderHint(QPainter::Antialiasing);
    scene()->render(&painter, QRectF(0, 100, scene()->width(), scene()->height()));
    painter.end();

    updateDistanceAxis();
    updateTimeAxis();
}

void DiagramWidget::showWeakenItem()
{
    if (!SystemJson::instance.weaken_unselected)
        return;
    if (!weakItem) {
        //weakItem = new QGraphicsRectItem(margins().left, margins().up,
        //    scene()->width() - margins().left - margins().right,
        //    scene()->height() - margins().up - margins().down
        //);
        weakItem = new QGraphicsRectItem(0, 0, scene()->width(), scene()->height());
        scene()->addItem(weakItem);
        weakItem->setBrush(QColor(255, 255, 255, 150));
        weakItem->setZValue(9);
        weakItem->setPen(QPen(Qt::transparent));
    }
    else {
        weakItem->setVisible(true);
    }
}

void DiagramWidget::hideWeakenItem()
{
    if (weakItem)
        weakItem->setVisible(false);
}

#include <QGraphicsProxyWidget>

void DiagramWidget::showPosTip(const QPoint& pos, const QString& msg, const QString& title)
{
    if (!posTip) {
        posTip = new qeutil::QEBalloonTip({}, title, msg, this);
        //pw = scene()->addWidget(posTip, Qt::ToolTip);
    }
    else {
        posTip->setContents(title, msg);
    }
    posTip->balloon(pos, 10000, true);
}

bool DiagramWidget::toPng(const QString& filename, const QString& title, const QString& note)
{
    using namespace std::chrono_literals;
    auto start = std::chrono::system_clock::now();
    constexpr int note_apdx = 80;
    QImage image(scene()->width(), scene()->height() + 100 + note_apdx,
        QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter;
    painter.begin(&image);
    paintToFile(painter, title, note);
    bool flag= image.save(filename);
    if (flag) {
        auto end = std::chrono::system_clock::now();
        showNewStatus(tr("导出PNG  用时%1毫秒").arg((end - start) / 1ms));
    }
    return flag;
}

void DiagramWidget::removeTrain(const Train& train)
{
    for (auto adp : train.adapters()) {
        for (auto p : adp->lines()) {
            auto* item = _page->takeTrainItem(p.get());
            if (item) {
                scene()->removeItem(item);
                delete item;
            }
        }
    }
    if (&train == _selectedTrain.get())
        _selectedTrain.reset();
}

void DiagramWidget::removeTrain(QVector<std::shared_ptr<TrainAdapter>>&& adps)
{
    for (auto adp : adps) {
        for (auto p : adp->lines()) {
            auto* item = _page->takeTrainItem(p.get());
            if (item) {
                scene()->removeItem(item);
                delete item;
            }
        }
    }
}

void DiagramWidget::updateTrain(std::shared_ptr<Train> train, 
    QVector<std::shared_ptr<TrainAdapter>>&& adps)
{
    removeTrain(std::move(adps));
    paintTrain(train);
    if (_selectedTrain == train)
        highlightTrain(train);
}

void DiagramWidget::repaintTrain(std::shared_ptr<Train> train)
{
    bool isSel = (train == _selectedTrain);
    removeTrain(*train);
    paintTrain(train);
    if (isSel) {
        _selectedTrain = train;
        highlightTrain(train);
    }
}

void DiagramWidget::setTrainShow(std::shared_ptr<Train> train, bool show)
{
    for (auto adp : train->adapters())
        setTrainShow(adp, show);
}

void DiagramWidget::setTrainShow(std::shared_ptr<TrainAdapter> adp, bool show)
{
    if (!_page->containsRailway(adp->railway())) {
        return;
    }
    for (auto line : adp->lines()) {
        setTrainShow(line, show);
    }
}

void DiagramWidget::setTrainShow(std::shared_ptr<TrainLine> line, bool show)
{
    if (!_page->containsRailway(line->adapter().railway())) {
        return;
    }
    line->setIsShow(show);   //安全起见，保证同步
    if (show) {
        //显示
        auto* item = _page->getTrainItem(line.get());
        if (item) {
            item->setVisible(true);
        }
        else {
            paintTrainLine(line);
        }
            
    }
    else {
        auto* item = _page->getTrainItem(line.get());
        if (item)
            item->setVisible(false);
    }
}

void DiagramWidget::setupMenu(const SharedActions& actions)
{
    if (!contextMenu) {
        contextMenu = new QMenu(this);
        auto* m = contextMenu;
        auto* act = new QAction(tr("重新铺画当前运行图"));
        act->setShortcut(Qt::SHIFT + Qt::Key_F5);
        act->setShortcutContext(Qt::WidgetShortcut);
        connect(act, &QAction::triggered, this, &DiagramWidget::paintGraph);
        addAction(act);
        contextMenu->addAction(act);

        act = new QAction(tr("放大视图"));
        act->setShortcut(Qt::CTRL + Qt::Key_Equal);
        act->setShortcutContext(Qt::WidgetShortcut);
        addAction(act);
        m->addAction(act);
        connect(act, &QAction::triggered, this, &DiagramWidget::zoomIn);

        act = new QAction(tr("缩小视图"));
        act->setShortcut(Qt::CTRL + Qt::Key_Minus);
        act->setShortcutContext(Qt::WidgetShortcut);
        addAction(act);
        m->addAction(act);
        connect(act, &QAction::triggered, this, &DiagramWidget::zoomOut);

        m->addSeparator();
        m->addAction(actions.refreshAll);
        m->addSeparator();

        m->addAction(actions.search);
        m->addAction(actions.rulerRef);
        m->addAction(actions.trainRef);
        m->addAction(actions.eventList);
        m->addSeparator();
        m->addAction(actions.timeAdjust);
        m->addAction(actions.batchCopy);
        m->addAction(actions.intervalExchange);
        m->addSeparator();
        m->addAction(actions.addTrain);
        m->addAction(actions.rulerPaint);
        m->addAction(actions.greedyPaint);
    }
}

void DiagramWidget::mousePressEvent(QMouseEvent* e)
{
    QGraphicsView::mousePressEvent(e);
    if (updating)
        return;
    if (e->button() == Qt::LeftButton) {
        auto pos = mapToScene(e->pos());
        unselectTrain();

        auto* item = posTrainItem(pos);
        if (item) {
            selectTrain(item);
        }
    }
}

void DiagramWidget::mouseMoveEvent(QMouseEvent* e)
{
    QGraphicsView::mouseMoveEvent(e);
    if (updating)
        return;
    if (!SystemJson::instance.show_train_tooltip)
        return;
    auto pos = mapToScene(e->pos());
    TrainItem* item = posTrainItem(pos);
    if (!item || item->train() != _selectedTrain) {
        setToolTip("");
        return;
    }

    auto line = item->trainLine();
    auto rail = line->railway();

    double y = pos.y() - item->getStartY();  // 绝对坐标
    auto itr = line->stationFromYCoeff(line->railway()->yCoeffFromAbsValue(y, config()));
    const auto& dq = line->stations();
    if (itr == dq.end())
        return;
    static constexpr double STATION_SPAN = 5;
    //第一种情况：属于站内
    if (std::abs(rail->yValueFromCoeff(itr->yCoeff(), config()) - y) <= STATION_SPAN) {
        stationToolTip(itr, *line);
        return;
    }
    //否则：找区间的另一个站
    if (itr == dq.begin())
        return;
    auto prev = itr; --prev;    //区间前一个站
    if (std::abs(rail->yValueFromCoeff(prev->yCoeff(), config()) - y) <= STATION_SPAN) {
        stationToolTip(prev, *line);
        return;
    }
    //到现在：只能是区间了
    intervalToolTip(prev, itr, *line);
}


void DiagramWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    QGraphicsView::mouseDoubleClickEvent(e);
}

void DiagramWidget::resizeEvent(QResizeEvent* e)
{
    QGraphicsView::resizeEvent(e);
    if (!updating) {
        updateDistanceAxis();
        updateTimeAxis();
    }
}

bool DiagramWidget::event(QEvent* e)
{
    bool flag = QGraphicsView::event(e);
    if (e->type() == QEvent::WindowActivate || e->type() == QEvent::FocusIn) {
        emit pageFocussedIn(_page);
        return true;
    }
    return flag;
}

void DiagramWidget::contextMenuEvent(QContextMenuEvent* e)
{
    if (contextMenu) {
        contextMenu->popup(mapToGlobal(e->pos()));
    }
}

void DiagramWidget::setHLines(std::shared_ptr<Railway> rail, double start_y, double width,
    QList<QGraphicsItem*>& leftItems, QList<QGraphicsItem*>& rightItems)
{
    const Config& cfg = config();
    const auto& margins = cfg.margins;
    const QColor& textColor = cfg.text_color;
    QColor brushColor(Qt::white);
    brushColor.setAlpha(200);
    double label_start_x = cfg.leftStationBarX();

    double height = rail->diagramHeight(cfg);   //注意这只是当前线路的高度

    auto* rectLeft = scene()->addRect(0,
        start_y - margins.title_row_height - margins.first_row_append,
        cfg.leftRectWidth() + margins.left_white,
        height + 2 * margins.first_row_append + margins.title_row_height
    );
    rectLeft->setBrush(QBrush(brushColor));
    rectLeft->setPen(QPen(Qt::transparent));
    rectLeft->setZValue(-1);
    leftItems.append(rectLeft);

    auto* rectRight = scene()->addRect(
        scene()->width() - cfg.rightRectWidth() - margins.right_white,
        start_y - margins.title_row_height - margins.first_row_append,
        cfg.rightRectWidth(),
        height + 2 * margins.first_row_append + margins.title_row_height
    );
    rectRight->setBrush(brushColor);
    rectRight->setPen(QPen(Qt::transparent));
    rightItems.append(rectRight);

    const QColor& gridColor = cfg.grid_color;
    QPen defaultPen(gridColor, cfg.default_grid_width),
        boldPen(gridColor, cfg.bold_grid_width);

    double rect_start_y = start_y - margins.title_row_height - margins.first_row_append;


    QFont nowFont;
    nowFont.setPointSize(12);
    nowItem = scene()->addSimpleText(" ", nowFont);
    nowItem->setBrush(textColor);
    nowItem->setZValue(16);

    QFont textFont;
    //textFont.setBold(true);

    if (cfg.show_ruler_bar || cfg.show_mile_bar) {
        auto* rulerRect = scene()->addRect(
            margins.left_white, rect_start_y,
            cfg.leftTitleRectWidth(),
            height + margins.title_row_height + margins.first_row_append * 2,
            defaultPen
        );
        leftItems.append(rulerRect);

        //表头下横分界线
        leftItems.append(scene()->addLine(
            margins.left_white,
            rect_start_y + margins.title_row_height,
            cfg.leftTitleRectWidth() + margins.left_white,
            rect_start_y + margins.title_row_height,
            defaultPen
        ));

        if (cfg.show_mile_bar && cfg.show_ruler_bar) {
            //标尺栏纵向界限
            auto* line = scene()->addLine(
                margins.ruler_label_width + margins.left_white,
                rect_start_y,
                margins.ruler_label_width + margins.left_white,
                height + start_y + margins.first_row_append,
                defaultPen
            );
            leftItems.append(line);
        }

        if (cfg.show_ruler_bar) {

            //标尺栏横向界限
            leftItems.append(scene()->addLine(
                margins.left_white,
                rect_start_y + margins.title_row_height / 2.0,
                margins.ruler_label_width + margins.left_white,
                rect_start_y + margins.title_row_height / 2.0,
                defaultPen
            ));

            const char* s0 = rail->ordinate() ? "排图标尺" : "区间距离";
            leftItems.append(addLeftTableText(
                s0, textFont, margins.left_white, rect_start_y, margins.ruler_label_width, margins.title_row_height / 2.0
            ));

            leftItems.append(addLeftTableText(
                "下行", textFont, margins.left_white, rect_start_y + margins.title_row_height / 2.0,
                margins.ruler_label_width / 2.0, margins.title_row_height / 2.0
            ));

            leftItems.append(addLeftTableText(
                "上行", textFont, margins.left_white+ margins.ruler_label_width / 2.0,
                rect_start_y + margins.title_row_height / 2.0, margins.ruler_label_width / 2.0,
                margins.title_row_height / 2.0
            ));

            //改为无条件上下行分开标注
            leftItems.append(scene()->addLine(
                margins.ruler_label_width / 2.0 + margins.left_white,
                rect_start_y + margins.title_row_height / 2.0,
                margins.ruler_label_width / 2.0 + margins.left_white,
                start_y + height + margins.first_row_append,
                defaultPen
            ));
        }

        if (cfg.show_mile_bar) {
            leftItems.append(addLeftTableText(
                "延长公里", textFont, cfg.mileBarX(), rect_start_y,
                margins.mile_label_width, margins.title_row_height
            ));
        }

    }

    if (cfg.show_count_bar) {
        auto* countRect = scene()->addRect(
            scene()->width() - margins.right_white - margins.count_label_width,
            rect_start_y,
            margins.count_label_width,
            height + margins.title_row_height + margins.first_row_append * 2,
            defaultPen
        );
        rightItems.append(countRect);

        // 右侧
        rightItems.append(scene()->addLine(
            scene()->width() - margins.count_label_width - margins.right_white,
            rect_start_y + margins.title_row_height / 2.0,
            scene()->width() - margins.right_white,
            rect_start_y + margins.title_row_height / 2.0,
            defaultPen
        ));

        rightItems.append(scene()->addLine(
            scene()->width() - margins.right_white - margins.count_label_width,
            rect_start_y + margins.title_row_height,
            scene()->width() - margins.right_white,
            rect_start_y + margins.title_row_height,
            defaultPen
        ));

        rightItems.append(scene()->addLine(
            scene()->width() - margins.right_white - margins.count_label_width / 2.0,
            rect_start_y + margins.title_row_height / 2.0,
            scene()->width() - margins.right_white - margins.count_label_width / 2.0,
            start_y + height + margins.first_row_append,
            defaultPen
        ));

        rightItems.append(addLeftTableText(tr("客货对数"),
            textFont, scene()->width() - margins.right_white - margins.count_label_width,
            rect_start_y, margins.count_label_width,
            margins.title_row_height / 2.0
        ));

        rightItems.append(addLeftTableText(tr("下行"),
            textFont, scene()->width() - margins.right_white - margins.count_label_width,
            rect_start_y + margins.title_row_height / 2.0,
            margins.count_label_width / 2.0, margins.title_row_height / 2.0
        ));

        rightItems.append(addLeftTableText(tr("上行"),
            textFont, scene()->width() - margins.right_white - margins.count_label_width / 2.0,
            rect_start_y + margins.title_row_height / 2.0,
            margins.count_label_width / 2.0, margins.title_row_height / 2.0
        ));

    }
    

    //铺画车站。以前已经完成了绑定，这里只需要简单地把所有有y坐标的全画出来就好
    for (auto p : rail->stations()) {
        if (p->y_coeff.has_value() && p->_show) {
            const auto& pen = p->level <= cfg.bold_line_level ?
                boldPen : defaultPen;
            double h = start_y + rail->yValueFromCoeff(p->y_coeff.value(), cfg);
            drawSingleHLine(textFont, h, p->name.toDisplayLiteral(),
                pen, width, leftItems, rightItems, label_start_x);
            //延长公里
            if (cfg.show_mile_bar) {
                leftItems.append(addStationTableText(
                    QString::number(p->mile, 'f', 1), textFont,
                    cfg.mileBarX() , h, margins.mile_label_width
                ));
            }

        }
    }

    if (!cfg.show_ruler_bar && !cfg.show_count_bar)
        return;

    auto ruler = rail->ordinate();
    auto upFirst = rail->firstUpInterval();

    //cum: cummulative 和前面的累计
    //必须这样做的原因是可能有车站没有显示，这时也不划线
    double lasty = start_y, cummile = 0.0;
    int cuminterval = 0;
    bool cumvalid = true;
    int maxpassen = 0, maxfreigh = 0;

    // 区间对数表
    Diagram::sec_cnt_t passencnt, freighcnt;
    if (cfg.show_count_bar) {
        auto&& p = _diagram.sectionPassenFreighCount(rail);
        passencnt = std::forward<Diagram::sec_cnt_t>(p.first);
        freighcnt = std::forward<Diagram::sec_cnt_t>(p.second);
    }

    //标注区间数据  每个区间标注带上区间【终点】的界限
    for (auto p = rail->firstDownInterval(); p; p = rail->nextIntervalCirc(p)) {
        //标注区间终点分划线
        double x = margins.left_white;
        if (!p->isDown()) x += margins.ruler_label_width / 2.0;
        double xright = scene()->width() - margins.right_white - margins.count_label_width;
        if (!p->isDown())xright += margins.count_label_width / 2.0;
        double y = rail->yValueFromCoeff(p->toStation()->y_coeff.value(), cfg) + start_y;
        if (p->toStation()->_show) {
            if (cfg.show_ruler_bar)
                leftItems.append(scene()->addLine(
                    x, y, x + margins.ruler_label_width / 2.0, y, defaultPen
                ));
            if (cfg.show_count_bar)
                rightItems.append(scene()->addLine(
                    xright, y, xright + margins.count_label_width / 2.0, y, defaultPen
                ));
        }
            
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
        if (auto itr = passencnt.find(p); itr != passencnt.end()) {
            maxpassen = std::max(itr->second, maxpassen);
        }
        if (auto itr = freighcnt.find(p); itr != freighcnt.end()) {
            maxfreigh = std::max(itr->second, maxfreigh);
        }
        if (p->toStation()->_show) {
            if (cfg.show_ruler_bar) {
                const QString& text = ruler ?
                    cumvalid ? QString::asprintf("%d:%02d", cuminterval / 60, cuminterval % 60) : "NA" :
                    QString::number(cummile, 'f', 1);
                leftItems.append(alignedTextItem(
                    text, textFont,
                    margins.ruler_label_width / 2.0, x,
                    (lasty + y) / 2, false, 0.9
                ));
            }
            if (cfg.show_count_bar) {
                rightItems.append(alignedTextItem(
                    tr("%1/%2").arg(maxpassen).arg(maxfreigh), textFont,
                    margins.count_label_width / 2.0, xright,
                    (lasty + y) / 2, false, 0.9
                ));
            }

            lasty = y;
            cummile = 0.0;
            cuminterval = 0;
            cumvalid = true;
            maxpassen = 0; maxfreigh = 0;
        }
        
    }
    //补充起点的下行和终点的上行边界
    if (cfg.show_ruler_bar) {
        leftItems.append(scene()->addLine(
            margins.left_white, start_y, margins.left_white + margins.ruler_label_width / 2.0, start_y,
            defaultPen
        ));
        leftItems.append(scene()->addLine(
            margins.left_white + margins.ruler_label_width / 2.0,
            start_y + height, margins.left_white + margins.ruler_label_width, start_y + height, defaultPen
        ));
    }
    if (cfg.show_count_bar) {
        rightItems.append(scene()->addLine(
            scene()->width() - margins.right_white - margins.count_label_width, start_y,
            scene()->width() - margins.right_white - margins.count_label_width / 2.0, start_y,
            defaultPen
        ));
        rightItems.append(scene()->addLine(
            scene()->width() - margins.right_white - margins.count_label_width / 2.0,
            start_y + height,
            scene()->width() - margins.right_white, start_y + height, defaultPen
        ));
    }
}

void DiagramWidget::setVLines(double width, int hour_count, 
    const QList<QPair<double, double>> railYRanges)
{
    int gap = config().minutes_per_vertical_line;
    double gap_px = minitesToPixels(gap);
    int minute_thres = config().minute_mark_gap_pix;
    int minute_marks_gap = std::max(int(minute_thres / gap_px), 1);  //每隔多少竖线标注一次分钟
    int vlines = 60 / gap;   //每小时纵线数量+1
    int centerj = vlines / 2;   //中心那条线的j下标。与它除minute_marks_gap同余的是要标注的

    QPen
        pen_hour(config().grid_color, config().bold_grid_width),
        pen_half(config().grid_color, config().default_grid_width, Qt::DashLine),
        pen_other(config().grid_color, config().default_grid_width);

    QList<QGraphicsItem*> topItems, bottomItems;

    QColor color(Qt::white);
    color.setAlpha(200);

    topItems.append(scene()->addRect(
        config().totalLeftMargin() - 15, 0,
        width + config().totalLeftMargin() + 30, 35, QPen(Qt::transparent), color
    ));
    topItems.append(scene()->addLine(
        config().totalLeftMargin() - 15, 35,
        width + config().totalLeftMargin() + 15, 35, QPen(config().grid_color, 2)
    ));

    bottomItems.append(scene()->addRect(
        config().totalLeftMargin() - 15, scene()->height() - 35,
        width + config().totalLeftMargin() + 30, 35, QPen(Qt::transparent), color
    ));
    bottomItems.append(scene()->addLine(
        config().totalLeftMargin() - 15, scene()->height() - 35,
        width + config().totalLeftMargin() + 15,
        scene()->height() - 35, QPen(config().grid_color, 2)
    ));

    QFont font;
    font.setPixelSize(25);
    font.setBold(true);

    QFont fontmin;
    fontmin.setPixelSize(15);
    fontmin.setBold(true);

    for (int i = 0; i < hour_count + 1; i++) {
        double x = config().totalLeftMargin() + i * 3600 / config().seconds_per_pix;
        int hour = (i + config().start_hour) % 24;
        auto* textItem1 = addTimeAxisMark(hour, font, x);
        textItem1->setY(30 - textItem1->boundingRect().height());
        topItems.append(textItem1);

        auto* textItem2 = addTimeAxisMark(hour, font, x);
        textItem2->setY(scene()->height() - 30);
        bottomItems.append(textItem2);

        if (i == hour_count)
            break;

        //小时线
        if (i) {
            for (const auto& t : railYRanges) {
                scene()->addLine(x, t.first, x, t.second, pen_hour);
            }
        }
        //分钟线
        for (int j = 1; j < vlines; j++) {
            x += gap * 60 / config().seconds_per_pix;
            double minu = j * gap;
            for (const auto& t : railYRanges) {
                auto* line = scene()->addLine(x, t.first, x, t.second);
                if (minu == 30)
                    line->setPen(pen_half);
                else
                    line->setPen(pen_other);
            }
            if (j % minute_marks_gap == centerj % minute_marks_gap) {
                //标记分钟数
                textItem1 = addTimeAxisMark(int(std::round(minu)), fontmin, x);
                textItem1->setY(30 - textItem1->boundingRect().height());
                topItems.append(textItem1);
                textItem2 = addTimeAxisMark(int(std::round(minu)), fontmin, x);
                textItem2->setY(scene()->height() - 30);
                bottomItems.append(textItem2);
            }
        }
    }
    marginItems.top = scene()->createItemGroup(topItems);
    marginItems.top->setZValue(15);
    marginItems.bottom = scene()->createItemGroup(bottomItems);
    marginItems.bottom->setZValue(15);
}


double DiagramWidget::minitesToPixels(int minutes) const
{
    return minutes * 60.0 / config().seconds_per_pix;
}


void DiagramWidget::paintTrain(std::shared_ptr<Train> train)
{
    paintTrain(*train);
}

void DiagramWidget::paintTrain(Train& train)
{
    _page->clearTrainItems(train);
    if (!train.isShow())
        return;

    for (auto adp : train.adapters()) {
        //暂时按照暴力平方遍历的方法去找符合条件的Railway
        for (int i = 0; i < _page->railwayCount(); i++) {
            auto r = _page->railways().at(i);
            if ((adp->railway()) == r) {
                for (auto line : adp->lines()) {
                    if (line->isNull()) {
                        //这个是不应该的
                        qDebug() << "DiagramWidget::paintTrain: WARNING: " <<
                            "Unexpected null TrainLine! " << train.trainName().full() << Qt::endl;
                    }
                    else if(line->show()) {
                        auto* item = new TrainItem(_diagram, line, *adp->railway(), *_page,
                            _page->startYs().at(i));
                        _page->addItemMap(line.get(), item);
                        item->setZValue(5);
                        scene()->addItem(item);
                    }
                }
            }
        }
    }
}

void DiagramWidget::paintTrainLine(std::shared_ptr<TrainLine> line)
{
    if (line->isNull()) {
        //这个是不应该的
        qDebug() << "DiagramWidget::paintTrain: WARNING: " <<
            "Unexpected null TrainLine! " << line->adapter().train()->trainName().full() << Qt::endl;
    }
    else {
        auto* item = new TrainItem(_diagram, line, *line->adapter().railway(), *_page,
            _page->railwayStartY(*line->adapter().railway()));
        _page->addItemMap(line.get(), item);
        item->setZValue(5);
        scene()->addItem(item);
    }
}

QGraphicsSimpleTextItem* DiagramWidget::addLeftTableText(const char* str, 
    const QFont& textFont, double start_x, double start_y, double width, double height)
{
    return addLeftTableText(QObject::tr(str), textFont, start_x, start_y, width, height);
}

QGraphicsSimpleTextItem* DiagramWidget::addLeftTableText(const QString& str, 
    const QFont& textFont, double start_x, double start_y, double width, double height)
{
    const QColor& textColor = config().text_color;

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
    const QColor& textColor = config().text_color;
    //start_x += margins.left_white;

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
    scene()->addLine(config().totalLeftMargin(), y, width + config().totalLeftMargin(), y, pen);

    leftItems.append(alignedTextItem(name, textFont, margins().label_width - 5,
        label_start_x + 5, y));
    
    rightItems.append(alignedTextItem(name, textFont, margins().label_width,
                                      scene()->width() - config().rightRectWidth()-5 - margins().right_white , y));
}

const MarginConfig& DiagramWidget::margins() const
{
    return _page->margins();
}

const Config &DiagramWidget::config() const
{
    return _page->config();
}

QGraphicsSimpleTextItem* DiagramWidget::alignedTextItem(const QString& text, 
    const QFont& baseFont, double label_width, double start_x, double center_y, 
    bool use_stretch,double scale)
{
    auto* textItem = scene()->addSimpleText(text, baseFont);
    textItem->setBrush(config().text_color);
    textItem->setY(center_y - textItem->boundingRect().height() / 2);
    double textWidth = textItem->boundingRect().width();
    QFont font(baseFont);
    if (textWidth > label_width*scale) {
        int stretch = 100 * label_width*scale / textWidth;
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

QGraphicsSimpleTextItem* DiagramWidget::addTimeAxisMark(int value, const QFont& font, int x)
{
    auto* item = scene()->addSimpleText(QString::number(value), font);
    item->setX(x - item->boundingRect().width() / 2);
    item->setBrush(config().grid_color);
    return item;
}

void DiagramWidget::selectTrain(TrainItem* item)
{
    if (updating)
        return;
    if (!item)
        return;

    emit railFocussedIn(item->trainLine()->railway());
    _selectedTrain = item->train();
    _page->highlightTrainItems(*_selectedTrain);

    nowItem->setText(_selectedTrain->trainName().full());

    showWeakenItem();
    emit trainSelected(_selectedTrain);
    emit showNewStatus(tr("运行图 [%1] 选中车次运行线 [%2]").arg(_page->name(),
        _selectedTrain->trainName().full()));
}

void DiagramWidget::unselectTrain()
{
    if (updating)
        return;
    if (_selectedTrain) {
        _page->unhighlightTrainItems(*_selectedTrain);
        _selectedTrain = nullptr;
        nowItem->setText(" ");
    }
    hideWeakenItem();
}

void DiagramWidget::showAllForbids()
{
    for (int i = 0; i < _page->railwayCount();i++) {
        auto p = _page->railwayAt(i);
        for (auto f : p->forbids()) {
            showForbid(f, Direction::Down, i);
            showForbid(f, Direction::Up, i);
        }
    }
}


void DiagramWidget::updateForbid(std::shared_ptr<Forbid> forbid, Direction dir)
{
    showForbid(forbid, dir);
}

void DiagramWidget::zoomIn()
{
    scale(1.25, 1.25);
}

void DiagramWidget::zoomOut()
{
    scale(0.80, 0.80);
}

void DiagramWidget::locateToStation(std::shared_ptr<const Railway> railway, 
    std::shared_ptr<const RailStation> station, const QTime& tm)
{
    double x = calXFromStart(tm) ;
    if (x > config().diagramWidth()) {
        QMessageBox::warning(this, tr("错误"), tr("无法定位到所给时刻，"
            "可能因为所给时刻不在铺画范围内。"));
        return;
    }
    x += config().totalLeftMargin();
    double y = _page->railwayStartY(*railway) +
        railway->yValueFromCoeff(station->y_coeff.value(), config());
    QString text = tr("线路：%1\n车站：%2\n时刻：%3")
        .arg(railway->name(), station->name.toSingleLiteral(), tm.toString("hh:mm:ss"));
    centerOn(x, y);
    showPosTip(mapToGlobal(mapFromScene(x, y)), text);
}

void DiagramWidget::locateToMile(std::shared_ptr<const Railway> railway, double mile, const QTime& tm)
{
    Railway::SectionInfo info = railway->getSectionInfo(mile);
    if (!info.has_value()) {
        QMessageBox::warning(this, tr("错误"), tr("所给里程标无法定位到运行图上，" 
            "可能是因为所给里程标不在有效范围内。"));
        return;
    }
    double y = railway->yValueFromCoeff(std::get<0>(info.value()), config()) + _page->railwayStartY(*railway);
    double x = calXFromStart(tm);
    if (x > config().diagramWidth()) {
        QMessageBox::warning(this, tr("错误"), tr("无法定位到所给时刻，"
            "可能因为所给时刻不在铺画范围内。"));
        return;
    }
    x += config().totalLeftMargin();
    QString text = tr("线路：%1\n里程标：%2 km\n时刻：%3")
        .arg(railway->name(), QString::number(mile, 'f', 3), tm.toString("hh:mm:ss"));
    centerOn(x, y);
    showPosTip(mapToGlobal(mapFromScene(x, y)), text);
}

void DiagramWidget::showForbid(std::shared_ptr<Forbid> forbid, Direction dir)
{
    int idx = _page->railwayIndex(*(forbid->railway()));
    if (idx == -1)
        return;
    showForbid(forbid, dir, idx);
}

void DiagramWidget::showForbid(std::shared_ptr<Forbid> forbid, Direction dir, int index)
{
    removeForbid(forbid, dir);
    if (!forbid->isDirShow(dir)) {
        return;
    }
    double starty = _page->startYs().at(index);
    QPen pen(Qt::transparent);
    bool isService = (forbid->index() == 0);
    QColor color;
    if (isService) {
        color = QColor(85, 85, 85);
    }
    else {
        color = QColor(85, 85, 255);
    }
    color.setAlpha(200);
    QBrush brush(color);
    if (!forbid->different()) {
        if (dir != Direction::Down)
            return;
        brush.setStyle(Qt::DiagCrossPattern);
    }
    else if (dir == Direction::Down) {
        brush.setStyle(Qt::FDiagPattern);
    }
    else {
        brush.setStyle(Qt::BDiagPattern);
    }
    QBrush brush2(Qt::transparent);
    if (!isService) {
        brush2 = QColor(170, 170, 170, 80);
    }

    for (auto node = forbid->firstDirNode(dir); node; node = node->nextNode()) {
        addForbidNode(forbid, node, brush, pen, starty);
        if (!isService) {
            addForbidNode(forbid, node, brush2, pen, starty);
        }
    }
}

void DiagramWidget::removeForbid(std::shared_ptr<Forbid> forbid, Direction dir)
{
    auto& items = _page->dirForbidItem(forbid.get(), dir);
    for (auto p : items) {
        scene()->removeItem(p);
    }
    items.clear();
}

void DiagramWidget::addForbidNode(std::shared_ptr<Forbid> forbid, 
    std::shared_ptr<ForbidNode> node, const QBrush& brush, const QPen& pen, double start_y)
{
    auto rail = forbid->railway();
    auto& railint = node->railInterval();
    double y1 = rail->yValueFromCoeff(railint.fromStation()->y_coeff.value(), config()),
        y2 = rail->yValueFromCoeff(railint.toStation()->y_coeff.value(), config());
    if (y1 > y2)
        std::swap(y1, y2);
    //保证y1<=y2  方便搞方框
    double xstart = calXFromStart(node->beginTime),
        xend = calXFromStart(node->endTime);
    double width = config().diagramWidth();    //图形总宽度
    if (xstart == xend)   //莫得数据，再见
        return;
    else if (xstart < xend) {
        //没有跨界的简单情况
        if (xstart <= width) {
            //存在界内部分
            xend = std::min(xend, width);
            auto* item = scene()->addRect(xstart + config().totalLeftMargin(),
                y1 + start_y, xend - xstart, y2 - y1,
                pen, brush);
            _page->addForbidItem(forbid.get(), railint.direction(), item);
        }
    }
    else {
        //存在跨界的情况  先处理右半部分
        if (xstart <= width) {
            _page->addForbidItem(forbid.get(),railint.direction(), scene()->addRect(
                xstart + config().totalLeftMargin(), y1 + start_y, width - xstart, y2 - y1, pen, brush
            ));
        }
        //左半部分  一定有
        _page->addForbidItem(forbid.get(), railint.direction(), scene()->addRect(
            config().totalLeftMargin(), y1 + start_y, xend, y2 - y1, pen, brush
        ));
    }
}

double DiagramWidget::calXFromStart(const QTime& time) const
{
    int sec = startTime.secsTo(time);
    if (sec < 0)
        sec += 24 * 3600;
    return sec / config().seconds_per_pix;
}

TrainItem* DiagramWidget::posTrainItem(const QPointF& pos)
{
    auto* item = scene()->itemAt(pos, transform());
    if (!item)
        return nullptr;
    //qDebug() << "item: " << item->type() << Qt::endl;
    //if (item == marginItems.left) {
    //    qDebug() << "left!!" << Qt::endl;
    //    //scene()->addRect(item->boundingRect());
    //}
    while (item->parentItem())
        item = item->parentItem();
    if (item->type() == TrainItem::Type)
        return qgraphicsitem_cast<TrainItem*>(item);
    return nullptr;
}

void DiagramWidget::stationToolTip(std::deque<AdapterStation>::const_iterator st, const TrainLine& line)
{
    QString text = tr("%1 在 %2 站 ").arg(line.train()->trainName().full())
        .arg(st->trainStation->name.toSingleLiteral());
    if (st->trainStation->isStopped()) {
        text += tr("%1/%2 停车 %3").arg(st->trainStation->arrive.toString("hh:mm:ss"))
            .arg(st->trainStation->depart.toString("hh:mm:ss"))
            .arg(st->trainStation->stopString());
    }
    else {
        text += tr("%1/...").arg(st->trainStation->arrive.toString("hh:mm:ss"));
    }
    setToolTip(text);
}

void DiagramWidget::intervalToolTip(std::deque<AdapterStation>::const_iterator former, 
    std::deque<AdapterStation>::const_iterator latter, const TrainLine& line)
{
    const auto& ts1 = former->trainStation, & ts2 = latter->trainStation;
    int sec = qeutil::secsTo(ts1->depart, ts2->arrive);
    double mile = std::abs(latter->railStation.lock()->mile -
        former->railStation.lock()->mile);
    double spd = mile / sec * 3600;
    QString text = tr("%1 在 %2 - %3 区间  %4-%5  运行 %6\n区间里程 %7 km  技术速度 %8 km/h")
        .arg(line.train()->trainName().full())
        .arg(ts1->name.toSingleLiteral())
        .arg(ts2->name.toSingleLiteral())
        .arg(ts1->depart.toString("hh:mm:ss"))
        .arg(ts2->arrive.toString("hh:mm:ss"))
        .arg(qeutil::secsToString(sec))
        .arg(mile, 0, 'f', 3)
        .arg(spd, 0, 'f', 3);
    setToolTip(text);
}


void DiagramWidget::updateTimeAxis()
{
    if (updating)
        return;
    QPoint p(0, 0);
    auto ps = mapToScene(p);
    marginItems.top->setY(ps.y());
    nowItem->setY(ps.y());
    QPoint p1(0, height());
    auto ps1 = mapToScene(p1);
    marginItems.bottom->setY(ps1.y() - scene()->height() - 27);
}

void DiagramWidget::updateDistanceAxis()
{
    if (updating)return;
    QPoint point(0, 0);
    auto scenepoint = mapToScene(point);
    marginItems.left->setX(scenepoint.x());
    nowItem->setX(scenepoint.x());
    QPoint p2(width(), 0);
    auto sp2 = mapToScene(p2);
    marginItems.right->setX(sp2.x() - scene()->width() - 20);
}

void DiagramWidget::highlightTrain(std::shared_ptr<Train> train)
{
    if (!_page->hasTrain(*train))
        return;
    unselectTrain();
    _selectedTrain = train;

    setTrainShow(train, true);
    _page->highlightTrainItems(*_selectedTrain);

    nowItem->setText(_selectedTrain->trainName().full());
    showWeakenItem();
}

void DiagramWidget::highlightRouting(std::shared_ptr<Routing> routing)
{
    for (auto& p : routing->order()) {
        if (p.isVirtual())
            continue;
        _page->highlightTrainItemsWithLink(*(p.train()));
    }
    showWeakenItem();
}

void DiagramWidget::unhighlightRouting(std::shared_ptr<Routing> routing)
{
    for (auto& p : routing->order()) {
        if (p.isVirtual())
            continue;
        _page->unhighlightTrainItemsWithLink(*(p.train()));
    }
    hideWeakenItem();
}


void DiagramWidget::showTrainEventText()
{
    if (!_selectedTrain) {
        QMessageBox::warning(this, QObject::tr("错误"),
            QObject::tr("当前车次事件表：请先选择一个车次！"));
        return;
    }
    TrainEventList lst = _diagram.listTrainEvents(*_selectedTrain);
    QString res;
    for (const auto& p : lst) {
        res += _selectedTrain->trainName().full() + " 在 " +
            p.first->railway()->name() + " 上的事件时刻表: \n";
        for (const auto& q : p.second) {
            //站内事件表
            for (const auto& r : q.stEvents) {
                res += r.toString() + '\n';

                //debug: 把与本次列车有交集的车都画出来！
                //if (r.another.has_value()&&!r.another.value().get().isShow()) {
                //    auto& rr = const_cast<Train&>(r.another.value().get());
                //    rr.setIsShow(true);
                //    paintTrain(rr);
                //}
            }
            for (const auto& r : q.itEvents) {
                res += r.toString() + '\n';
            }
            
        }
    }

    QDialog* dialog=new QDialog(this);
    QVBoxLayout* layout = new QVBoxLayout;
    auto* browser = new  QTextBrowser;
    browser->setText(res);
    layout->addWidget(browser);
    dialog->setLayout(layout);
    dialog->show();
    dialog->exec();
}
