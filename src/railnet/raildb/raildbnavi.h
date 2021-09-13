﻿#pragma once
#include <QWidget>
#include <memory>

class QUndoStack;
class RailCategory;
class QMenu;
class Railway;
class QTreeView;
class RailDBModel;
class RailDB;

/**
 * @brief The RailDBNavi class
 * 线路数据库的导航部分。将主要的操作逻辑尽量封装在这里。
 */
class RailDBNavi : public QWidget
{
    Q_OBJECT
    std::shared_ptr<RailDB> _raildb;
    RailDBModel* const model;

    QTreeView* tree;
    QMenu* meRail,*meCat;

    QUndoStack* _undo;
    bool _changed=false;
public:
    explicit RailDBNavi(std::shared_ptr<RailDB> raildb, QWidget *parent = nullptr);
private:
    void initUI();
    void initContext();

    /**
     * 当前选中的线路。如果当前不是线路，返回空
     */
    std::shared_ptr<Railway> currentRailway();
    std::shared_ptr<RailCategory> currentCategory();
    
    void clearDBUnchecked();
    void afterResetDB();
    bool saveQuestion();

signals:
    void focusInRailway(std::shared_ptr<Railway>);
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
    void actEditRail();
    void actRemoveRail();
    void actRuler();
    void actForbid();
    void actExportToDiagram();

    void onCurrentChanged(const QModelIndex& cur, const QModelIndex& prev);
    void markChanged();
    void markUnchanged();

public slots:
    void refreshData();
    void actOpen();
    void actNewDB();
    void actSave();
    void actSaveAs();
    bool openDB(const QString& filename);

};

