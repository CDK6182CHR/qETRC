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
class QToolButton;
struct SplitRoutingData;

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
    QToolButton* btnSync;
    AddRoutingNodeDialog* dlgAdd = nullptr;
public:
    explicit RoutingEdit(TrainCollection& coll_, std::shared_ptr<Routing> routing_, QWidget *parent = nullptr);
    auto getRouting(){return routing;}
    auto* getModel(){return model;}
    virtual bool event(QEvent* e)override;
    bool isSynchronized()const;
private:
    void initUI();

signals:
    void closeDock();
    void routingInfoChanged(std::shared_ptr<Routing> routing, std::shared_ptr<Routing> info);
    
    /**
     * 次序变更；同时会完成infoChange。因此同时变化的，只要emit一个就行
     */
    void routingOrderChanged(std::shared_ptr<Routing> routing, std::shared_ptr<Routing> info);

    void focusInRouting(std::shared_ptr<Routing>);

    void synchronizationChanged(bool on);

    void routingSplit(std::shared_ptr<Routing>, std::vector<SplitRoutingData>&);

public slots:
    void refreshData();
    void refreshBasicData();
    void setRouting(std::shared_ptr<Routing> r);
    void resetRouting();

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
    void actSplit();
    void rowInserted(int row);
    void onParseDone(std::shared_ptr<Routing>original, std::shared_ptr<Routing> tmp);
    void onDetectDone(std::shared_ptr<const Routing> origin, std::shared_ptr<Routing> tmp);
};

