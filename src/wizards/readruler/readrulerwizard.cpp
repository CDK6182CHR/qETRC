#include "readrulerwizard.h"

#include <QtWidgets>
#include "data/diagram/diagram.h"

ReadRulerWizard::ReadRulerWizard(Diagram &diagram_, QWidget *parent):
    QWizard(parent), diagram(diagram_)
{
    setWindowTitle(tr("标尺综合"));
    setAttribute(Qt::WA_DeleteOnClose);
    resize(900, 800);
    initUI();
}

void ReadRulerWizard::initUI()
{
    initStartPage();
    pgInterval=new ReadRulerPageInterval(diagram.railCategory());
    addPage(pgInterval);
}

void ReadRulerWizard::initStartPage()
{
    auto* page = new QWizardPage;
    page->setTitle(tr("概览"));
    page->setSubTitle(tr("欢迎使用标尺自动生成向导\n"
        "点击[下一步]开始配置。"));
    auto* vlay = new QVBoxLayout(page);
    auto* lab = new QLabel(tr("逻辑说明：\n"
        "此向导将引导用户选择一组本线的区间，并按照一组选定的车次在该区间的"
        "运行时分，计算区间运行时分标准（标尺）。\n"
        "按照车次在区间的起停附加情况（通通，起通，通停，"
        "起停四种）可将车次分为四类。并按照下列两种算法之一决定每一类的标准数据\n"
        "（1）众数模式。找出每一组数据（运行秒数）中出现次数最多的那个"
        "作为本类的运行数据。如果有多个出现次数一样的，则取最快的那个。\n"
        "（2）均值模式。首先删除每一组数据中的离群数据。如果数据偏离样本均值"
        "超过用户指定的秒数或者用户指定的样本标准差倍数，则剔除数据。然后取所有"
        "剩余数据的平均值作为本类运行数据。\n\n"
        "产生的四类情况的数据实质上就是一个线性方程组。按照方程的数量，"
        "分为四种情况：\n"
        "a. 4类都有数据，即有四个方程。在众数模式下，如果存在一个数量最少的类，"
        "则删除这个方程，求解剩余3个方程构成的线性方程组即得结果。"
        "否则选择出现次数最少的（并列）类之一删除，使得计算出来的结果最快。"
        "（pyETRC v3.1.3版本开始，众数模式中任何情况下不使用伪逆）\n"
        "b. 有3类有数据，即3个方程，刚好对应3个未知数（通通时分、起步附加、"
        "停车附加），线性方程组一定有唯一解，求解即可。\n"
        "c. 有2类有数据。此时需要用到用户输入的[默认起步附加时分]和[默认停车"
        "附加时分]中的一个。如果起、停之中有一个是能确定的，则使用这个能确定"
        "的数据；如果起、停是对称的，则优先用[默认起步附加时分]。\n"
        "d. 只有1类有数据。此时用户输入的[默认起步附加时分]和"
        "[默认停车附加时分]都会使用。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);
    addPage(page);
}
