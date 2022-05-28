#pragma once

#ifndef QETRC_MOBILE_2
#include <QWidget>
#include <QString>
#include <QUndoCommand>
#include <QList>
#include <deque>


namespace qecmd {
    struct  StartingTerminalData;
}

class Train;
class TrainCollection;
class TrainListModel;
class QTableView;
class QLineEdit;
class TrainType;


/**
 * @brief The TrainListWidget class
 * pyETRC.TrainWidget
 * 列车列表  主体是一个类似只读的ListView
 * 暂时直接套ListView实现
 * 注意尽量不要用Diagram的API，方便以后用到列车数据库中去
 * undoStack传入空指针表示不支持撤销
 * 
 * 2022.05.14：把批量更改始发终到站等一系列功能搞到这边来执行。
 * 由于拿不到TrainContext的指针，最终一步压栈只能交给TrainContext完成。
 */
class TrainListWidget : public QWidget
{
    Q_OBJECT

    TrainCollection& coll;
    QUndoStack* const _undo;
    QTableView* table;
    QLineEdit* editSearch;
    TrainListModel* model;

public:
    explicit TrainListWidget(TrainCollection& coll_, QUndoStack* undo, QWidget *parent_ = nullptr);

    /**
     * 刷新数据。
     * TODO: 这个操作总觉得很危险。。
     */
    void refreshData();

    auto* getModel() { return model; }

private:
    void initUI();

    /**
     * 用于批量操作的：当前已选择的列车表。
     * 如果为空，则直接报错
     */
    QList<std::shared_ptr<Train>> batchOpSelectedTrains();
    

signals:
    void currentTrainChanged(std::shared_ptr<Train> train);
    void editTrain(std::shared_ptr<Train> train);
    void trainShowChanged(std::shared_ptr<Train> train, bool show);
    void addNewTrain();

    /**
     * 列车发生排序，但没有增删改
     */
    //void trainReordered();

    void batchChangeStartingTerminal(qecmd::StartingTerminalData&);

    void batchExportTrainEventList(const QList<std::shared_ptr<Train>>&);

    void batchAutoBusiness(const QList<std::shared_ptr<Train>>&);

    void batchAutoChangeType(std::deque<std::pair<std::shared_ptr<Train>, std::shared_ptr<TrainType>>>&);

    void batchAutoCorrect(const QList<std::shared_ptr<Train>>&);
        
private slots:
    void searchTrain();
    void clearFilter();
    void editButtonClicked();

    /**
     * 2022.02.06  实现为批量更改类型
     */
    void batchChange();

    /**
     * 要做的事：
     * （1）删除运行线；
     * （2）从Diagram中删除数据；（这个由Main负责。这里没这个权限）
     * （3）创建CMD对象，通知Main发生了删除事件；
     * （4）暂定由Main负责发起更新界面数据的操作。
     */
    void actRemoveTrains();

    void onCurrentRowChanged(const QModelIndex& idx);

    void resetStartingTerminalFromTimetable(const QList<std::shared_ptr<Train>>& trains);

    void actResetStartingTerminalFromTimetableBat();

    void autoStartingTerminal(const QList<std::shared_ptr<Train>>& trains);
    void autoStartingTerminalLooser(const QList<std::shared_ptr<Train>>& trains);
    void autoTrainType(const QList<std::shared_ptr<Train>>& trains);

    void actAutoStartingTerminalBat();
    void actAutoStartingTerminalLooserBat();
    void actAutoTrainTypeBat();

    /**
     * 2022.05.14
     * 批量导出选中车次的事件表。
     * 由于这里拿不到diagram引用，只能交给TrainContext去处理
     */
    void actExportTrainEventListBat();

    void actExportTrainTimetableBat();

    void exportTrainTimetable(const QList<std::shared_ptr<Train>>& trains);

    /**
     * 2022.05.14  批量设置自动营业站
     * 和前面几个不一样；这里直接发送到TrainContext处理，
     * 因为原版实现就是在TrainContext里面的。
     */
    void actAutoBusinessBat();

    /**
     * 2022年5月28日  批量自动更正时刻表，
     * 参照AutoBusiness的路径来实现。
     */
    void actAutoCorrectionBat();

    void selectAll();

    void deselectAll();

    void selectInverse();

public slots:

    /**
     * 2022.05.14  从MainWindow移动过来
     */
    void actResetStartingTerminalFromTimetableAll();

    void actAutoStartingTerminalAll();

    void actAutoStartingTerminalLooserAll();

    void actAutoTrainTypeAll();

    void actExportTrainEventListAll();

};

#endif

