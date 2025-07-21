#include "printdiagramdialog.h"
#include "util/buttongroup.hpp"

#include "kernel/diagramwidget.h"
#include "data/diagram/diagrampage.h"
#include "data/diagram/diagram.h"

#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QTextEdit>
#include <QSpinBox>

PrintDiagramDialog::PrintDiagramDialog(DiagramWidget *dw_, QWidget *parent):
    QDialog(parent),dw(dw_),page(dw_->page())
{
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void PrintDiagramDialog::initUI()
{
    resize(400,400);
    setWindowTitle(tr("输出运行图 - %1").arg(page->name()));
    auto* vlay=new QVBoxLayout;
    auto* form=new QFormLayout;

    auto* label=new QLabel(tr("请注意，以下编辑的内容仅用于本次输出，不会保存。\n"
        "标题和注记默认为运行图页面的标题和备注，如果希望保存这些设置，"
        "可以到运行图页面的工具栏页面中编辑。"));
    label->setWordWrap(true);
    vlay->addWidget(label);

    edName=new QLineEdit;
    edName->setText(page->name()+tr("运行图"));
    form->addRow(tr("运行图标题"),edName);
    vlay->addLayout(form);

    auto* chlay = new QHBoxLayout;
    spHours = new QSpinBox;
    spHours->setRange(1, dw->diagram().options().period_hours);
    spHours->setValue(6);
    chlay->addWidget(spHours);
    chlay->addWidget(new QLabel(tr("仅用于分页PDF")));
    form->addRow(tr("每页小时数"), chlay);

    vlay->addWidget(new QLabel(tr("运行图备注：")));
    edNote=new QTextEdit;
    edNote->setPlainText(page->note());
    vlay->addWidget(edNote);

    auto* g=new ButtonGroup<4>({"输出PDF", "输出分页PDF", "输出PNG","取消"});
    g->connectAll(SIGNAL(clicked()),this,{
                      SLOT(onSavePdf()), SLOT(onSavePdfMultiPage()), SLOT(onSavePng()),SLOT(close())
                  });
    vlay->addLayout(g);
    setLayout(vlay);
}

void PrintDiagramDialog::onSavePdf()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("导出PDF"), edName->text(),
        tr("PDF文档 (*.pdf)"));
    if (fn.isEmpty())
        return;
#if 0
    bool flag = dw->toPdf(fn, edName->text(), edNote->toPlainText());
    if (flag) {
        QMessageBox::information(this, tr("提示"), tr("导出PDF文档成功"));
        done(QDialog::Accepted);
    }
    else {
        QMessageBox::warning(this, tr("错误"), tr("导出PDF文档失败，可能因为文件占用。"));
    }
#else 
    // 2024.04.10: async impl
    dw->toPdfAsync(fn, edName->text(), edNote->toPlainText(), this);
#endif
}

void PrintDiagramDialog::onSavePdfMultiPage()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("导出分页PDF"), edName->text(),
        tr("PDF文档 (*.pdf)"));
    if (fn.isEmpty())
        return;
    dw->toPdfAsyncMultiPage(fn, edName->text(), edNote->toPlainText(), this, spHours->value());
}

void PrintDiagramDialog::onSavePng()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("导出PNG"), edName->text(),
        tr("PNG图形 (*.png)"));
    if (fn.isEmpty())
        return;
    bool flag = dw->toPng(fn, edName->text(), edNote->toPlainText());
    if (flag) {
        QMessageBox::information(this, tr("提示"), tr("导出PNG图片成功"));
        done(QDialog::Accepted);
    }
    else {
        QMessageBox::warning(this, tr("错误"), tr("导出PNG图片失败，可能因为文件占用。"));
    }
}
