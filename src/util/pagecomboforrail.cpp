#include "pagecomboforrail.h"
#include "data/diagram/diagram.h"
#include "data/diagram/diagrampage.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>


PageComboForRail::PageComboForRail(Diagram &diagram_, QWidget *parent):
    QComboBox(parent), diagram(diagram_)
{

}

PageComboForRail::PageComboForRail(Diagram &diagram_, std::shared_ptr<Railway> railway,
                                   QWidget *parent):
    QComboBox(parent), diagram(diagram_)
{
    setRailway(railway);
}

int PageComboForRail::pageIndex() const
{
    return currentData(Qt::UserRole).toInt();
}

void PageComboForRail::refreshData()
{
    setupItems();
}

int PageComboForRail::dlgGetPageIndex(Diagram &diagram, std::shared_ptr<Railway> rail,
            QWidget *parent, const QString &title, const QString &prompt)
{
    auto t = diagram.pageIndexWithRail(rail);
    if (t.empty())return -1;
    else if (t.size() == 1)return t.front();
    auto* dlg=new QDialog(parent);
    auto* vlay=new QVBoxLayout(dlg);
    dlg->setWindowTitle(title);
    auto* lab=new QLabel(prompt);
    lab->setWordWrap(true);
    vlay->addWidget(lab);
    auto* cb=new PageComboForRail(diagram, rail);
    vlay->addWidget(cb);
    auto* box=new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    vlay->addWidget(box);
    connect(box,&QDialogButtonBox::accepted,dlg,&QDialog::accept);
    connect(box,&QDialogButtonBox::rejected,dlg,&QDialog::reject);

    int res=-1;
    int flag=dlg->exec();
    if(flag){
        res=cb->pageIndex();
    }
    dlg->setParent(nullptr);
    delete dlg;
    return res;
}

void PageComboForRail::setupItems()
{
    clear();
    if(!_railway)return;
    auto& pages=diagram.pages();
    for(int i=0;i<pages.size();i++){
        auto p=pages.at(i);
        if(p->containsRailway(_railway)){
            addItem(p->name(),i);
        }
    }
}

void PageComboForRail::setRailway(const std::shared_ptr<Railway> &railway)
{
    _railway=railway;
    setupItems();
}

void PageComboForRail::onIndexChanged(int i)
{
    if (i==-1){
        // 无效的选项
        _page.reset();
    }else{
        int idx=currentData(Qt::UserRole).toInt();
        _page=diagram.pages().at(idx);
        emit pageIndexChanged(idx);
    }
}

