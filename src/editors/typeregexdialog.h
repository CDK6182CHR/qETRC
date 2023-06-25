#pragma once

#include <QDialog>

class QLabel;
class QTableView;
class QEControlledTable;
class TypeManager;
class TypeRegexModel;
class QCheckBox;

class TypeRegexDialog : public QDialog
{
    Q_OBJECT;
    TypeManager& manager;
    TypeRegexModel* const model;
    const bool forDefault;

    QEControlledTable* ctab;
    QTableView* table;
    QCheckBox* ckTransparent;

public:
    TypeRegexDialog(TypeManager& manager_,bool forDefault, QWidget* parent=nullptr);

private:
    void initUI();
signals:
    void typeRegexApplied(TypeManager& manager, std::shared_ptr<TypeManager> data);   // 值类型
private slots:
    void actApply();
    void refreshData();
    void informTransparent();
};

