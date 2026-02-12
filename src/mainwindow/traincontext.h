#pragma once

#ifndef QETRC_MOBILE_2
#include <QObject>
#include <memory>
#include <QColor>
#include <deque>
#include <QUndoCommand>
#include <QSet>
#include <QPointer>

#include <data/train/train.h>

class SARibbonContextCategory;
class QLineEdit;
class QComboBox;
class QCheckBox;
class SARibbonToolButton;
class QDoubleSpinBox;
class Diagram;
class PenStyleCombo;
class BasicTrainWidget;
class EditTrainWidget;
class Routing;
class TrainType;
class TrainListModel;
class TrainCollection;
class DiagramNaviModel;
class TrainTagDialog;
class TrainTagManager;
class TrainTagListDirectModel;

struct TrainStationBounding;
namespace qecmd {
    struct StartingTerminalData;
    struct PathInfoInTrain;
}
namespace ads {
class CDockWidget;
}

class TrainContext;

namespace qecmd {
    class AutoTrainType :public QUndoCommand {
    public:
        using data_t = std::deque<std::pair<std::shared_ptr<Train>, std::shared_ptr<TrainType>>>;
        AutoTrainType(data_t&& d, TrainContext* context, QUndoCommand* parent = nullptr) :
            QUndoCommand(QObject::tr("自动推断列车类型"), parent), data(std::forward<data_t>(d)),
            cont(context) {}
        virtual void undo()override;
        virtual void redo()override;
    private:
        data_t data;
        TrainContext* const cont;
        void commit();
    };

