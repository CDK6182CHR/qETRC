#pragma once

#include "trainfiltercore.h"
#include <QJsonObject>

/**
 * @brief The PredefTrainFilterCore class
 * 2023.01.15  The predefined TrainFilterCore, including extra fields for identification,
 * also supports I/O
 */
class PredefTrainFilterCore : public TrainFilterCore
{
    QString _name,_note;
public:
    explicit PredefTrainFilterCore(Diagram& diagram);

    auto& name()const {return _name;}
    auto& note()const {return _note;}
    void setName(const QString& n){_name=n;}
    void setNote(const QString& n){_note=n;}

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;
};

