#pragma once
#include <QDialog>

#include "util/selectrailwaycombo.h"

class QCheckBox;
class Diagram;

/**
 * @brief The OutputSubDiagramDialog class
 * 导出单线路的运行图
 */
class OutputSubDiagramDialog : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;

    SelectRailwayCombo* cbRail;
    QCheckBox* ckLocal;
public:
    OutputSubDiagramDialog(Diagram& diagram_, QWidget*parent=nullptr);
private:
    void initUI();
private slots:
    void onOutputTrc();
    void onOutputPyetrc();
};

