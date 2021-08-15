#pragma once

#include <QObject>
#include <QDialog>
#include <QTextEdit>
#include <memory>
#include <SARibbonContextCategory.h>
#include <QLineEdit>
#include <QUndoCommand>

#include "data/diagram/diagram.h"
#include "data/diagram/diagrampage.h"

class MainWindow;

/**
 * @brief The PageContext class
 * 运行图页面的代理
 */
class PageContext : public QObject
{
    Q_OBJECT
    Diagram& diagram;
    MainWindow*const mw;
    std::shared_ptr<DiagramPage> page{};
    SARibbonContextCategory*const cont;

    QLineEdit* edName;

public:
    explicit PageContext(Diagram& diagram_,SARibbonContextCategory* context,
                         MainWindow* mw);

    void setPage(std::shared_ptr<DiagramPage> page);
    void resetPage();
    auto getPage(){return page;}
    void refreshData();
    void refreshAllData();
    auto context() { return cont; }

private:
    void initUI();

private slots:
    void actRemovePage();

    void actEdit();

    void actPrint();

public slots:
    void onEditApplied(std::shared_ptr<DiagramPage> page,std::shared_ptr<DiagramPage> newinfo);

    void commitEditInfo(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> newinfo);

signals:
    void pageRemoved(int i);
    void pageNameChanged(int index);
};


/**
 * 编辑基本信息，暂时只有名称和注记
 */
class EditPageDialog :public QDialog
{
    Q_OBJECT
    QLineEdit* edName;
    QTextEdit* edNote;
    std::shared_ptr<DiagramPage> page;
    Diagram& diagram;
public:
    EditPageDialog(std::shared_ptr<DiagramPage> page_,Diagram& dia, QWidget* parent = nullptr);

private:
    void initUI();
private slots:
    void actApply();
signals:
    void editApplied(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> newinfo);
};



namespace qecmd {

    class EditPageInfo :
        public QUndoCommand 
    {
        std::shared_ptr<DiagramPage> page, newpage;
        PageContext* const cont;
    public:
        EditPageInfo(std::shared_ptr<DiagramPage> page,std::shared_ptr<DiagramPage> newpage,
            PageContext* context, QUndoCommand* parent=nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };
}