    class AutoTrainPen : public QUndoCommand {
        std::deque<std::shared_ptr<Train>> _trains;
        std::deque<QPen> _pens;
        TrainContext* const cont;
    public:
        AutoTrainPen(std::deque<std::shared_ptr<Train>>&& trains,
            TrainContext* context, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class ManualTrainPen : public QUndoCommand {
        std::deque<std::shared_ptr<Train>> _trains;
        std::deque<QPen> _pens;
        TrainContext* const cont;

    public:
        ManualTrainPen(std::deque<std::shared_ptr<Train>> && trains, TrainContext* context, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    class ChangeTrainPen : public QUndoCommand {
        std::deque<std::shared_ptr<Train>> _trains;
        std::deque<std::optional<QPen>> _pens;
        TrainContext* const cont;
    public:
        ChangeTrainPen(std::deque<std::shared_ptr<Train>>&& trains, std::optional<QPen> pen, TrainContext* context,
            QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    private:
        void commit();
    };
}


class MainWindow;
class LocateBoundingDialog;
/**
 * @brief The TrainContext class
 * 列车的ContextMenu。尝试从MainWindow分出来，是为了减少MainWindow代码量。
 * 把相关操作代理了，MainWindow只负责转发
 * 暂时按照QObject处理，但其实好像不是很必要
 */
class TrainContext : public QObject
{
    Q_OBJECT
    Diagram& diagram;
    std::shared_ptr<Train> train{};
    SARibbonContextCategory* const cont;
    MainWindow* const mw;

    QLineEdit* edName, * edStart, * edEnd, * edRouting;
    SARibbonToolButton* btnShown;

    //后缀m表示可修改的控件
    QLineEdit *edNamem,*edStartm,*edEndm;
    QLineEdit *edNameDown,*edNameUp;
    QComboBox* comboType;
    PenStyleCombo* comboLs;
    QCheckBox *checkPassen;
    SARibbonToolButton* btnColor, * btnAutoUI, * btnToRouting, * btnCreateRouting, * btnAddToRouting;
    QDoubleSpinBox* spWidth;
    QColor tmpColor;

    QLineEdit* edPaths, * edTags;

    QList<BasicTrainWidget*> basicWidgets;
    QList<EditTrainWidget*> editWidgets;
    QList<ads::CDockWidget*> basicDocks, editDocks;
    QSet<EditTrainWidget*> syncEditors;
    QList<QPointer<TrainTagDialog>> tagDialogs;

    TrainTagListDirectModel* _tagCompletionModel = nullptr;

    LocateBoundingDialog* dlgLocate = nullptr;
public:
    TrainContext(Diagram& diagram_, SARibbonContextCategory* const context_, MainWindow* mw);
    auto* context() { return cont; }
    auto getTrain() { return train; }
    void resetTrain();

    /**
     * Return the model for tag completion; create it if not existed yet
     */
    TrainTagListDirectModel* getTagCompletionModel();

signals:
    void highlightTrainLine(std::shared_ptr<Train> train);
    void timetableChanged(std::shared_ptr<Train> train);
    //void actRemoveTrain(int index);
    void focusInRouting(std::shared_ptr<Routing>);
    void dulplicateTrain(std::shared_ptr<Train>);
    void trainTagListUpdated();
    
private:
    void initUI();

    void setupTypeCombo();

    /**
     * 返回当前列车对应的BasicWidget的下标
     * 暂定线性查找
     * 如果找不到，返回-1
     */
    int getBasicWidgetIndex();

    int getBasicWidgetIndex(std::shared_ptr<Train> t);

    int getEditWidgetIndex(std::shared_ptr<Train> t);

    /**
     * 由所给dock查找下标，用于关闭面板时删除
     */
    int getBasicWidgetIndex(ads::CDockWidget* dock);

    /**
     * 更新指定车次打开的时刻表窗口。未见得是当前车次。
     * 2021.10.17：加上EditWidget的更新。
     */
    void updateTrainWidget(std::shared_ptr<Train> t);

    /**
     * 2021.10.17  加上EditWidget的基础信息刷新
     */
    void updateTrainWidgetTitles(std::shared_ptr<Train> t);

public slots:
    void setTrain(std::shared_ptr<Train> train_);

    void refreshData();

    /**
     * 2023.08.16 setup only the path data.
     */
    void refreshPath();

    void refreshTags();

    /**
     * 更新所有数据，包括自己的数据和下设的widget
     */
    void refreshAllData();

    void removeTrainWidget(std::shared_ptr<Train> train);

    /**
     * 由更新命令直接调用，将操作cmd压栈
     */
    void onTrainTimetableChanged(std::shared_ptr<Train> train, std::shared_ptr<Train> table);

    /**
     * 2021.08.13  压栈操作
     * 由RulerPaint调用，主要是始发终到站的更新
     */
    void onTrainInfoChanged(std::shared_ptr<Train> train, std::shared_ptr<Train> info);

    /**
     * 实际更新时刻表。undo/redo调用。
     * 更新数据，重新绑定数据，更新时刻表页面，更新列表页面（可能有里程等数据的变化），铺画运行图
     */
    void commitTimetableChange(std::shared_ptr<Train> train, std::shared_ptr<Train> table);

    /**
     * 其他数据不变，只有单一车站时刻变化。由TimetableQuickEditableModel调用。
     * 也是更新时刻表，但不同的是，已知线路绑定情况不会变，不用重新绑定。
     */
    void onTrainStationTimeChanged(std::shared_ptr<Train> train, bool repaint);

    /**
     * 列车时刻表变化后的操作，2021.08.10抽离出来
     */
    void afterTimetableChanged(std::shared_ptr<Train> train);

    /**
     * 更新信息，重新铺画运行图（但不重新绑定）
     * 更新当前页面（目前只需要更新当前页面），通知列表页面更新
     * 似乎现在直接利用timetableChanged()信号就够了
     */
    void commitTraininfoChange(std::shared_ptr<Train> train, std::shared_ptr<Train> info);

    /**
     * 显示或者创建当前车次的基本编辑面板。
     */
    void actShowBasicWidget();

    void showBasicWidget(std::shared_ptr<Train> train);

    void actShowEditWidget();

    /**
     * 显示/创建完全编辑面板
     */
    void showEditWidget(std::shared_ptr<Train> train);

    /**
     * 清理运行图时，关闭所有既有的面板
     */
    void removeAllTrainWidgets();

    /**
     * 完成换线后操作，即重新铺画、更新等。
     */
    void commitExchangeTrainInterval(std::shared_ptr<Train> train1,
        std::shared_ptr<Train> train2);

    /**
     * 2022.05.14：操作压栈，
     * 由TrainListWidget那边来connect。
     */
    void actBatchChangeStartingTerminal(qecmd::StartingTerminalData& data);

    /**
     * 2022.05.14：操作压栈。
     */
    void actBatchAutoTrainType(qecmd::AutoTrainType::data_t& data);

    // 2024.03.17：操作压栈。TrainListWidget的部分操作接口。
    // 输入的参数将被move!!
    void batchAutoTrainPen(std::deque<std::shared_ptr<Train>>& trains);

    /**
     * 2025.02.01  批量取消自动运行线（保留当前运行线设置为手动设置）
     */
    void batchManualTrainPen(std::deque<std::shared_ptr<Train>> trains);

    /**
     * 2025.02.01  批量设置运行线类型  操作压栈 
     */
    void batchChangeTrainPen(std::deque<std::shared_ptr<Train>> trains, std::optional<QPen> pen);

    // 2024.03.17：操作压栈。MainWindow的全部车次操作。
    void actAutoPenAll();

    void actManualPenAll();

    /**
     * 自动始发终到的执行：执行始发终到站变更；铺画运行线；更新TrainList和相关Widget
     */
    void commitAutoStartingTerminal(qecmd::StartingTerminalData& data);

    /**
     * 自动列车类型的执行：类型变更；铺画运行线；更新TrainList和相关Current
     */
    void commitAutoType(std::deque<std::pair<std::shared_ptr<Train>, 
        std::shared_ptr<TrainType>>>& data);

    /**
     * 2024.03.17  for Undo/redo autoPen. The same operations after the data is set in the qecmd class.
     */
    void commitAutoPenOrUndo(const std::deque<std::shared_ptr<Train>>& trains);

    void commitBatchChangeTrainPen(const std::deque<std::shared_ptr<Train>>& trains);

    void actInterpolation(const QVector<std::shared_ptr<Train>>& trains,
        const QVector<std::shared_ptr<Train>>& data);

    void commitInterpolation(const QVector<std::shared_ptr<Train>>& trains,
        const QVector<std::shared_ptr<Train>>& data);

    /**
     * 撤销所有推定结果，直接从Toolbar调用。
     * 推定和撤销推定的commit操作实际上是完全一样的
     */
    void actRemoveInterpolation();

    /**
     * 代替quickWidget处理定位问题：显示对话框
     */
    void locateToBoundStation(const QVector<TrainStationBounding>& boudings,
        const TrainTime& time);

    /**
     * 自动营业站  代理MainWindow的东西
     * 操作压栈
     */
    void actAutoBusiness();

    /**
     * 自动营业站，批选而不是全选
     * 2022.05.14  由TrainListWidget调起
     */
    void actAutoBusinessBat(const QList<std::shared_ptr<Train>>& trainRange);

    void actAutoCorrectionAll();

    void actAutoCorrectionBat(const QList<std::shared_ptr<Train>>& trainRange);

    void actRemoveNonBound();

    void actRemoveNonBoundTrains();

    void actRemoveEmptyTrains();

#if 0

    /**
     * 自动营业站 执行 似乎不需要重新铺画，只需要刷新数据。
     * 2022.05.28注：交换时刻表然后重新绑定的操作，都和commitInterpolation一样，
     * 不再另外写实现了。
     */
    void commitAutoBusiness(const QVector<std::shared_ptr<Train>>& trains);

    /**
     * 自动更正时刻表  执行
     * 与AutoBusiness的区别是需要刷新
     */
    void commitAutoCorrection(const QVector<std::shared_ptr<Train>>& trains);

#endif

    /**
     * 当列车发生大范围变化时，更新当前context, 以及速览时刻、速览信息等窗口，
     * 以及所有的BasicWidget
     * 不管当前列车有没有改动。
     * 注意不要重复执行这个操作。
     */
    void refreshCurrentTrainWidgets();


    /**
     * 导入列车时刻表，代理MainWindow处理。
     */
    void actImportTrainFromCsv();

    /**
     * 2022.05.14
     * 从ETRC的trf格式批量导入；每个文件
     */
    void actImportTrainFromTrf();

    /**
     * 批量导出列车事件表到csv。
     * 直接解析返回的数据包写csv，不用其他的复杂API
     */
    void batchExportTrainEvents(const QList<std::shared_ptr<Train>>& trains);

    /**
     * 2023.02.03  使用事件表那套算法，简单推定通过站时刻。
     * 默认在当前列车+当前线路执行，不提供选项。
     */
    void actSimpleInterpolation();

    void actDragTimeSingle(std::shared_ptr<Train> train, int station_id, const TrainStation& data, 
        Qt::KeyboardModifiers mod);

    /**
     * 2024.03.26  non-local dragging (with Shift pressed)
     */
    void actDragTimeNonLocal(std::shared_ptr<Train> train, int station_id, std::shared_ptr<Train> data,
        Qt::KeyboardModifiers mod);

    /**
     * Post-processing of assign/remove paths to single train.
     * The operations seems to be the same.
     * The after- prefix implies the actual operation is not conducted here.
     */
    void afterChangeTrainPaths(std::shared_ptr<Train> train, const std::vector<TrainPath*>& paths);

    void afterChangeTrainPaths(std::shared_ptr<Train> train, const std::vector<qecmd::PathInfoInTrain>& data);

    /**
     * 2023.08.16  move from TrainListWidget, for the clearing of Paths
     */
    void actRemoveTrains(const QList<std::shared_ptr<Train>>& trains, const QList<int>& indexes);

    /**
     * 2023.08.16   moved from NaviTree
     */
    void actRemoveSingleTrain(int index);

    /**
     * 2023.08.22  the post-processing of re-bind trains by paths.
     */
    void afterTrainsReboundByPath(const std::vector<std::shared_ptr<Train>>& trains,
        const std::vector<QVector<std::shared_ptr<TrainAdapter>>>& adapters);

    void actMergeTrains();

    /**
     * Update the train shown status if the train is current train
     */
    void updateTrainShownStatus(std::shared_ptr<Train> train);

    /**
	 * 2025.08.13  Open the tag dialog for the given train.
     * The dialog will be stored in tagDialogs for further updating.
	 * We do not check whether multiple dialogs for the same train are opened.
     */
	void openTagDialog(std::shared_ptr<Train> train);

    /**
     * Add a new train tag. The tag may have already existed or not.
     */
    void actAddTrainTag(std::shared_ptr<Train> train, const QString& tagName);

	void actRemoveTrainTag(std::shared_ptr<Train> train, int index);

    /**
     * Call this after the train tag list is changed (add, remove, rename).
     * Called by the commands.
     */
    void onTrainTagListUpdated();

    void onTrainTagRenamed(std::shared_ptr<TrainTag> tag);

    void onTrainTagNoteChanged(std::shared_ptr<TrainTag> tag);

    /**
     * Call this function when the tags for a train has changed.
     */
	void onTrainTagChanged(std::shared_ptr<Train> train);

    /**
     * Call this function when tags for a number of trains have been changed.
     * Simply update all possible widgets without check for efficiency.
     */
    void onTrainTagChangedBatch();

    void actBatchAddTrainTag(const QString& tagName, const std::vector<std::shared_ptr<Train>>& trains);

    void actBatchRemoveTrainTag(std::shared_ptr<TrainTag>, const std::vector<std::pair<std::shared_ptr<Train>, int>>& data);

private slots:
    void showTrainEvents();

    void actToggleTrainLineShown(bool checked);

    void actShowTrainLine();

    void onTrainDockClosed();

    void onEditDockClosed();

    void removeBasicDockAt(int idx);

    void removeEditDockAt(int idx);

    void onFullNameChanged();

    void onAutoUIChanged(bool on);

    void actSelectColor();

    void actApply();

    void actRemoveCurrentTrain();

    /**
     * 2021.10.17  从Edit里面删除列车
     */
    void actRemoveTrainFromEdit(std::shared_ptr<Train> train);

    void actShowTrainLineDialog();

    void actRulerRef();

    void actExchangeInterval();

    /**
     * 压栈操作
     */
    void actApplyExchangeInterval(std::shared_ptr<Train> train1, std::shared_ptr<Train>train2,
        Train::StationPtr start1, Train::StationPtr end1,
        Train::StationPtr start2, Train::StationPtr end2, 
        bool includeStart, bool includeEnd); 

    void actAdjustTimetable();

    void actDiagnose();

    void actToRouting();

    void actCorrection();

    void actSplitTrain();

    void actApplySplitTrain(std::shared_ptr<Train> train, QVector<std::shared_ptr<Train>> newTrains);

    void actDulplicateTrain();

    void actAddPaths();

    void actRemovePaths();

    void actClearPaths();

    void actFocusInPath();

    /**
     * 2024.03.23  Create new routing based on current train. 
     */
    void actCreateRouting();

    /**
     * 2024.03.25  Add to existing routing, or create new routing (by calling actCreateRouting)
     */
    void actAddToRouting();

    void actChangeTrain();

};


namespace qecmd {
    class ChangeTimetable :
        public QUndoCommand
    {
        std::shared_ptr<Train> train, table;
        TrainContext* const cont;
    public:
        ChangeTimetable(std::shared_ptr<Train> train, std::shared_ptr<Train> newtable,
            TrainContext* context, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class UpdateTrainInfo :
        public QUndoCommand
    {
        std::shared_ptr<Train> train, info;
        TrainContext* const cont;
    public:
        UpdateTrainInfo(std::shared_ptr<Train> train_, std::shared_ptr<Train> newinfo,
            TrainContext* context, QUndoCommand* parent = nullptr) :
            QUndoCommand(QObject::tr("更新列车信息: ") + newinfo->trainName().full(),parent),
            train(train_), info(newinfo), cont(context) {}
        virtual void undo()override {
            cont->commitTraininfoChange(train, info);
        }
        virtual void redo()override {
            cont->commitTraininfoChange(train, info);
        }
    };

    class ExchangeTrainInterval :
        public QUndoCommand {
        std::shared_ptr<Train> train1, train2;
        Train::StationPtr start1, end1, start2, end2;
        bool includeStart, includeEnd;
        TrainContext* const cont;
    public:
        ExchangeTrainInterval(std::shared_ptr<Train> train1_, std::shared_ptr<Train> train2_,
            Train::StationPtr start1_, Train::StationPtr end1_,
            Train::StationPtr start2_, Train::StationPtr end2_,
            bool includeStart_, bool includeEnd_, TrainContext* context,
            QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("区间换线: %1 & %2").arg(train1_->trainName().full(),
            train2_->trainName().full()),parent),
            train1(train1_),train2(train2_),start1(start1_),end1(end1_),start2(start2_),
            end2(end2_),includeStart(includeStart_),includeEnd(includeEnd_),cont(context)
        {}

        virtual void undo()override;
        virtual void redo()override;

    private:
        void commit();
    };

    struct StartingTerminalData {
        std::deque<std::pair<std::shared_ptr<Train>, StationName>> startings;
        std::deque<std::pair<std::shared_ptr<Train>, StationName>> terminals;
        void commit();
    };

    class AutoStartingTerminal :public QUndoCommand {
        StartingTerminalData data;
        TrainContext* const cont;
    public:
        AutoStartingTerminal(StartingTerminalData&& data_,
            TrainContext* context,QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("自动始发终到站适配"), parent),
            data(data_),cont(context){}
        virtual void undo()override { commit(); }
        virtual void redo()override { commit(); }
    private :
        void commit();
    };



    class TimetableInterpolation :public QUndoCommand {
        QVector<std::shared_ptr<Train>> trains, data;
        TrainContext* const cont;
    public:
        TimetableInterpolation(const QVector<std::shared_ptr<Train>>& trains,
            const QVector<std::shared_ptr<Train>>& data,
            TrainContext* context, QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("列车时刻插值"),parent),
            trains(trains),data(data),cont(context){}
        virtual void undo()override;
        virtual void redo()override;
    };

    /**
     * 撤销所有推定结果：所有备注为“推定”的站直接删除
     */
    class RemoveInterpolation :public QUndoCommand {
        QVector<std::shared_ptr<Train>> trains, data;
        TrainContext* const cont;
    public:
        RemoveInterpolation(const QVector<std::shared_ptr<Train>>& trains,
            const QVector<std::shared_ptr<Train>>& data, TrainContext* cont,
            QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("撤销所有推定"),parent),trains(trains),data(data),
            cont(cont){}
        void undo()override;
        void redo()override;
    };

    class AutoBusiness :public QUndoCommand {
        QVector<std::shared_ptr<Train>> trains, data;
        TrainContext* const cont;
    public:
        AutoBusiness(QVector<std::shared_ptr<Train>>&& trains,
            QVector<std::shared_ptr<Train>>&& data, TrainContext* cont,
            QUndoCommand* parent = nullptr):
            QUndoCommand(QObject::tr("自动营业站"),parent),
            trains(std::forward<QVector<std::shared_ptr<Train>>>(trains)),
            data(std::forward<QVector<std::shared_ptr<Train>>>(data)),cont(cont){}

        void undo()override;
        void redo()override;
    private:
        void commit();
    };

    class BatchAutoCorrection :public QUndoCommand {
        QVector<std::shared_ptr<Train>> trains, data;
        TrainContext* const cont;
    public:
        BatchAutoCorrection(QVector<std::shared_ptr<Train>>&& trains,
            QVector<std::shared_ptr<Train>>&& data, TrainContext* cont,
            QUndoCommand* parent = nullptr) :
            QUndoCommand(QObject::tr("自动更正时刻表"), parent),
            trains(std::forward<QVector<std::shared_ptr<Train>>>(trains)),
            data(std::forward<QVector<std::shared_ptr<Train>>>(data)), cont(cont) {}

        void undo()override;
        void redo()override;
    private:
        void commit();
    };

    /**
     * 2023.02.03  快速推定，和ChangeTimetable唯一的区别是text()不一样
     * 构造函数多一个railway，仅仅用来生成text()。
     */
    class TimetableInterpolationSimple : public ChangeTimetable {
    public:
        TimetableInterpolationSimple(std::shared_ptr<Train> train, std::shared_ptr<Train> newtable,
            TrainContext* context, const Railway* rail, QUndoCommand* parent = nullptr);
    };

    /**
    * 2023.06.02  for dragging
    * like: AdjustTrainStationTime (defined in timetablequickmodel.h)
    * but called by dragging. 
    * The actual operation should be done inside this class.
     */
    class DragTrainStationTime : public QUndoCommand
    {
        std::shared_ptr<Train> train;
        int station_id;
        Qt::KeyboardModifiers mod;
        TrainStation data;
        TrainContext* const cont;

        static constexpr const int ID = 103;
        bool first = true;
    public:
        DragTrainStationTime(std::shared_ptr<Train> train, int station_id, Qt::KeyboardModifiers mod, const TrainStation& data,
            TrainContext* cont, QUndoCommand* parent = nullptr);

        virtual void undo()override;
        virtual void redo()override;
        virtual int id()const override { return ID; }
        virtual bool mergeWith(const QUndoCommand* other)override;
    private:
        void commit();
    };

    /**
     * 2024.03.26: non-local dragging of timetable. 
     * This is different from ChangeTimetable, since the ADDRESSES of the stations should not change
     */
    class DragNonLocalTime: public QUndoCommand
    {
        std::shared_ptr<Train> train, data;
        int station_id;
        Qt::KeyboardModifiers mod;
        TrainContext* const cont;

        static constexpr const int ID = 104;
        bool first = true;

    public:
        DragNonLocalTime(std::shared_ptr<Train> train, std::shared_ptr<Train> data,
            int station_id, Qt::KeyboardModifiers mod, TrainContext* cont, QUndoCommand* parent = nullptr);
        virtual void undo()override;   // not changed
        virtual void redo()override;
        virtual int id()const override { return ID; }
        virtual bool mergeWith(const QUndoCommand* other)override;
    };

    /**
     * Train-centered operation: assign (multiple) paths to (single) train.
     * The actual operations are performed here in the class.
     */
    class AssignPathsToTrain : public QUndoCommand
    {
        std::shared_ptr<Train> train;
        std::vector<TrainPath*> paths;
        TrainContext* const cont;
    public:
        AssignPathsToTrain(std::shared_ptr<Train> train, std::vector<TrainPath*>&& paths,
            TrainContext* cont, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    /**
     * This struct logs the path pointer as well as the indexes info of 1-1 path-train data.
     * The indexes are the ORIGINAL indexes in the list (i.e. BEFORE the removing)
     */
    struct PathInfoInTrain {
        TrainPath* path;
        int path_index_in_train;
        int train_index_in_path;
    };

    /**
     * Remove paths based on the data.
     * The info in data is guaranteed to be sorted by the order of `path_index_in_train`.
     */
    class RemovePathsFromTrain : public QUndoCommand {
        std::shared_ptr<Train> train;
        std::vector<PathInfoInTrain> data;
        TrainContext* const cont;
    public:
        RemovePathsFromTrain(std::shared_ptr<Train> train, std::vector<PathInfoInTrain>&& data,
            TrainContext* cont, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };


    /**
     * Similar to RemovePathsFromTrain, but this version removes *all* paths, which may be faster and simpler.
     * The indexes_in_path is generated in the constructor, and not required from outside.\
     * The operations are directly conducted here for convenience.
     */
    class ClearPathsFromTrain : public QUndoCommand {
        std::shared_ptr<Train> train;
        std::vector<TrainPath*> paths;
        std::vector<int> indexes_in_path;
        TrainContext* const cont;
    public:
        ClearPathsFromTrain(std::shared_ptr<Train> train, TrainContext* cont, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    /**
     * 2023.08.16  remove the trains. See also RemoveTrainsSimple (previous RemoveTrains)
     * Moved from TrainListModel.h/cpp
     */
    class RemoveTrains : public QUndoCommand
    {
    public:
        RemoveTrains(const QList<std::shared_ptr<Train>>& trains,
            const QList<int>& indexes, TrainCollection& coll_,
            TrainListModel* model_, TrainContext* cont,
            QUndoCommand* parent = nullptr);
    };

    class RemoveSingleTrain : public QUndoCommand
    {
    public:
        RemoveSingleTrain(TrainContext* cont, DiagramNaviModel* navi_,
            std::shared_ptr<Train> train_, int index_, QUndoCommand* parent = nullptr);
    };

    /**
     * 2023.08.22: For the trains bound using path.
     * On railway or path changed, re-bind all trains, and repaint their train lines.
     * Currently, the adapters are reserved, only calculated at the first time (ctor).
     */
    class RebindTrainsByPaths : public QUndoCommand
    {
        std::vector<std::shared_ptr<Train>> trains;
        std::vector<QVector<std::shared_ptr<TrainAdapter>>> adapters;
        TrainContext* const cont;
        bool first = true;
    public:
        RebindTrainsByPaths(std::vector<std::shared_ptr<Train>>&& trains_,
            TrainContext* cont, QUndoCommand* parent = nullptr);

        virtual void undo()override;
        virtual void redo()override;
    };

    /**
	 * 2025.08.13  Add a new tag to the manager. The new tag must NOT exist before.
     */
    class AddNewTrainTag : public QUndoCommand
    {
        TrainTagManager& manager;
        std::shared_ptr<TrainTag> tag;
		TrainContext* const cont;

    public:
        AddNewTrainTag(TrainTagManager& manager_, std::shared_ptr<TrainTag> tag_, TrainContext* cont, QUndoCommand* parent=nullptr);

		void undo()override;
        void redo()override;
    };

    class AppendTagToTrain : public QUndoCommand
    {
        std::shared_ptr<Train> train;
        std::shared_ptr<TrainTag> tag;
        TrainContext* const cont;

    public:
		AppendTagToTrain(std::shared_ptr<Train> train_, std::shared_ptr<TrainTag> tag_, TrainContext* cont, QUndoCommand* parent = nullptr);

		void undo()override;
		void redo()override;
    };

    class RemoveTagFromTrain : public QUndoCommand
    {
        std::shared_ptr<Train> train;
        int index;
        std::shared_ptr<TrainTag> tag;
        TrainContext* const cont;
    public:
        RemoveTagFromTrain(std::shared_ptr<Train> train_, int index_, 
			std::shared_ptr<TrainTag> tag_, TrainContext* cont, QUndoCommand* parent = nullptr);

		void undo()override;
		void redo()override;
    };

    class ChangeTagName : public QUndoCommand
    {
        TrainTagManager& manager;
        std::shared_ptr<TrainTag> tag, data;
        TrainContext* const cont;
    public:
        ChangeTagName(TrainTagManager& manager_, std::shared_ptr<TrainTag> tag, std::shared_ptr<TrainTag> data,
            TrainContext* cont, QUndoCommand* parent = nullptr);

        void undo()override;
        void redo()override;
    private:
        void commit();
    };

    class ChangeTagNote : public QUndoCommand
    {
        TrainTagManager& manager;
        std::shared_ptr<TrainTag> tag, data;
        TrainContext* const cont;
    public:
        ChangeTagNote(TrainTagManager& manager_, std::shared_ptr<TrainTag> tag, std::shared_ptr<TrainTag> data,
            TrainContext* cont, QUndoCommand* parent = nullptr);

        void undo()override;
        void redo()override;
    };

    class BatchRemoveTagFromTrains : public QUndoCommand
    {
    public:
        using data_t = std::vector<std::pair<std::shared_ptr<Train>, int>>;

    private:
        std::shared_ptr<TrainTag> tag;
        data_t data;
        TrainContext* const cont;

    public:
        BatchRemoveTagFromTrains(std::shared_ptr<TrainTag> tag, data_t data, TrainContext* cont, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    /**
     * Delete the train tag from the manager. Also remove this tag from ALL trains.
     * The later is implemented using sub-command.
     */
    class DeleteTrainTag : public QUndoCommand
    {
        TrainCollection& coll;
        std::shared_ptr<TrainTag> tag;
        TrainContext* const cont;
    public:
        DeleteTrainTag(std::shared_ptr<TrainTag> tag, TrainCollection& coll, TrainContext* cont, QUndoCommand* parent = nullptr);

        void undo()override;
        void redo()override;
    };

    class BatchAddTagToTrains : public QUndoCommand
    {
        std::shared_ptr<TrainTag> tag;
        std::vector<std::shared_ptr<Train>> trains;
        TrainContext* const cont;
    public:
        BatchAddTagToTrains(std::shared_ptr<TrainTag> tag, std::vector<std::shared_ptr<Train>> trains,
            TrainContext* cont, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };
}

#endif
