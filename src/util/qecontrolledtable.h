#pragma once

#include <QWidget>
#include <QTableView>

/**
 * @brief The QEControlledTable class
 * 附带增删移动功能的TableView  类似pyetrc.utility.PEControlledTable
 */
class QEControlledTable : public QWidget
{
    Q_OBJECT
    QTableView* _table;
public:
    explicit QEControlledTable(QWidget *parent = nullptr);
    auto* table(){return _table;}

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
