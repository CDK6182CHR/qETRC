#pragma once

#include <QObject>
#include <SARibbonContextCategory.h>
#include <QUndoCommand>
#include <DockWidget.h>
#include "editors/routing/routingedit.h"

class Routing;
class MainWindow;
class Diagram;
class QLineEdit;

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
    QList<ads::CDockWidget*> routingDocks;
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
private slots:
    void actEdit();
    void toggleHighlight(bool on);

    void removeRoutingEdit();

    void actParseText();

public slots:
    void refreshData();
    void refreshAllData();
    void setRouting(std::shared_ptr<Routing> routing);
    void openRoutingEditWidget(std::shared_ptr<Routing> routing);
    void removeRoutingEditWidget(std::shared_ptr<Routing> routing);
    void removeRoutingEditWidgetAt(int i);

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


}

