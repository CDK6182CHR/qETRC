#pragma once

#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QUndoCommand>
#include "data/diagram/config.h"

/**
 * @brief The ConfigDialog class
 * pyETRC.ConfigWidget
 * 运行图配置项。暂时采用Dialog实现。
 */
class ConfigDialog : public QDialog
{
    Q_OBJECT

    /**
     * @brief _cfg
     * 暂定每个Dialog对象都是一次性的，只负责配置一个Config对象。
     * 目前只有全局的，但以后可能会允许每个Page分别配置。
     */
    Config& _cfg;
    bool repaint=false;

    QSpinBox* spStartHour, * spEndHour, * spVLines, * spMinMarkInter;
    QSpinBox *spMarginUp,*spMarginDown,*spRulerWidth,
            *spMileWidth,*spHWhite,*spStationWidth,*spInterval;
    QSpinBox *spValidWidth,*spStartLabelHeight,*spEndLabelHeight,
            *spBaseHeight,*spStepHeight;
    QDoubleSpinBox* sdScaleX, * sdSlimWidth, * sdBoldWidth, *sdScaleYdist,
            *sdScaleYsec;
    QComboBox* cbShowTimeMark;
    QCheckBox *ckFullName,*ckEndLabel,*ckAvoidCollid;

public:
    ConfigDialog(Config& cfg,QWidget* parent=nullptr);
    auto& config(){return _cfg;}

signals:

    /**
     * @brief repaintDiagrams
     * 并不是所有的修改都会触发重新绘制。如果需要重新绘图，发射此信号
     */
    void repaintDiagrams();

    void onConfigApplied(Config& cfg,const Config& newcfg,bool repaint);

private:
    void initUI();

private slots:
    void refreshData();

    /**
     * @brief actApply
     * 注意这里不实际执行；而是发送信号给MainWindow处理。
     * 将操作压栈。因为考虑到，撤销、重做发生时，此窗口可能已经无了
     */
    void actApply();

    void onAvoidCollidChanged(bool on);
};

class ViewCategory;

namespace qecmd {

    class ChangeConfig:public QUndoCommand {
        Config& cfg;
        Config newcfg;
        const bool repaint;
        ViewCategory* const cat;
    public:
        ChangeConfig(Config& cfg_, const Config& newcfg_, bool repaint_,ViewCategory* cat_,
            QUndoCommand* parent=nullptr):
        QUndoCommand(QObject::tr("更新显示设置"),parent),cfg(cfg_),newcfg(newcfg_),repaint(repaint_),
            cat(cat_)
        {}
        virtual void undo()override;
        virtual void redo()override;
    };
}

