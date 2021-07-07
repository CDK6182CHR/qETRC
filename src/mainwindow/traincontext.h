#pragma once

#include <SARibbonContextCategory.h>
#include <memory>
#include <QLineEdit>
#include <DockWidget.h>

#include "data/diagram/diagram.h"
#include "editors/basictrainwidget.h"

class MainWindow;
/**
 * @brief The TrainContext class
 * 列车的ContextMenu。尝试从MainWindow分出来，是为了减少MainWindow代码量。
 * 把相关操作代理了，MainWindow只负责转发
 * 暂时按照QObject处理，但其实好像不是很必要
 */
class TrainContext : public QObject
{
    Q_OBJECT
    Diagram& diagram;
    std::shared_ptr<Train> train{};
    SARibbonContextCategory* const cont;
    MainWindow* const mw;

    QLineEdit* edName, * edStart, * edEnd;

    QList<BasicTrainWidget*> basicWidgets;
    QList<ads::CDockWidget*> basicDocks;
public:
    TrainContext(Diagram& diagram_, SARibbonContextCategory* const context_, MainWindow* mw);
    auto* context() { return cont; }
    auto getTrain() { return train; }
    void resetTrain();

signals:
    void highlightTrainLine(std::shared_ptr<Train> train);
    
private:
    void initUI();

    /**
     * 返回当前列车对应的BasicWidget的下标
     * 暂定线性查找
     * 如果找不到，返回-1
     */
    int getBasicWidgetIndex();

public slots:
    void setTrain(std::shared_ptr<Train> train_);

private slots:
    void showTrainEvents();
    void actShowTrainLine();

    /**
     * 显示或者创建当前车次的基本编辑面板。
     */
    void actShowBasicWidget();
};

