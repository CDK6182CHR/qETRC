#pragma once

#ifndef QETRC_MOBILE_2
#include <QObject>
#include <QDialog>
#include <memory>
#include <QUndoCommand>

class QTextEdit;
class QLineEdit;
class SARibbonContextCategory;
class DiagramPage;
class Diagram;
class MainWindow;
class Railway;

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

    /**
     * 选择当前运行图包含的线路。弹出对话框。
     */
    std::shared_ptr<Railway> selectRailway();

private slots:
    void actRemovePage();

    void actEdit();

    void actPrint();

    void actConfig();

    void actActivatePage();

    // 使用当前运行图的Config，操作压栈
    void actUseDiagramConfig();

    /**
     * 允许修改线路表的更新操作；相当于新建了。
     * 但是Page的地址保持不变。
     */
    void actResetPage();

    void onResetApplied(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> data);

    void actDulplicate();

    void hExpand();

    void hShrink();

    void vExpand();

    void vShrink();

    void actChangePage();

public slots:
    void onEditApplied(std::shared_ptr<DiagramPage> page,std::shared_ptr<DiagramPage> newinfo);

    void commitEditInfo(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> newinfo);

    /**
     * 2021.10.15
     * 与EditInfo的区别是，多一个重绘
     */
    void commitResetPage(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> newinfo);

    void actEditRailway();

    void actSwitchToRailway();

    /**
     * 2024.08.08  Automatically set top/down margins according to labels.
     */
    void actAutoTopDownMargin();

    void actResetPageFor(std::shared_ptr<DiagramPage> page);

signals:
    void pageRemoved(int i);
    void pageNameChanged(int index);
    void dulplicatePage(std::shared_ptr<DiagramPage>);
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


    class ResetPage : public QUndoCommand 
    {
        std::shared_ptr<DiagramPage> page, data;
        PageContext* const cont;
    public:
        ResetPage(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> newpage,
            PageContext* context, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };
}

#endif
