#pragma once

#include <QDialog>
#include <QTableView>
#include <QLineEdit>
#include <QItemSelection>
#include <QUndoCommand>

#include "data/diagram/diagram.h"
#include "data/diagram/diagrampage.h"
#include "model/diagram/railtablemodel.h"

class MainWindow;
namespace qecmd {
    class AddPage :public QUndoCommand {
        Diagram& diagram;
        std::shared_ptr<DiagramPage> page;
        MainWindow* mw;
    public:
        AddPage(Diagram& diagram_, std::shared_ptr<DiagramPage> page_,
            MainWindow* mw_, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };
}


/**
 * @brief The AddPageDialog class
 * 添加运行图视窗 （DiagramPage）的临时对话框
 * 暂定 临时创建，用完删除
 */
class AddPageDialog : public QDialog
{
    Q_OBJECT
    Diagram& diagram;
    RailTableModel* model;
    QTableView* table;
    QLineEdit* editName, * editPrev;
public:
    AddPageDialog(Diagram& diagram_, QWidget* parent=nullptr);

private:
    void initUI();

signals:
    void creationDone(std::shared_ptr<DiagramPage> page);

private slots:
    void okClicked();
    void preview(const QItemSelection& sel);
};

