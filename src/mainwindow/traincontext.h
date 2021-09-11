#pragma once

#include <QObject>
#include <memory>
#include <QColor>
#include <deque>
#include <QUndoCommand>

#include <data/train/train.h>

class SARibbonContextCategory;
class SARibbonLineEdit;
class SARibbonComboBox;
class SARibbonCheckBox;
class SARibbonToolButton;
class QDoubleSpinBox;
class Diagram;
class PenStyleCombo;
class BasicTrainWidget;
class Routing;
class TrainType;
namespace qecmd {
    struct StartingTerminalData;
}
namespace ads {
class CDockWidget;
}


class MainWindow;
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

    SARibbonLineEdit* edName, * edStart, * edEnd, * edRouting;

    //后缀m表示可修改的控件
    SARibbonLineEdit *edNamem,*edStartm,*edEndm;
    SARibbonLineEdit *edNameDown,*edNameUp;
    SARibbonComboBox* comboType;
    PenStyleCombo* comboLs;
    SARibbonCheckBox *checkPassen;
    SARibbonToolButton* btnColor, * btnAutoUI, * btnToRouting;
    QDoubleSpinBox* spWidth;
    QColor tmpColor;

    QList<BasicTrainWidget*> basicWidgets;
    QList<ads::CDockWidget*> basicDocks;
public:
    TrainContext(Diagram& diagram_, SARibbonContextCategory* const context_, MainWindow* mw);
    auto* context() { return cont; }
    auto getTrain() { return train; }
    void resetTrain();

signals:
    void highlightTrainLine(std::shared_ptr<Train> train);
    void timetableChanged(std::shared_ptr<Train> train);
    void actRemoveTrain(int index);
    void focusInRouting(std::shared_ptr<Routing>);
    
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

    /**
     * 由所给dock查找下标，用于关闭面板时删除
     */
    int getBasicWidgetIndex(ads::CDockWidget* dock);

    /**
     * 更新指定车次打开的时刻表窗口。未见得是当前车次。
     */
    void updateTrainWidget(std::shared_ptr<Train> t);

    void updateTrainWidgetTitles(std::shared_ptr<Train> t);

public slots:
    void setTrain(std::shared_ptr<Train> train_);

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
     * 自动始发终到的执行：执行始发终到站变更；铺画运行线；更新TrainList和相关Widget
     */
    void commitAutoStartingTerminal(qecmd::StartingTerminalData& data);

    /**
     * 自动列车类型的执行：类型变更；铺画运行线；更新TrainList和相关Current
     */
    void commitAutoType(std::deque<std::pair<std::shared_ptr<Train>, 
        std::shared_ptr<TrainType>>>& data);

    void actInterpolation(const QVector<std::shared_ptr<Train>>& trains,
        const QVector<std::shared_ptr<Train>>& data);

    void commitInterpolation(const QVector<std::shared_ptr<Train>>& trains,
        const QVector<std::shared_ptr<Train>>& data);

private slots:
    void showTrainEvents();
    void actShowTrainLine();

    void onTrainDockClosed();

    void removeBasicDockAt(int idx);

    void onFullNameChanged();

    void onAutoUIChanged(bool on);

    void actSelectColor();

    void actApply();

    void refreshData();

    void actRemoveCurrentTrain();

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

    /**
     * 当列车发生大范围变化时，更新当前context, 以及速览时刻、速览信息等窗口，
     * 以及所有的BasicWidget
     * 不管当前列车有没有改动。
     * 注意不要重复执行这个操作。
     */
    void refreshCurrentTrainWidgets();
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
}

