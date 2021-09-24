#pragma once

#include <QObject>

#include "railnet/graph/railnet.h"

class RailDB;
class SARibbonContextCategory;
class MainWindow;
class RailDBWindow;
class RailDBNavi;
namespace ads {
class CDockWidget;
}

/**
 * @brief The RailDBContext class
 * 线路数据库的context，负责管理RailDB的一切，
 * 取代RailWindow。
 * 启动时，不初始化数据库。尽量不使用Diagram的接口。
 */
class RailDBContext : public QObject
{
    Q_OBJECT
    SARibbonContextCategory*const cont;
    MainWindow* const mw;
    ads::CDockWidget* dock=nullptr;
    RailDBWindow* const window;   // 必须在raildb之前初始化！ 
    const std::shared_ptr<RailDB> _raildb;
    RailNet net;
    bool _active = false;
public:
    explicit RailDBContext(SARibbonContextCategory* cont,
                           MainWindow* mw_);
    auto* getWindow() { return window; }
    RailDBNavi* getNavi();

    bool deactiveOnClose();
private:
    void initUI();

signals:

private slots:
    void onWindowDeactivated();

public slots:
    /**
     * @brief activateDB
     * 进入db模式。包括显示当前context，读取打开文档。
     * 如果已经是active状态（_raildb指针非空），则显示dock
     */
    void activateDB();

    /**
     * @brief deactivateDB
     * 退出db模式：隐藏context；删除widget（但dock可以不删除）。
     */
    void deactivateDB();

    /**
     * 刷新net；从当前的RailDB提取数据
     */
    void loadNet();

};

