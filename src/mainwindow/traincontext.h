#pragma once

#include <SARibbonContextCategory.h>
#include <memory>

#include "data/diagram/diagram.h"

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
public:
    TrainContext(Diagram& diagram_, SARibbonContextCategory* const context_, MainWindow* mw);
    auto* context() { return cont; }
    
private:
    void initUI();

public slots:
    void setTrain(std::shared_ptr<Train> train_);

private slots:
    void showTrainEvents();
};

