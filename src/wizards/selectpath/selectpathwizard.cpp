#include "selectpathpagepreview.h"
#include "selectpathpagestart.h"
#include "selectpathwizard.h"
#include "railnet/path/pathselectwidget.h"
#include "railnet/path/pathoperationmodel.h"

#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

SelectPathWizard::SelectPathWizard(const RailNet& net, QWidget *parent):
    QWizard(parent),net(net)
{
    setWindowTitle(tr("交互式经由选择向导"));
    resize(1000,800);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void SelectPathWizard::initializePage(int id)
{
    QWizard::initializePage(id);
    if (id==2){
        if (selUp->getSeqModel()->sequence().empty())
            generateInversePath();
    }else if(id==3){
        pgPreview->setupData(pgStart->exportRuler(), pgStart->minRulerCount());
    }
}

void SelectPathWizard::initUI()
{
    pgStart=new SelectPathPageStart(this);
    addPage(pgStart);

    selDown=new PathSelectWidget(net);
    selDown->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    selDown->setContentsMargins(0,0,0,0);
    pgDown=new QWizardPage();
    pgDown->setTitle(tr("正向径路"));
    pgDown->setSubTitle(tr("请选择正向径路。生成的径路，将作为最终导出线路的"
        "下行方向。"));
    auto* vlay=new QVBoxLayout(pgDown);
    vlay->addWidget(selDown);
//    vlay->setContentsMargins(0,0,0,0);
    addPage(pgDown);

    selUp=new PathSelectWidget(net);
    selUp->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    pgUp=new QWizardPage;
    pgUp->setTitle(tr("反向径路"));
    pgUp->setSubTitle(tr("默认情况下，反向径路已经自动生成好。在反向径路生成无误的情况下，"
        "可以直接跳过本步骤。如果反向径路生成有误，可以手动重新生成。\n"
        "反向径路的起点、终点应当和正向径路的终点、起点分别一致，但本步骤不做检查。"));
    vlay=new QVBoxLayout(pgUp);
    auto* btn=new QPushButton(tr("重新自动生成反向径路"));
    connect(btn,&QPushButton::clicked,this,&SelectPathWizard::regenerateInversePath);
    vlay->addWidget(btn);
    vlay->addWidget(selUp);
//    vlay->setContentsMargins(0,0,0,0);
    addPage(pgUp);

    pgPreview=new SelectPathPagePreview(net, selDown->getSeqModel()->sequence(),
                                        selUp->getSeqModel()->sequence());
    addPage(pgPreview);
}

void SelectPathWizard::regenerateInversePath()
{
    auto flag=QMessageBox::question(this,tr("提示"),
                                    tr("是否确认重新生成反向径路？"
        "如此前已有更改，将被覆盖，且不能撤销。"));
    if (flag==QMessageBox::Yes)
        generateInversePath();
}

void SelectPathWizard::generateInversePath()
{
    QString report;
    bool flag=selUp->getSeqModel()->fromInverseSeq(
                selDown->getSeqModel()->sequence(), &report);
    if (!flag){
        QMessageBox::warning(this,tr("错误"),tr("自动生成反向径路错误，原因如下：\n%1\n"
        "现在已保留出错前的部分径路。请手动选择剩余径路。").arg(report));
    }
}

void SelectPathWizard::accept()
{
    auto rail=pgPreview->getRailway();
    if(!rail){
        QMessageBox::warning(this,tr("错误"),tr("意外的空线路"));
        return;
    }
    emit railwayConfirmed(rail);
    QWizard::accept();
}

void SelectPathWizard::reject()
{
    if (currentId()>0){
        auto flag=QMessageBox::question(this,tr("经由选择"),tr("是否确认退出"
        "交互式经由选择向导？已做的配置不会生效。"));
        if (flag!=QMessageBox::Yes)
            return;
    }
    QWizard::reject();
}
