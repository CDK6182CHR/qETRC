#include "selecttraincombo.h"

#include <QtWidgets>

SelectTrainCombo::SelectTrainCombo(TrainCollection &coll_, QWidget *parent):
    QHBoxLayout(parent), coll(coll_)
{
    initUI();
}

void SelectTrainCombo::initUI()
{
    edName=new QLineEdit;
    edName->setToolTip(tr("请输入部分或完整车次，编辑结束后将在右侧下拉列表中显示"
            "符合条件的车次。"));
    connect(edName,&QLineEdit::editingFinished,
            this,&SelectTrainCombo::onEditingFinished);
    addWidget(edName,3);

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
        cbTrains->addItem(t->trainName().full());
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
