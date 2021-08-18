#include "outputsubdiagramdialog.h"

#include <QtWidgets>
#include "util/buttongroup.hpp"
#include "data/diagram/diagram.h"

OutputSubDiagramDialog::OutputSubDiagramDialog(Diagram &diagram_, QWidget *parent):
    QDialog(parent),diagram(diagram_)
{
    setWindowTitle(tr("导出单线路运行图"));
    resize(400,400);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void OutputSubDiagramDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("此功能提供导出到ETRC和pyETRC所支持的运行图格式。由于pyETRC和ETRC皆只支持"
            "单一线路的运行图，故请选择一条线路进行导出。\n"
            "请注意pyETRC可以直接打开本系统的运行图文件（无需使用本功能导出），"
            "但这种情况下只能读取第一条线路。\n"
            "请注意导出ETRC运行图文件将带来显著的数据损失，通常先导出再读入ETRC文件得不到相同的结果。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);
    auto* flay=new QFormLayout;
    cbRail=new SelectRailwayCombo(diagram.railCategory());
    flay->addRow(tr("选择线路"),cbRail);
    ckLocal=new QCheckBox(tr("仅导出经由所选线路运行的车次"));
    flay->addRow(tr("选项"),ckLocal);
    vlay->addLayout(flay);

    auto* g=new ButtonGroup<3>({"导出ETRC文件","导出pyETRC文件","关闭"});
    g->connectAll(SIGNAL(clicked()),this,{SLOT(onOutputTrc()),SLOT(onOutputPyetrc()),
                  SLOT(close())});
    vlay->addLayout(g);
}

void OutputSubDiagramDialog::onOutputTrc()
{
    auto rail=cbRail->railway();
    if(!rail){
        QMessageBox::warning(this,tr("错误"),tr("请先选择线路！"));
        return;
    }
    QString f=QFileDialog::getSaveFileName(this,tr("导出ETRC运行图文件"),rail->name(),
              tr("ETRC运行图文件 (*.trc)\n所有文件 (*)"));
    if(!f.isNull()){
        bool flag=diagram.toTrc(f,rail,ckLocal->isChecked());
        if(flag)
            QMessageBox::information(this,tr("提示"),tr("导出ETRC运行图文件成功"));
        else
            QMessageBox::warning(this,tr("错误"),tr("导出ETRC运行图文件失败"));
    }

}

void OutputSubDiagramDialog::onOutputPyetrc()
{
    auto rail=cbRail->railway();
    if(!rail){
        QMessageBox::warning(this,tr("错误"),tr("请先选择线路！"));
        return;
    }
    QString f=QFileDialog::getSaveFileName(this,tr("导出pyETRC单线路运行图文件"),rail->name(),
              tr("pyETRC运行图文件 (*.pyetgr;*.json)\n所有文件 (*)"));
    if(!f.isNull()){
        bool flag=diagram.toSinglePyetrc(f,rail,ckLocal->isChecked());
        if(flag)
            QMessageBox::information(this,tr("提示"),tr("导出pyETRC单线路运行图文件成功"));
        else
            QMessageBox::warning(this,tr("错误"),tr("导出pyETRC单线路运行图文件失败"));
    }
}
