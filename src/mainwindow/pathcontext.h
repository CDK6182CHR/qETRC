#pragma once

#ifndef QETRC_MOBILE_2

#include <QObject>
#include <QUndoCommand>

class SARibbonLineEdit;
class Train;
class TrainPath;
class MainWindow;
class SARibbonContextCategory;
class Diagram;
class PathEdit;
class PathListWidget;
namespace ads {
    class CDockWidget;
}

class PathContext : public QObject
{
    Q_OBJECT
    Diagram& diagram;
    SARibbonContextCategory* const cont;
    MainWindow* const mw;
    TrainPath* path=nullptr;

    SARibbonLineEdit* edName;

    //2023.08.14: trainPaths
    QList<PathEdit*> pathEdits;
    QList<ads::CDockWidget*> pathDocks;

public:
    explicit PathContext(Diagram& diagram, SARibbonContextCategory* cont,
                          MainWindow* mw, QObject *parent = nullptr);
    auto* context(){return cont;}

    void setPath(TrainPath* path);
    void refreshData();
    auto* getPath(){return path;}

private:
    void initUI();
    int pathEditIndex(const TrainPath* path);

signals:

private slots:
    void actEditPath();
    void actRemovePath();
    void actDuplicatePath();

    void actShowTrains();
    void actAddTrains();

public slots:

    void openPathEdit(TrainPath* path);

    /**
     * open editor for path by the index IN THE DATA CLASS.
     * The index is guarenteed to be valid.
     */
    void openPathEditIndex(int idx);

    void removePathDocks();

    /**
     * 操作压栈  更新列车径路
     */
    void actUpdatePath(TrainPath* path, std::unique_ptr<TrainPath>& data);

    void commitUpdatePath(TrainPath* path);

    void afterPathRemoved(TrainPath* path);

    void actClearTrains();

    void afterPathTrainsChanged(TrainPath* path, const std::vector<std::weak_ptr<Train>>& trains);

    void removePath(int idx);
};

namespace qecmd {
    class UpdatePath :public QUndoCommand {
        TrainPath* path;
        std::unique_ptr<TrainPath> data;
        PathContext* const cont;
    public:
        UpdatePath(TrainPath* path, std::unique_ptr<TrainPath>&& data_, PathContext* cont,
            QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class ClearTrainsFromPath :public QUndoCommand {
        TrainPath* path;
        std::vector<int> indexes_in_train;
        std::vector<std::weak_ptr<Train>> trains;
        PathContext* const cont;
    public:
        ClearTrainsFromPath(TrainPath* path, PathContext* cont, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    /**
     * 2023.08.16  move from pathlistwidget to pathcontext, and add removing trains
     */
    class RemoveTrainPath :public QUndoCommand
    {
        int idx;
        PathListWidget* const pw;
        std::unique_ptr<TrainPath> path;
        PathContext* const cont;
    public:
        // Note: the path pointer p is only used in the constructor
        RemoveTrainPath(TrainPath* p, int idx, PathListWidget* pw, PathContext* cont, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    /**
     * 2023.08.24  Remove some trains from the path (but not necessarily all trains)
     */
    class RemoveTrainsFromPath : public QUndoCommand
    {
        TrainPath* path;
        // ... 
        // 2023.08.24  TODO here
    };


}

#endif

