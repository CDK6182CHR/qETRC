#pragma once

#include <QWidget>
#include <QSet>
#include <memory>
#include "model/train/routingeditmodel.h"
#include "addroutingnodedialog.h"

class Routing;
class QTableView;
class QTextEdit;
class QLineEdit;
class TrainCollection;


/**
 * @brief The RoutingEdit class
 * 单个交路的编辑页面。
 * 原来的pyETRC.circuitDialog，但现在改用QWidget，以支持Dock
 */
class RoutingEdit : public QWidget
{
    Q_OBJECT;
    TrainCollection& coll;
    std::shared_ptr<Routing> routing;
    RoutingEditModel* const model;

    QTableView* table;
    QTextEdit* edNote;
    QLineEdit *edName,*edModel,*edOwner;
    AddRoutingNodeDialog* dlgAdd = nullptr;
public:
    explicit RoutingEdit(TrainCollection& coll_, std::shared_ptr<Routing> routing_, QWidget *parent = nullptr);
    auto getRouting(){return routing;}
    auto* getModel(){return model;}
    void refreshData();
    void refreshBasicData();
private:
    void initUI();

signals:
    void closeDock();
    void routingInfoChanged(std::shared_ptr<Routing> routing, std::shared_ptr<Routing> info);
    
    /**
     * 次序变更；同时会完成infoChange。因此同时变化的，只要emit一个就行
     */
    void routingOrderChanged(std::shared_ptr<Routing> routing, std::shared_ptr<Routing> info);

private slots:
    void actAddBefore();
    void actAddAfter();
    void insertAt(int row);
    void actRemove();
    void actMoveUp();
    void actMoveDown();
    void actParse();
    void actDetect();
    void actApply();
    void actCancel();

};

