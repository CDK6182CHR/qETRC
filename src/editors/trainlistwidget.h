#pragma once

#include <QWidget>
#include <QTableView>
#include <QString>
#include <QLineEdit>
#include <QUndoCommand>
#include <QList>

#include "data/train/traincollection.h"
#include "model/train/trainlistmodel.h"

class MainWindow;

/**
 * 和undo/redo有关的命令放在qecmd命名空间以回避冲突
 */
namespace qecmd {

    /**
     * 删除一组列车。在TrainListWidget中调用。
     * 暂定持有TrainCollection的引用，undo/redo有权限执行添加删除操作。
     * 注意indexes应当排好序，并且和trains一一对应！
     */
    class RemoveTrains :
        public QUndoCommand
    {
        QList<std::shared_ptr<Train>> _trains;
        QList<int> _indexes;
        TrainCollection& coll;

        /**
         * 如果持有MainWindow指针，则在Undo, redo时执行相应的界面操作
         */
        MainWindow* mw = nullptr;
        bool first = true;
    public:
        RemoveTrains(const QList<std::shared_ptr<Train>>& trains,
            const QList<int>& indexes, TrainCollection& coll_,
            MainWindow* mw,
            QUndoCommand* parent = nullptr);

        virtual void undo()override;

        /**
         * 注意push操作会执行这个函数！
         * 因为TrainListWidget必须保证删除按钮是有效的（无论是否有Slot接受这个CMD）
         * 所以第一次的redo不能在这里做。置标志位。
         */
        virtual void redo()override;

        const auto& trains()const { return _trains; }
        auto& trains() { return _trains; }

        void setMainWindow(MainWindow* d) { mw = d; }
    };
}

/**
 * @brief The TrainListWidget class
 * pyETRC.TrainWidget
 * 列车列表  主体是一个类似只读的ListView
 * 暂时直接套ListView实现
 * 注意尽量不要用Diagram的API，方便以后用到列车数据库中去
 */
class TrainListWidget : public QWidget
{
    Q_OBJECT

    TrainCollection& coll;
    QTableView* table;
    QLineEdit* editSearch;
    TrainListModel* model;

public:
    explicit TrainListWidget(TrainCollection& coll_, QWidget *parent_ = nullptr);
    void refreshData();

private:
    void initUI();
    

signals:
    void currentTrainChanged(std::shared_ptr<Train> train);
    void editTrain(std::shared_ptr<Train> train);
    void trainShowChanged(std::shared_ptr<Train> train, bool show);
    void addNewTrain();

    /**
     * 列车删除后发送信号给MainWindow
     * 注意不能传cmd，否则如果没人接受，就内存泄漏了
     */
    void trainsRemoved(const QList<std::shared_ptr<Train>>& trains, const QList<int>& indexes);

    /**
     * 列车发生排序，但没有增删改
     */
    void trainReordered();

private slots:
    void searchTrain();
    void editButtonClicked();
    void batchChange();

    /**
     * 要做的事：
     * （1）删除运行线；
     * （2）从Diagram中删除数据；（这个由Main负责。这里没这个权限）
     * （3）创建CMD对象，通知Main发生了删除事件；
     * （4）暂定由Main负责发起更新界面数据的操作。
     */
    void removeTrains();
};


