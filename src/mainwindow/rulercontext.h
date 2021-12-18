#pragma once

#include <QObject>
#include <QUndoCommand>
#include <QString>
#include <SARibbonContextCategory.h>
#include <QLineEdit>

#include "data/rail/rail.h"
#include "data/diagram/diagram.h"

class MainWindow;

/**
 * @brief The RulerContext class
 * 当前标尺的Context
 */
class RulerContext : public QObject
{
    Q_OBJECT;
    std::shared_ptr<Ruler> ruler{};
    Diagram& diagram;
    SARibbonContextCategory*const cont;
    MainWindow* const mw;
    QLineEdit* edRulerName, * edRailName;
public:
    explicit RulerContext(Diagram& diagram, SARibbonContextCategory* context, MainWindow* mw_);

    void setRuler(std::shared_ptr<Ruler> ruler);
    void resetRuler() { ruler.reset(); }
    auto getRuler(){return ruler;}
    void refreshData();
    void refreshAllData();
    auto* context() { return cont; }

private:
    void initUI();


signals:

    /**
     * 仅在排图标尺被更新时，通知重新铺画运行图
     */
    void ordinateRulerModified(std::shared_ptr<Railway> rail);

    /**
     * 标尺被删除，关闭面板
     */
    void focusOutRuler();

    void rulerNameChanged(std::shared_ptr<Ruler>);

public slots:

    void actChangeRulerData(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> nr);

    /**
     * 标尺的diff或者数据变化。操作已经执行完毕，这里只管执行后续变化
     */
    void commitRulerChange(std::shared_ptr<Ruler> ruler);

    void actChangeRulerName(std::shared_ptr<Ruler> ruler, const QString& name);

    /**
     * 已经在undo里面修改过了；这里负责进行后续修改：
     * （1）更新context；（2）更新相关的Widget；（3）更新相关的标题；（4）更新排图标尺那个表
     * 其中1自己就能完成，2~4交给RailContext
     */
    void commitChangeRulerName(std::shared_ptr<Ruler> ruler);

    /**
     * 删除完毕后调用
     */
    void commitRemoveRuler(std::shared_ptr<Ruler> ruler, bool isord);

    /**
     * 恢复完数据后再调用！
     */
    void undoRemoveRuler(std::shared_ptr<Ruler> ruler, bool isord);

    /**
     * 由Navi那边的context menu 调起的删除标尺命令
     */
    void actRemoveRulerNavi(std::shared_ptr<Ruler> ruler);


    void dulplicateRuler(std::shared_ptr<Ruler> ruler);

private slots:

    void actShowEditWidget();

    void actSetAsOrdinate();

    void actRemoveRuler();

    void actReadFromSingleTrain();

    /**
     * 创建当前标尺副本。
     * 先创建新标尺，再修订它。
     */
    void actDulplicateRuler();


};


namespace qecmd {
    class UpdateRuler :public QUndoCommand {
        std::shared_ptr<Ruler> ruler;
        std::shared_ptr<Railway> nr;
        RulerContext* cont;
    public:
        UpdateRuler(std::shared_ptr<Ruler> ruler_,std::shared_ptr<Railway> nr_,
            RulerContext* context, QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("更新标尺数据: ")+ruler_->name(),parent),
            ruler(ruler_),nr(nr_), cont(context){}
        virtual void undo()override;
        virtual void redo()override;
    };

    class ChangeRulerName :public QUndoCommand {
        std::shared_ptr<Ruler> ruler;
        QString name;
        RulerContext*const cont;
    public:
        ChangeRulerName(std::shared_ptr<Ruler> ruler_,const QString& name_,
            RulerContext* context,QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("更改标尺名称: ")+name_,parent),
            ruler(ruler_),name(name_),cont(context){}
        virtual void undo()override;
        virtual void redo()override;
    };

    class RemoveRuler :public QUndoCommand {
        std::shared_ptr<Ruler> ruler;    //这只是个头结点
        std::shared_ptr<Railway> data;
        bool isOrd;
        RulerContext* const cont;
    public:
        RemoveRuler(std::shared_ptr<Ruler> ruler_,
            std::shared_ptr<Railway> data_,
            bool ordinate, RulerContext* context,QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("删除标尺: ")+ruler_->name(),parent),
            ruler(ruler_),data(data_),isOrd(ordinate),cont(context){}
        virtual void undo()override;
        virtual void redo()override;

    };
}

