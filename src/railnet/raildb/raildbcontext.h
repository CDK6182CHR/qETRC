#pragma once

#include <QObject>

#include "railnet/graph/railnet.h"

class RailDB;
class SARibbonContextCategory;
class MainWindow;
class RailDBWindow;
class RailDBNavi;

class QuickPathSelector;

class RailPreviewDialog;
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
    ads::CDockWidget* dbDock = nullptr, * quickDock = nullptr, * pathDock = nullptr;
    RailDBWindow* const window;   // 必须在raildb之前初始化！
    QuickPathSelector* quickSelector=nullptr;  // 必须在net之前初始化
    const std::shared_ptr<RailDB> _raildb;
    RailNet net;
    bool _active = false;

    RailPreviewDialog* dlgPreview=nullptr;
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
     * 退出db模式：隐藏context；清空数据 （db和net）
     */
    void deactivateDB();

    /**
     * @brief activateQuickSelector
     * 启动快速经由选择器。
     * 如有必要，创建窗口和dock，读取db和net
     */
    void activateQuickSelector();

private:

    /**
     * 进入路网管理模式（显示Context），但不显示其他任何窗口。
     */
    void activateBase();

    /**
     * @brief loadDB  读取默认线路数据库文件
     * activate时调用；但这里不包含图形操作。
     */
    void loadDB();

    /**
     * 刷新net；从当前的RailDB提取数据
     */
    void loadNet();

signals:
    void exportRailToDiagram(std::shared_ptr<Railway> rail);

private slots:
    void previewRail(std::shared_ptr<Railway> railway, const QString& pathString);

    void actShowAdj();

    void actRefreshNet();
};

