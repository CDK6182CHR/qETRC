#include "selecttraincombo.h"

#include <QtWidgets>

SelectTrainCombo::SelectTrainCombo(TrainCollection &coll_, QWidget *parent):
    QHBoxLayout(parent), coll(coll_)
{
    initUI();
}

std::shared_ptr<Train> SelectTrainCombo::dialogGetTrain(TrainCollection& coll_, 
    QWidget* parent, const QString& title,const QString& prompt)
{
    QDialog* dialog=new QDialog(parent);
    dialog->setWindowTitle(title);
    auto* vlay = new QVBoxLayout(dialog);
    if (!prompt.isEmpty()) {
        auto* lab = new QLabel(prompt);
        lab->setWordWrap(true);
        vlay->addWidget(lab);
    }

    auto* cb = new SelectTrainCombo(coll_);
    vlay->addLayout(cb);
    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    vlay->addWidget(box);
    connect(box, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(box, SIGNAL(rejected()), dialog, SLOT(reject()));
    auto t = dialog->exec();
    std::shared_ptr<Train> res{};
    if (t) {
        res = cb->train();
    }
    dialog->setParent(nullptr);
    delete dialog;
    return res;
}

void SelectTrainCombo::initUI()
{
    edName=new QLineEdit;
    edName->setToolTip(tr("请输入部分或完整车次，编辑结束后将在右侧下拉列表中显示"
            "符合条件的车次。"));
    connect(edName,&QLineEdit::editingFinished,
            this,&SelectTrainCombo::onEditingFinished);
    addWidget(edName,1);

    cbTrains=new QComboBox;
    connect(cbTrains,SIGNAL(currentIndexChanged(int)),
            this,SLOT(onIndexChanged(int)));
    addWidget(cbTrains,2);
}

void SelectTrainCombo::onEditingFinished()
{
    matched = coll.multiSearchTrain(edName->text());
    cbTrains->clear();
    for(auto t:matched){
        cbTrains->addItem(tr("%1 (%2->%3)").arg(t->trainName().full())
                          .arg(t->starting().toSingleLiteral())
                          .arg(t->terminal().toSingleLiteral()));
    }
    cbTrains->setFocus();
    if(cbTrains->count() > 1){
        cbTrains->showPopup();
    }
}

void SelectTrainCombo::onIndexChanged(int i)
{
    if (i>=0 && i<matched.size()){
        if(_train!=matched.at(i)){
            _train=matched.at(i);
            emit currentTrainChanged(_train);
        }
    }else{
        qDebug()<<"SelectTrainCombo::onIndexChanged: WARNING: invalid index "
                <<i<<Qt::endl;
    }
}
