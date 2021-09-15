#pragma once
#include <QWidget>
#include <QUndoCommand>
#include <memory>
#include <deque>


class QUndoStack;
class RailCategory;
class QMenu;
class Railway;
class Forbid;
class QTreeView;
class RailDBModel;
class RailDB;
namespace navi {
class AbstractComponentItem;
}


/**
 * @brief The RailDBNavi class
 * 线路数据库的导航部分。将主要的操作逻辑尽量封装在这里。
 */
class RailDBNavi : public QWidget
{
    Q_OBJECT
    using ACI=navi::AbstractComponentItem;

    std::shared_ptr<RailDB> _raildb;
    RailDBModel* const model;

    QTreeView* tree;
    QMenu* meRail,*meCat;

    QUndoStack* _undo;
    bool _changed=false;
public:
    explicit RailDBNavi(std::shared_ptr<RailDB> raildb, QWidget *parent = nullptr);
    auto* getTree() { return tree; }
    auto* getModel() { return model;}
    auto* undoStack() { return _undo; }
private:
    void initUI();
    void initContext();

    /**
     * 当前选中的线路。如果当前不是线路，返回空
     */
    std::shared_ptr<Railway> currentRailway();
    std::shared_ptr<RailCategory> currentCategory();
    ACI* currentItem();
    
    void clearDBUnchecked();
    void afterResetDB();
    bool saveQuestion();

signals:
    void focusInRailway(std::shared_ptr<Railway>, const std::deque<int>& idx);
    void exportRailwayToDiagram(std::shared_ptr<Railway>);

    /**
     * @brief changedFlagChanged
     * _changed记号变更的通知。
     */
    void changedFlagChanged(bool);
    void dbReset();
private slots:
    void showContextMenu(const QPoint& pos);
    void actNewRail();
    void actNewSubcat();
    void actNewParallelCat();
    
    void actRemoveRail();
    void actRuler();
    void actForbid();
    void actExportToDiagram();
    
    void onCurrentChanged(const QModelIndex& cur, const QModelIndex& prev);
    void markChanged();
    void markUnchanged();

    void actExpand();
    void actCollapse();

    /**
     * 天窗编辑。直接更新，无需后续操作。操作压栈
     */
    void actChangeForbid(std::shared_ptr<Forbid> forbid, std::shared_ptr<Railway> data);


public slots:
    void actEditRail();
    void refreshData();
    void actOpen();
    void actNewDB();
    void actSave();
    void actSaveAs();
    bool openDB(const QString& filename);
    void actSetAsDefaultFile();

};


namespace qecmd {
    class RemoveRailDB :public QUndoCommand 
    {
        std::shared_ptr<Railway> railway;
        std::deque<int> path;
        RailDBModel* const model;
    public:
        RemoveRailDB(std::shared_ptr<Railway> railway_, const std::deque<int>& path_,
            RailDBModel* model_, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class InsertRailDB :public QUndoCommand
    {
        std::shared_ptr<Railway> railway;
        std::deque<int> path;
        RailDBModel* const model;
    public :
        InsertRailDB(std::shared_ptr<Railway> railway_, const std::deque<int>& path_,
            RailDBModel* model_, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    class UpdateForbidDB :public QUndoCommand {
        std::shared_ptr<Forbid> forbid;
        std::shared_ptr<Railway> data;
    public:
        UpdateForbidDB(std::shared_ptr<Forbid> forbid, std::shared_ptr<Railway> data,
            QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };
}
