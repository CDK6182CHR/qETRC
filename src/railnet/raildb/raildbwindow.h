#pragma once
#include <QMainWindow>
#include <QUndoCommand>
#include <memory>
#include <QPersistentModelIndex>
#include <deque>

#include "data/rail/railinfonote.h"

class RailStationWidget;
class RailDBNavi;
class QUndoStack;
class RailDB;
class Railway;
class QUndoView;
class RailDBModel;

class RailDBWindow : public QMainWindow
{
    Q_OBJECT
    const std::shared_ptr<RailDB> _raildb;

    RailDBNavi* navi;
    RailStationWidget* editor;
    
    /**
     * 当前editor中编辑对象对应的index。必须保持一致。
     * 注意：增删数据时的有效性问题
     */
    std::deque<int> editorPath;

    QUndoView* undoView;

public:
    explicit RailDBWindow(QWidget *parent = nullptr);
    auto* getNavi(){return navi;}
    auto railDB() { return _raildb; }
    const auto& getEditingPath() { return editorPath; }
    
    /**
     * 直接转发给navi->deactive 
     */
    bool deactive();

private:
    void initUI();
    void initMenuBar();
signals:
    void exportRailwayToDiagram(std::shared_ptr<Railway>);
private slots:
    void afterResetDB();
    void updateWindowTitle(bool changed);
    void onNaviRailChanged(std::shared_ptr<Railway> railway,
                           const std::deque<int>& idx);

    void onEditorRailNameChanged(std::shared_ptr<Railway> railway, const QString& name);
    void onEditorStationChanged(std::shared_ptr<Railway> railway, std::shared_ptr<Railway> newtable,
        bool equiv);
    void onEditorInvalidApplied();
    void onEditorRailNoteChanged(std::shared_ptr<Railway> railway, const RailInfoNote& note);

public slots:

    /**
     * 操作已经执行。这里负责进行更新操作，主要包括editor和左边的navi
     */
    void commitUpdateStations(std::shared_ptr<Railway> railway, const std::deque<int>& path);
    void commitUpdateRailName(std::shared_ptr<Railway> railway, const std::deque<int>& path);

};

namespace qecmd {
    class UpdateRailStationsDB :public QUndoCommand
    {
        std::shared_ptr<Railway> railway, table;
        std::deque<int> path;
        RailDBWindow* const wnd;
    public:
        UpdateRailStationsDB(std::shared_ptr<Railway> railway_, std::shared_ptr<Railway> table_,
            const std::deque<int>& path_,
            RailDBWindow* wnd_, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class UpdateRailNameDB :public QUndoCommand {
        std::shared_ptr<Railway> railway;
        QString name;
        std::deque<int> path;
        RailDBWindow* const wnd;
    public:
        UpdateRailNameDB(std::shared_ptr<Railway> railway_, const QString& name_,
            const std::deque<int>& path_, RailDBWindow* wnd_, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };


    class UpdateRailNoteDB :public QUndoCommand {
        std::shared_ptr<Railway> railway;
        RailInfoNote data;
        std::deque<int> path;
        RailDBModel* const model;
    public:
        UpdateRailNoteDB(std::shared_ptr<Railway> railway, const RailInfoNote& data,
            const std::deque<int>& path, RailDBModel* model, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };
}

