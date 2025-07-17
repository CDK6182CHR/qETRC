#include "intervaltraindialog.h"

#include <editors/train/trainfilterselector.h>
#include <data/diagram/diagram.h>
#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <data/common/qesystem.h>
#include <util/buttongroup.hpp>
#include <data/train/traintype.h>
#include <util/utilfunc.h>

#include "intervaltraintable.h"

bool IntervalTrainDialog::informMultiCheck = true;
bool IntervalTrainDialog::informRegexCheck = true;

IntervalTrainDialog::IntervalTrainDialog(Diagram &diagram, QWidget *parent):
    QDialog(parent),
    diagram(diagram),filter(new TrainFilterSelector(diagram.trainCollection(),this)),
    counter(diagram.trainCollection()),
    table(new IntervalTrainTable(diagram.options(), this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void IntervalTrainDialog::initUI()
{
    setWindowTitle(tr("区间车次表"));
    resize(800, 800);
    auto* vlay=new QVBoxLayout(this);
    auto* flay=new QFormLayout;

    auto* hlay = new QHBoxLayout;
    edFrom=new QLineEdit;
    ckMultiStart = new QCheckBox(tr("使用多车站分隔符|"));
    connect(ckMultiStart, &QCheckBox::toggled, this, &IntervalTrainDialog::onMultiChecked);
    hlay->addWidget(edFrom);
    hlay->addWidget(ckMultiStart);

    ckRegexStart = new QCheckBox(tr("使用正则表达式"));
    connect(ckRegexStart, &QCheckBox::toggled, this, &IntervalTrainDialog::onRegexChecked);
    hlay->addWidget(ckRegexStart);
    
    flay->addRow(tr("发站"),hlay);

    hlay = new QHBoxLayout;
    edTo = new QLineEdit;
    ckMultiEnd = new QCheckBox(tr("使用多车站分隔符|"));
    connect(ckMultiEnd, &QCheckBox::toggled, this, &IntervalTrainDialog::onMultiChecked);
    hlay->addWidget(edTo);
    hlay->addWidget(ckMultiEnd);

    ckRegexEnd = new QCheckBox(tr("使用正则表达式"));
    connect(ckRegexEnd, &QCheckBox::toggled, this, &IntervalTrainDialog::onRegexChecked);
    hlay->addWidget(ckRegexEnd);

    flay->addRow(tr("到站"),hlay);


    hlay=new QHBoxLayout;
    ckStop=new QCheckBox(tr("仅停车（含始发终到）车次"));
    ckBusiness=new QCheckBox("仅营业车次");
    hlay->addWidget(ckBusiness);
    hlay->addWidget(ckStop);

    hlay->addWidget(filter);

    flay->addRow(tr("筛选"),hlay);
    vlay->addLayout(flay);

    auto* g=new ButtonGroup<3>({"查询","导出CSV","关闭"});
    g->connectAll(SIGNAL(clicked()),this,
                  {SLOT(updateData()),SLOT(toCsv()),SLOT(close())});
    vlay->addLayout(g);



    vlay->addWidget(table);
}

void IntervalTrainDialog::updateData()
{
    const QString& from_name=edFrom->text();
    const QString& to_name=edTo->text();
    if(from_name.isEmpty() || to_name.isEmpty()){
        QMessageBox::warning(this,tr("错误"),tr("请输入非空车站名！"));
        return;
    }
    counter.setFilter(filter->filter());
    counter.setBusinessOnly(ckBusiness->isChecked());
    counter.setStopOnly(ckStop->isChecked());
    counter.setMultiStart(ckMultiStart->isChecked());
    counter.setMultiEnd(ckMultiEnd->isChecked());
    counter.setRegexStart(ckRegexStart->isChecked());
    counter.setRegexEnd(ckRegexEnd->isChecked());

    auto&& data=counter.getIntervalTrains(from_name,to_name);
    table->getModel()->resetData(std::move(data));
    table->getModel()->refreshData();
}

void IntervalTrainDialog::toCsv()
{
    qeutil::exportTableToCsv(table->getModel(), this,
                             tr("区间车次表%1-%2").arg(edFrom->text(),
                                                  edTo->text()));
}

void IntervalTrainDialog::onMultiChecked(bool on)
{
    if (informMultiCheck && on) {
        QMessageBox::information(this, tr("提示"), tr("此功能允许一次使用多个车站查询，"
            "输入的多个车站以垂直线| （U+007C，标准键盘\\符号上方）分隔，分隔符两端请不要加"
            "空格等其他任何符号。"
            "启用本功能时，如果车站站名内存在此符号，则会被当做两个站处理。\n"
            "输入的多个车站为“或”的关系，只要列车时刻表内存在一个车站，就能匹配。\n"
            "此提示在本程序运行期间仅展示一次。"));
        informMultiCheck = false;
    }
}

void IntervalTrainDialog::onRegexChecked(bool on)
{
    if (informRegexCheck && on) {
        QMessageBox::information(this, tr("提示"), tr("此功能允许使用正则表达式(regular expression)对站名"
            "进行范围匹配。匹配的判据为QRegularExpression::match(), QRegularExpressionMatch::hasMatch()。"
            "关于正则表达式的具体规则，可以自行搜索。注：\n"
            "（1） 若站名包含域解析符(::)，则匹配中将整个站名视为整体，不进行（本系统内置的）不严格匹配。\n"
            "（2）若正则表达式和多车站查询同时开启，则首先按照垂直线|（U+007C）划分，再将每一部分"
            "按正则表达式解析。"
            "（3）友情提示：如果要匹配完整的站名，可以使用正则表达式的行首、行尾表达，例如 ^成都$ \n" 
            "此提示在本程序运行期间仅展示一次。"));
        informRegexCheck = false;
    }
}
