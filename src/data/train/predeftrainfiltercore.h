#pragma once

#include "trainfiltercore.h"
#include <QJsonObject>

#include <array>
#include <memory>

class TrainCollection;

/**
 * @brief The PredefTrainFilterCore class
 * 2023.01.15  The predefined TrainFilterCore, including extra fields for identification,
 * also supports I/O
 */
class PredefTrainFilterCore : public TrainFilterCore
{
    QString _name,_note;
public:
    explicit PredefTrainFilterCore()=default;
    PredefTrainFilterCore(const PredefTrainFilterCore&) = default;
    PredefTrainFilterCore(PredefTrainFilterCore&&) = default;
    PredefTrainFilterCore& operator=(const PredefTrainFilterCore&) = default;
    PredefTrainFilterCore& operator=(PredefTrainFilterCore&&) = default;

    auto& name()const {return _name;}
    auto& note()const {return _note;}
    void setName(const QString& n){_name=n;}
    void setNote(const QString& n){_note=n;}

    void fromJson(const QJsonObject& obj, const TrainCollection& coll);
    QJsonObject toJson()const;

    enum SysFilterId {
        AllTrains=0,
        PassengerTrains,
        ShownTrains,
        MAX_FILTERS
    };

    /**
     * The sysFilter's are internally defined and hard-coded filters, that cannot be modified by user.
     * The sysFilter of one specified type is logically singleton.
     */
    static const PredefTrainFilterCore* getSysFilter(SysFilterId id);
    static const QString& getSysFilterName(SysFilterId id);

    void swapWith(PredefTrainFilterCore& other);

private:
    static std::array<std::unique_ptr<const PredefTrainFilterCore>, MAX_FILTERS> sysFilters;
    static std::array<QString, MAX_FILTERS> sysFilterNames;
    static std::unique_ptr<const PredefTrainFilterCore> makeSysFilter(SysFilterId id);
    
};

Q_DECLARE_METATYPE(PredefTrainFilterCore*)

