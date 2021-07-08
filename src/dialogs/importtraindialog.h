#pragma once

#include <QDialog>
#include <QTableView>
#include <QLineEdit>
#include <QCheckBox>
#include "data/diagram/diagram.h"
#include "data/train/traincollection.h"
#include "model/train/trainlistmodel.h"
#include "editors/trainlistwidget.h"
#include "util/buttongroup.hpp"

/**
 * @brief The ImportTrainDialog class  导入车次对话框
 * pyETRC.ImportTrainDialog
 */
class ImportTrainDialog : public QDialog
{
    Q_OBJECT;

    /**
     * @brief coll  注意这个导入的过程针对TrainCollection进行
     * coll是当前图的，用引用；other是导入的，用值类型
     */
    Diagram& diagram;
    TrainCollection other;
    TrainListWidget* widget;
    TrainListModel* model;

    QCheckBox* ckLocal;
    QLineEdit* edFile, * edPrefix, * edSuffix;
    RadioButtonGroup<2,QVBoxLayout>* rdConflict;
    RadioButtonGroup<3,QVBoxLayout>* rdRouting;

public:
    ImportTrainDialog(Diagram& diagram_, QWidget* parent);

private:
    void initUI();

signals:
    /**
     * 导入线路结束，通告铺画运行线
     */
    void trainsImported();

private slots:
    void actView();

    /**
     * 暂时直接照写原来Python版本的实现。
     * 直接在这里实现添加，然后通知主窗口更新
     * 目前看来，列车时刻表与别的东西耦合不严重，因此提交时直接交换时刻表对象，然后重新绑定即可
     */
    void actApply();
    void actCancel();
};


