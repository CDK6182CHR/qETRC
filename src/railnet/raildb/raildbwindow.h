#pragma once
#include <QMainWindow>
#include <memory>

class RailStationWidget;
class RailDBNavi;
class QUndoStack;
class RailDB;
class Railway;

class RailDBWindow : public QMainWindow
{
    Q_OBJECT
    const std::shared_ptr<RailDB> _raildb;

    RailDBNavi* navi;
    RailStationWidget* editor;

public:
    explicit RailDBWindow(QWidget *parent = nullptr);
    auto* getNavi(){return navi;}
    auto railDB() { return _raildb; }
private:
    void initUI();
    void initMenuBar();
signals:
    void exportRailwayToDiagram(std::shared_ptr<Railway>);
private slots:
    void afterResetDB();
    void updateWindowTitle(bool changed);
    void onNaviRailChanged(std::shared_ptr<Railway> railway);

};

