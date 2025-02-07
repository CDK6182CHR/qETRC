#include "selectrailwaycombo.h"

#include "data/rail/railway.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

SelectRailwayCombo::SelectRailwayCombo(RailCategory &cat_, QWidget *parent):
    QComboBox(parent),cat(cat_)
{
    connect(this,SIGNAL(currentIndexChanged(int)),this,SLOT(onIndexChanged(int)));
    refresh();
}

void SelectRailwayCombo::refresh()
{
    clear();
    for(auto p:cat.railways()){
        addItem(p->name());
    }
}

std::shared_ptr<Railway> SelectRailwayCombo::dialogGetRailway(RailCategory& cat,
    QWidget* parent, const QString& title, const QString& prompt)
{
    QDialog* dialog = new QDialog(parent);
    dialog->setWindowTitle(title);
    auto* vlay = new QVBoxLayout(dialog);
    if (!prompt.isEmpty()) {
        auto* lab = new QLabel(prompt);
        lab->setWordWrap(true);
        vlay->addWidget(lab);
    }

    auto* cb = new SelectRailwayCombo(cat);
    vlay->addWidget(cb);
    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    vlay->addWidget(box);
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    auto t = dialog->exec();
    std::shared_ptr<Railway> res{};
    if (t) {
        res = cb->railway();
    }
    dialog->setParent(nullptr);
    dialog->deleteLater();
    return res;
}

void SelectRailwayCombo::setRailway(std::shared_ptr<const Railway> rail)
{
    int idx = cat.getRailwayIndex(rail);
    setCurrentIndex(idx);
}

std::shared_ptr<Railway> SelectRailwayCombo::dialogGetRailway(const QList<std::shared_ptr<Railway>>& railways, 
    QWidget* parent, const QString& title, const QString& prompt)
{
    if (railways.empty())
        return nullptr;
    else if (railways.size() == 1)
        return railways.front();
    RailCategory cat;
    cat.railways() = railways;
    return dialogGetRailway(cat, parent, title, prompt);
}

void SelectRailwayCombo::onIndexChanged(int i)
{
    if(0<=i && i<cat.railways().size()){
        _railway=cat.railways().at(i);
        emit currentRailwayChanged(_railway);
    }else{
        _railway.reset();
    }
}
