#pragma once

#include <QDialog>

class QEControlledTable;
class QTableView;
class QLabel;
class TypeManager;
class TypeConfigModel;
class TrainType;
class QCheckBox;

/**
 * @brief The TypeConfigDialog class
 * 变化应用规则：
 * （1）如果行没被删除（ColName列能找到Type的指针）,那么认为指定类发生了更新，
 * 存储TrainType对象的副本，执行数据交换。这些对象的地址没有变化。
 * （2）对于新增的行（ColName的Type指针为空），那么理论上不会有任何车次指向这里，直接进行添加。
 * （3）对于删除的行，如果存在车次指向这个类型，那么删除这个操作无效，直接从车次重新添加引用；
 * 如果不存在外部对这里的引用，那么删除操作成功，从列表中移除它。
 */
class TypeConfigDialog : public QDialog
{
    Q_OBJECT;
    TypeManager& manager;
    TypeConfigModel* const model;
    const bool forDefault;

    QEControlledTable* ctab;
    QTableView* table;
    QCheckBox* ckTransparent;

public:
    explicit TypeConfigDialog(TypeManager& manager_,bool forDefault_, QWidget *parent = nullptr);

private:
    void initUI();

signals:
    /**
     * 应用更改的信号，通告给viewCat进行处理。
     * @param data: 包含所有没被删除的数据，含新增的。
     * @param modified: 规则（1），即更改了的类型。通过swap来实施变更。
     */
    void typeSetApplied(TypeManager& manager,
                        const QMap<QString,std::shared_ptr<TrainType>>& types,
     const QVector<QPair<std::shared_ptr<TrainType>, std::shared_ptr<TrainType>>>& modified);

private slots:
    void actApply();
    void onDoubleClicked(const QModelIndex& idx);
    void informTransparent();
public slots:
    void refreshData();
};

