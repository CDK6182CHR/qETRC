#include "railrulercombo.h"

#include <QComboBox>
#include <QtWidgets>
#include "data/rail/railcategory.h"
#include "data/rail/rail.h"

RailRulerCombo::RailRulerCombo(RailCategory &cat_, QWidget *parent):
    QHBoxLayout(parent),cat(cat_),withEmptyRuler(false)
{
    initUI();
}

RailRulerCombo::RailRulerCombo(RailCategory& cat_, const QString& emptyName_, QWidget* parent):
    QHBoxLayout(parent),cat(cat_),withEmptyRuler(true),emptyName(emptyName_)
{
    initUI();
}

std::shared_ptr<Ruler> RailRulerCombo::dialogGetRuler(RailCategory& cat_, 
    QWidget* parent, const QString& title, const QString& prompt)
{
    QDialog*dialog=new QDialog(parent);
    dialog->setWindowTitle(title);
    auto* vlay = new QVBoxLayout(dialog);
    if (!prompt.isEmpty()) {
        auto* lab = new QLabel(prompt);
        lab->setWordWrap(true);
        vlay->addWidget(lab);
    }
    auto* cb = new RailRulerCombo(cat_);
    vlay->addLayout(cb);
    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    vlay->addWidget(box);
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    auto t = dialog->exec();
    std::shared_ptr<Ruler> res{};
    if (t) {
        res = cb->ruler();
    }
    dialog->setParent(nullptr);
    delete dialog;
    return res;
}

void RailRulerCombo::initUI()
{
    cbRail=new QComboBox;
    cbRuler=new QComboBox;
    cbRail->setEditable(false);
    cbRuler->setEditable(false);
    addWidget(cbRail);
    addWidget(cbRuler);

    connect(cbRail,SIGNAL(currentIndexChanged(int)),this,
            SLOT(onRailIndexChanged(int)));
    connect(cbRuler,SIGNAL(currentIndexChanged(int)),this,
            SLOT(onRulerIndexChanged(int)));

    refreshRailwayList();
}

void RailRulerCombo::onRailIndexChanged(int i)
{
    _railway.reset();
    _ruler.reset();
    if (i>=0 && i<cat.railways().size()){
        _railway=cat.railways().at(i);
        cbRuler->clear();
        if (withEmptyRuler)
            cbRuler->addItem(emptyName);
        foreach(auto p, _railway->rulers()){
            cbRuler->addItem(p->name());
        }
        emit railwayChagned(_railway);
    }
}

void RailRulerCombo::onRulerIndexChanged(int i)
{
    _ruler.reset();
    if(_railway){
        if (withEmptyRuler)
            i--;
        if (i >= 0 && i < _railway->rulers().size()) {
            _ruler = _railway->getRuler(i);
            emit rulerChanged(_ruler);
        }
        else if (withEmptyRuler) {
            _ruler.reset();
            emit rulerChanged(_ruler);
        }
    }
}

void RailRulerCombo::refreshRulerList()
{
    cbRuler->clear();
    if (withEmptyRuler) {
        cbRuler->addItem(emptyName);
    }
    if(_railway){
        foreach(auto p,_railway->rulers()){
            cbRuler->addItem(p->name());
        }
    }
}

void RailRulerCombo::refreshRailwayList()
{
    cbRail->clear();
    for(auto p:cat.railways()){
        cbRail->addItem(p->name());
    }
}

RulerCombo::RulerCombo(std::shared_ptr<Railway> railway, QWidget* parent):
    QComboBox(parent),_railway(railway)
{
    refreshData();
    connect(this, qOverload<int>(&QComboBox::currentIndexChanged),
        this, &RulerCombo::onCurrentChanged);
}

void RulerCombo::refreshData()
{
    clear();
    foreach(auto ruler, _railway->rulers()) {
        addItem(ruler->name());
    }
}

void RulerCombo::onCurrentChanged(int index)
{
    if (index >= 0) {
        _ruler = _railway->rulers().at(index);
    }
    else {
        _ruler.reset();
    }
}
