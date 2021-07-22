#pragma once

#include <QObject>
#include <QUndoCommand>
#include <SARibbonContextCategory.h>

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
public:
    explicit RulerContext(Diagram& diagram, SARibbonContextCategory* context, MainWindow* mw_);

    void setRuler(std::shared_ptr<Ruler> ruler);
    auto getRuler(){return ruler;}
    void refreshData();
    auto* context() { return cont; }

private:
    void initUI();


signals:

    /**
     * 仅在排图标尺被更新时，通知重新铺画运行图
     */
    void ordinateRulerModified(Railway& rail);

public slots:

    void actChangeRulerData(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> nr);

    /**
     * 标尺的diff或者数据变化。操作已经执行完毕，这里只管执行后续变化
     */
    void commitRulerChange(std::shared_ptr<Ruler> ruler);

};


namespace qecmd {
    class UpdateRuler :public QUndoCommand {
        std::shared_ptr<Ruler> ruler;
        std::shared_ptr<Railway> nr;
        RulerContext* cont;
    public:
        UpdateRuler(std::shared_ptr<Ruler> ruler_,std::shared_ptr<Railway> nr_,
            RulerContext* context, QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("更新标尺数据: ")+ruler_->name()),
            ruler(ruler_),nr(nr_), cont(context){}
        virtual void undo()override;
        virtual void redo()override;
    };
}

