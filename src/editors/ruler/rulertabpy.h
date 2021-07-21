#pragma once

#include <QWidget>
#include "data/rail/rail.h"
#include "model/rail/rulermodel.h"

/**
 * @brief The RulerTabPy class
 * pyETRC风格的Ruler编辑面板，包含所有功能
 * Py后缀表示是按照原来pyETRC风格设计的面板，但以后可能会重新设计
 *
 * 暂时没实现
 */
class RulerTabPy : public QWidget
{
    Q_OBJECT;
    RulerModel* model;
    std::shared_ptr<Ruler> ruler;

    /**
     * @brief hasMain
     * 如果有，则可以执行从车次读取、设为排图标尺等功能
     */
    bool hasMain;
public:
    explicit RulerTabPy(std::shared_ptr<Ruler> ruler_, bool hasMain_,
                        QWidget *parent = nullptr);
    auto* getModel(){return model;}
    auto getRuler(){return ruler;}
    void refreshData();

private:
    void initUI();

signals:

};

