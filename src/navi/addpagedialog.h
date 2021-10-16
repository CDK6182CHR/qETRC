#pragma once

#include <QDialog>
#include <QUndoCommand>


class NaviTree;
class Diagram;
class DiagramPage;
class RailListModel;
class QTableView;
class QLineEdit;
class QTextEdit;


namespace qecmd {
    class AddPage :public QUndoCommand {
        Diagram& diagram;
        std::shared_ptr<DiagramPage> page;
        NaviTree* navi;
    public:
        AddPage(Diagram& diagram_, std::shared_ptr<DiagramPage> page_,
            NaviTree* nv, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    /**
     * 虽然和AddPage没关系，但是属于对偶操作，放在一起 
     */
    class RemovePage :public QUndoCommand {
        Diagram& diagram;
        std::shared_ptr<DiagramPage> page;
        NaviTree* navi;
        int index;
    public:
        RemovePage(Diagram& diagram_, int index_,
            NaviTree* navi_, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };
}

#if 0

/**
 * @brief The AddPageDialog class
 * 添加运行图视窗 （DiagramPage）的临时对话框
 * 暂定 临时创建，用完删除
 */
class AddPageDialogV1 : public QDialog
{
    Q_OBJECT
    Diagram& diagram;
    RailTableModel* model;
    QTableView* table;
    QLineEdit* editName, * editPrev;
    QTextEdit* edNote;
    std::shared_ptr<DiagramPage> page = nullptr;   // 非空表示执行修改操作
public:
    AddPageDialogV1(Diagram& diagram_, QWidget* parent=nullptr);

    /**
     * 2021.10.15
     * 此版本提供为修订
     */
    AddPageDialogV1(Diagram& diagram_, std::shared_ptr<DiagramPage> page_,
        QWidget* parent = nullptr);

private:
    void initUI();

    /**
     * 用于reset情况调用；设置数据
     */
    void setData();

signals:
    void creationDone(std::shared_ptr<DiagramPage> page);
    void modificationDone(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> data);

private slots:
    void okClicked(); 
    void preview(const QItemSelection& sel);
};

#endif


/**
 * 2021.10.16  第二版的实现 重新设计界面
 */
class AddPageDialog : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    RailListModel* const mdUnsel, * const mdSel;

    QTableView* tbUnsel, * tbSel;

    QLineEdit* edName, * edPrev;
    QTextEdit* edNote;
    std::shared_ptr<DiagramPage> page = nullptr;   // 非空表示执行修改操作
public:

    AddPageDialog(Diagram& diagram_, QWidget* parent);

    /**
     * 此版本提供为修订
     */
    AddPageDialog(Diagram& diagram_, std::shared_ptr<DiagramPage> page_,
        QWidget* parent);

private:
    void initUI();

    /**
     * 用于reset情况调用；设置数据
     */
    void setData();

signals:
    void creationDone(std::shared_ptr<DiagramPage> page);
    void modificationDone(std::shared_ptr<DiagramPage> page, std::shared_ptr<DiagramPage> data);

private slots:
    void okClicked();

    void selectAll();
    void deselectAll();
    void moveUp();
    void moveDown();
    
    void select();
    void deselect();

};

