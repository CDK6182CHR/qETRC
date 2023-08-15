#pragma once

#ifndef QETRC_MOBILE_2

#include <QObject>
#include <QUndoCommand>

class SARibbonLineEdit;
class TrainPath;
class MainWindow;
class SARibbonContextCategory;
class Diagram;
class PathEdit;
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

    void onPathRemoved(TrainPath* path);
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

}

#endif

