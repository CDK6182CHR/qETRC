#pragma once

#ifndef QETRC_MOBILE_2

#include <set>
#include <QObject>
#include <QUndoCommand>

class QLineEdit;
class Train;
class TrainPath;
class MainWindow;
class SARibbonContextCategory;
class Diagram;
class PathEdit;
class PathListWidget;
class PathRuler;

namespace ads {
    class CDockWidget;
}
namespace qecmd {
    struct TrainInfoInPath;
}

class PathContext : public QObject
{
    Q_OBJECT
    Diagram& diagram;
    SARibbonContextCategory* const cont;
    MainWindow* const mw;
    TrainPath* path=nullptr;

    QLineEdit* edName;

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

    void actAddPathRuler();
    void actSwitchToPathRuler();

    void actShowTrains();
    void actAddTrains();

    void actChangePath();

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

    void afterPathTrainsChanged(TrainPath* path, const std::vector<qecmd::TrainInfoInPath>& trains);

    void afterPathTrainsChanged(TrainPath* path, const QList<std::shared_ptr<Train>>& trains);

    void removePath(int idx);

    void actRemoveTrains(TrainPath* path, const std::set<int>& indexes);

    /**
     * Add train called by the dialog.
     * Here, the value-type of QList is used for safety. The efficiency should be acceptable since 
     * QList is an implicitly-shared class.
     */
    void addTrains(TrainPath* path, QList<std::shared_ptr<Train>> trains);

    void insertRulerAt(TrainPath* path, int idx, std::shared_ptr<PathRuler> ruler);
    void removeRulerAt(TrainPath* path, int idx);

    void actEditPathRuler(std::shared_ptr<PathRuler> ruler);
    void actRemovePathRuler(std::shared_ptr<PathRuler> ruler);
    void actDuplicatePathRuler(std::shared_ptr<PathRuler> ruler);

    /**
     * Called after the PathRuler is updated (this is actually performed in the cmd)
     */
    void onPathRulerUpdated(std::shared_ptr<PathRuler> ruler);
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

    struct TrainInfoInPath {
        std::weak_ptr<Train> train;
        int train_index_in_path;
        int path_index_in_train;
    };

    /**
     * 2023.08.24  Remove some trains from the path (but not necessarily all trains)
     */
    class RemoveTrainsFromPath : public QUndoCommand
    {
        TrainPath* path;
        std::vector<TrainInfoInPath> data;
        PathContext* const cont;
    public:
        RemoveTrainsFromPath(TrainPath* path, std::vector<TrainInfoInPath>&& data_,
            PathContext* cont, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class AddTrainsToPath : public QUndoCommand {
        TrainPath* path;
        QList<std::shared_ptr<Train>> trains;
        PathContext* const cont;
    public:
        AddTrainsToPath(TrainPath* path, QList<std::shared_ptr<Train>>&& trains_,
            PathContext* cont, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class AddPathRuler : public QUndoCommand {
        PathContext* m_cont;
        std::shared_ptr<PathRuler> m_ruler;
        int m_index;
    public:
        AddPathRuler(PathContext* context, std::shared_ptr<PathRuler> ruler, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    class RemovePathRuler : public QUndoCommand {
        PathContext* m_cont;
        std::shared_ptr<PathRuler> m_ruler;
        int m_index;
    public:
        RemovePathRuler(PathContext* context, std::shared_ptr<PathRuler> ruler, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    class UpdatePathRuler : public QUndoCommand {
        PathContext* m_cont;
        std::shared_ptr<PathRuler> m_ruler, m_data;
    public:
        UpdatePathRuler(PathContext* context, std::shared_ptr<PathRuler> ruler, std::shared_ptr<PathRuler> data,
            QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

}

#endif

