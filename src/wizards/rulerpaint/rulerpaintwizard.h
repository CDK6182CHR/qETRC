﻿#pragma once
#include <QWizard>
#include <list>
#include <QPointer>
#include <map>

#include "data/common/qeglobal.h"   // for meta-type decl

class TrainStation;
class Train;
//#include "rulerpaintpagestart.h"
//#include "rulerpaintpagestation.h"
//#include "rulerpaintpagetable.h"

class Diagram;
class MainWindow;
class RulerPaintPageStart;
class RulerPaintPageStation;
class RulerPaintPageTable;
class DiagramWidget;
struct AdapterStation;
class PaintStationInfoWidget;
class RailStation;

/**
 * @brief The RulerPaintWizard class
 * pyETRC.rulerPainter
 * 标尺排图向导，改用QWizard实现
 */
class RulerPaintWizard : public QWizard
{
    Q_OBJECT;
    Diagram& diagram;
    RulerPaintPageStart* pgStart;
    RulerPaintPageStation* pgStation;
    RulerPaintPageTable* pgTable;

    std::shared_ptr<const Train> trainRef;   // 起始页选择的车次  作为参考，不可修改
    std::shared_ptr<Train> trainTmp;         // 整合出来的临时车次
    std::list<TrainStation>::iterator itrStart, itrEnd;      // 注意这是tmp里面的迭代器，只有调整排图启用

    std::map<const RailStation*, QPointer<PaintStationInfoWidget>> infoWidgets;
public:
    /**
     * 存储数据的field名称
     */
//    static const QString fRail,fRuler,fDir,fAnchorStation;

    RulerPaintWizard(Diagram& diagram_, QWidget* parent=nullptr);

    /**
     * 中途return可以打断施法
     * 询问是否确认退出。退出前清理运行线。
     */
    virtual void reject()override;

    /**
     * 铺画完毕
     * 清理临时运行线，然后执行添加或者修改列车的操作
     */
    virtual void accept()override;
protected:
    virtual void initializePage(int id)override;

    /**
     * 如果退出铺画页面，需要清理掉临时运行线
     */
    virtual void cleanupPage(int id)override;

private:
    void initUI();

    void resetTmpTrain();

    void updateInfoWidget(PaintStationInfoWidget* w);
    void updateInfoWidgets();
    void clearInfoWidgets();

signals:
    void removeTmpTrainLine(const Train& train);
    void paintTmpTrainLine(std::shared_ptr<Train> train);

    /**
     * 铺画新列车模式下，接受铺画结果；直接接受trainTmp作为结果
     */
    void trainAdded(std::shared_ptr<Train> train);

    void trainTimetableModified(std::shared_ptr<Train> train, std::shared_ptr<Train> table);

    void trainInfoModified(std::shared_ptr<Train> train, std::shared_ptr<Train> info);

private slots:

    /**
     * 铺画临时运行线。入参为Table页面传过来的表。
     * table中的时刻表被移动到tmpTrain中
     */
    void updateTrainLine(std::shared_ptr<Train> table);

    /**
     * 2023.08.27  Try to find possible railway as default for the pageStation
     */
    void setDefaultRailway();

    /**
     * pgStation的当前线路变化触发
     * 设置默认的Anchor信息，根据已有的车次信息。
     */
    void setDefaultAnchorStation(); 

public slots:
    void onPaintingPointClicked(DiagramWidget* d, std::shared_ptr<Train> train, AdapterStation* st);
};

