#pragma once
#include <QDialog>

class QTextEdit;
class QLineEdit;
class DiagramPage;
class DiagramWidget;


/**
 * @brief The PrintDiagramDialog class
 * 输出运行图的对话框，包括PDF和PNG，支持输出前编辑标题和备注
 */
class PrintDiagramDialog:
        public QDialog
{
    Q_OBJECT;
    DiagramWidget*const dw;
    const std::shared_ptr<DiagramPage> page;
    QLineEdit* edName;
    QTextEdit* edNote;
public:
    PrintDiagramDialog(DiagramWidget* dw_,QWidget* parent);

private:
    void initUI();

private slots:
    void onSavePdf();
    void onSavePng();
};


