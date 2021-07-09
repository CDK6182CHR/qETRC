#pragma once

#include <QObject>
#include <memory>
#include <SARibbonContextCategory.h>

#include "data/diagram/diagram.h"
#include "data/diagram/diagrampage.h"

class MainWindow;

/**
 * @brief The PageContext class
 * 运行图页面的代理
 */
class PageContext : public QObject
{
    Q_OBJECT;
    Diagram& diagram;
    MainWindow*const mw;
    std::shared_ptr<DiagramPage> page{};
    SARibbonContextCategory*const cont;

public:
    explicit PageContext(Diagram& diagram_,SARibbonContextCategory* context,
                         MainWindow* mw);

    void setPage(std::shared_ptr<DiagramPage> page);
    void resetPage();
    auto getPage(){return page;}
    void refreshData();
    auto context() { return cont; }

private:
    void initUI();

private slots:
    void actRemovePage();

signals:
    void pageRemoved(int i);
};

