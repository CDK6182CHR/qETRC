#pragma once

#include <QWidget>
class QTableView;

/**
 * @brief The QEControlledTable class
 * 附带增删移动功能的TableView  类似pyetrc.utility.PEControlledTable
 */
class QEControlledTable : public QWidget
{
    Q_OBJECT
    QTableView* _table;
    bool doubleLine;
public:
    explicit QEControlledTable(QWidget *parent = nullptr, bool doubleLine=false);
    auto* table(){return _table;}

public slots:
    /**
     * @brief 2022.10.14 勾选框的单元格，一次性勾选/取消勾选/切换勾选选区内所有单元格
    */
    void toggleSelection();
    void checkSelection();
    void uncheckSelection();

private:
    void initUI();

signals:

private slots:
    void insertBefore();
    void insertAfter();
    void removeRow();
    void moveUp();
    void moveDown();

};
