#pragma once

#ifndef QETRC_MOBILE_2
#include <QDialog>
#include <QUndoCommand>
#include "data/diagram/config.h"

class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;
class QPushButton;
class DiagramPage;

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
    const bool forDefault;
    bool repaint=false;
    const std::shared_ptr<DiagramPage> page;

    QSpinBox* spStartHour, * spEndHour, * spMinMarkInter;
    QDoubleSpinBox* sdVLineBold, * sdVLineSecond, * sdVLineThird;
    QSpinBox* spMarginUp, * spMarginDown, * spRulerWidth,
        * spMileWidth, * spHWhite, * spStationWidth, * spInterval, * spCountWidth;
    QSpinBox *spValidWidth,*spStartLabelHeight,*spEndLabelHeight,
            *spBaseHeight,*spStepHeight;
    QDoubleSpinBox* sdScaleX, * sdSlimWidth, * sdBoldWidth, *sdScaleYdist,
            *sdScaleYsec;
    QSpinBox* spBoldLevel, * spShowLevel;
    QComboBox* cbShowTimeMark, * cbVLineStyle;
    QCheckBox *ckFullName,*ckEndLabel,*ckAvoidCollid;
    QCheckBox* ckHideStartLabelStarting, * ckHideStartLabelNonStarting;
    QCheckBox* ckHideEndLabelTerminal, * ckHideEndLabelNonTerminal;
    QCheckBox* ckShowRuler, * ckShowMile, * ckShowCount; 
    QCheckBox* ckTransparent;
    QPushButton* btnGridColor, * btnTextColor;

    QColor gridColor, textColor;
    

public:
    ConfigDialog(Config& cfg,bool forDefault_, QWidget* parent=nullptr);
    ConfigDialog(Config& cfg, const std::shared_ptr<DiagramPage>& page,
        QWidget* parent = nullptr);
    auto& config(){return _cfg;}

signals:

    /**
     * @brief repaintDiagrams
     * 并不是所有的修改都会触发重新绘制。如果需要重新绘图，发射此信号
     */
    void repaintDiagrams();

    void onConfigApplied(Config& cfg, const Config& newcfg, bool repaint, bool forDefault);
    void onPageConfigApplied(Config& cfg, const Config& newcfg, bool repaint,
        std::shared_ptr<DiagramPage>page);

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

    void actOk();

    void onAvoidCollidChanged(bool on);

    void actGridColor();

    void actTextColor();

    void informTransparent();
};

class ViewCategory;
class MainWindow;

namespace qecmd {

    class ChangeConfig:public QUndoCommand {
    protected:
        Config& cfg;
        Config newcfg;
        const bool repaint;
        const bool forDefault;
        ViewCategory* const cat;
    public:
        ChangeConfig(Config& cfg_, const Config& newcfg_, bool repaint_, bool forDefault_,
            ViewCategory* cat_,
            QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    /**
     * 2021.10.08
     * 修改页面的config，区别是后续操作只包含指定页面的重绘
     */
    class ChangePageConfig :public ChangeConfig {
    protected:
        std::shared_ptr<DiagramPage> page;
    public:
        ChangePageConfig(Config& cfg_, const Config& newcfg_, bool repaint_, 
            std::shared_ptr<DiagramPage> page, ViewCategory* cat_,
            QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    /**
     * 修改页面比例。
     * 实质性操作都一样，只是增加操作合并
     */
    class ChangePageScale : public ChangePageConfig {
        static constexpr int ID = 102;
    public:
        ChangePageScale(Config& cfg_, const Config& newcfg_, bool repaint_,
            std::shared_ptr<DiagramPage> page, ViewCategory* cat_,
            QUndoCommand* parent = nullptr);

        virtual int id()const override { return ID; }

        virtual bool mergeWith(const QUndoCommand* cmd)override;

    };

    /**
     * 更改最大跨越站数。目前由MainWindow中的Ribbon操作控制。
     */
    class ChangePassedStation :public QUndoCommand {
        int valueold, valuenew;
        MainWindow* const mw;
    public:
        ChangePassedStation(int vold, int vnew,MainWindow* mw_,
            QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("更改最大跨越站数为")+QString::number(vnew),parent),
            valueold(vold),valuenew(vnew),mw(mw_)
        {}

        virtual void undo()override;
        virtual void redo()override;
    };
}

#endif
