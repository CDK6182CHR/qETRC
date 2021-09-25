#pragma once

#include <QDialog>
#include <memory>

class QEControlledTable;
class QLineEdit;
class QTableView;
class RailStationModel;
class Railway;
class QTextBrowser;
class DialogAdapter;
/**
 * @brief The RailPreviewDialog class
 * 生成切片后的预览页面。全局类似单例的模式；展示时使用模态窗口。
 * 可以修改线名，径路表只能预览。
 */
class RailPreviewDialog : public QDialog
{
    Q_OBJECT
    std::shared_ptr<Railway> railway{};
    RailStationModel* const model;

    QEControlledTable* ctable;
    QTableView* table;
    QLineEdit* edName;
    QString pathString;
    QTextBrowser* pathBrowser = nullptr;
    DialogAdapter* pathDlg = nullptr;
public:
    RailPreviewDialog(QWidget* parent=nullptr);
private:
    void initUI();
signals:
    void railConfirmed(std::shared_ptr<Railway> rail);
private slots:
    void actConfirm();
    void actShowPath();
public slots:
    void setRailway(std::shared_ptr<Railway> railway, const QString& pathString);
    void refreshData();
    void refreshBasicData();
};

