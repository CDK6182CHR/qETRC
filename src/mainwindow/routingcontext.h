#pragma once

#ifndef QETRC_MOBILE_2
#include <QObject>
#include <SARibbonContextCategory.h>
#include <QUndoCommand>
#include <DockWidget.h>
#include <QSet>
#include <QList>
#include <memory>
#include <vector>
#include "editors/routing/routingedit.h"

class Routing;
class MainWindow;
class Diagram;
class QLineEdit;
class RoutingDiagramWidget;
struct SplitRoutingData;
class RoutingWidget;

/**
 * @brief The RoutingContext class
 */
class RoutingContext : public QObject
{
    Q_OBJECT;
    std::shared_ptr<Routing> _routing;
    Diagram& _diagram;
    SARibbonContextCategory* const cont;
    MainWindow* const mw;
    QList<RoutingEdit*> routingEdits;
    QList<RoutingDiagramWidget*> diagramWidgets;
    QList<ads::CDockWidget*> routingDocks, diagramDocks;
    QSet<RoutingEdit*> syncEdits;
    bool updating = false;
    SARibbonToolButton* btnHighlight;

    QLineEdit* edName;
public:
    explicit RoutingContext(Diagram& diagram, SARibbonContextCategory* context, MainWindow* mw_);
    auto routing(){return _routing;}
    auto* context(){return cont;}
private:
    void initUI();
    int getRoutingWidgetIndex(std::shared_ptr<Routing> routing);
    int getRoutingWidgetIndex(RoutingEdit* w);

    void updateRoutingEditBasic(std::shared_ptr<Routing> routing);
    void updateRoutingEdit(std::shared_ptr<Routing> routing);

signals:
    void routingHighlightChanged(std::shared_ptr<Routing> routing);
    void routingInfoChanged(std::shared_ptr<Routing> routing);
    void routingInfoChangedAt(int index);
    void removeRouting(std::shared_ptr<Routing> routing);
private slots:
    void actEdit();
    void toggleHighlight(bool on);

    void removeRoutingEdit();

    void actParseText();

    void actDetectTrain();

    void actRemoveRouting();

    void actRoutingDiagram();

    void actChangeRouting();

    void actMergeRouting();

public slots:
    void refreshData();
    void refreshAllData();

    /**
     * 2025.02.06  Refresh all the data for the given routing (including windows and the tool page data)
     */
    void refreshDataFor(std::shared_ptr<Routing> routing);

    void setRouting(std::shared_ptr<Routing> routing);
    void openRoutingEditWidget(std::shared_ptr<Routing> routing);
    void removeRoutingEditWidget(std::shared_ptr<Routing> routing);
    // 2024.04.19: this version removes only the non-synchronized widget
    void removeNonSyncRoutingEditWidget(std::shared_ptr<Routing> routing);
    void removeRoutingEditWidgetAt(int i);

    /**
     * 2024.04.19  add: called on close diagram. The synchronized widgets are kept.
     */
    void removeAllRoutingDocks();

    /**
     * 这时实际的操作。这个操作不需要undo。
     */
    void onRoutingHighlightChanged(std::shared_ptr<Routing> routing, bool on);

    //操作压栈
    void actRoutingInfoChange(std::shared_ptr<Routing> routing, std::shared_ptr<Routing> info);

    /**
     * 执行交路变更，主要是更新navi和routingWidget的信息
     */
    void commitRoutingInfoChange(std::shared_ptr<Routing> routing);

    // 操作压栈
    void actRoutingOrderChange(std::shared_ptr<Routing> routing, std::shared_ptr<Routing> info);

    void commitRoutingOrderChange(std::shared_ptr<Routing> routing, 
        QSet<std::shared_ptr<Train>> takenTrains);

    /**
     * 指定交路被删除
     * 关闭Widget，focusout，必要时重绘运行线
     */
    void onRoutingRemoved(std::shared_ptr<Routing> routing);

    void actBatchRoutingUpdate(const QVector<int>& indexes,
        const QVector<std::shared_ptr<Routing>>& data);

    /**
     * 批量交路识别之后的应用操作
     * 在这里执行操作
     */
    void commitBatchRoutingUpdate(const QVector<int>& indexes,
        const QVector<std::shared_ptr<Routing>>& data);

    void openRoutingDiagramWidget(std::shared_ptr<Routing> routing);

    /**
     * 2024.03.23  Create new routing containing the given train.
     */
    void createRoutingByTrain(std::shared_ptr<Train> train);

    /**
     * 2024.03.26  add (append) train to given routing
     */
    void addTrainToRouting(std::shared_ptr<Routing> routing, std::shared_ptr<Train> train);

    /**
     * 2024.08.03 Push stack: split routing into pieces
     */
    void actSplitRouting(std::shared_ptr<Routing> routing, std::vector<SplitRoutingData>& data);

};

namespace qecmd {
    class ChangeRoutingInfo :public QUndoCommand
    {
        std::shared_ptr<Routing> routing, info;
        RoutingContext* const cont;
    public:
        ChangeRoutingInfo(std::shared_ptr<Routing> routing_, std::shared_ptr<Routing> info_,
            RoutingContext* context, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class ChangeRoutingOrder :public QUndoCommand
    {
        std::shared_ptr<Routing> routing, data;
        RoutingContext* const cont;
    public:
        ChangeRoutingOrder(std::shared_ptr<Routing> routing_, std::shared_ptr<Routing> info_,
            RoutingContext* context, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    private:
        void commit();
    };

    class BatchChangeRoutings :public QUndoCommand
    {
        QVector<int> indexes;
        QVector<std::shared_ptr<Routing>> routings;
        RoutingContext* const cont;
    public:
        BatchChangeRoutings(const QVector<int>& indexes_,
            const QVector<std::shared_ptr<Routing>>& routings_, RoutingContext* context,
            QUndoCommand* parent = nullptr):
            QUndoCommand(QObject::tr("批量更改%1个交路").arg(indexes_.size()),parent),
            indexes(indexes_),routings(routings_),cont(context){}
        virtual void undo()override;
        virtual void redo()override;
    };


    class SplitRouting : public QUndoCommand
    {
        std::shared_ptr<Routing> routing;
        std::vector<SplitRoutingData> data;
        RoutingContext* const cont;
    public:
        SplitRouting(std::shared_ptr<Routing> routing, std::vector<SplitRoutingData>&& data,
            RoutingContext* cont, RoutingWidget* rw, QUndoCommand* parent = nullptr);
    };

}

#endif
