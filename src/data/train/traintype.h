#pragma once

#include <QString>
#include <memory>
#include <QPen>
#include <QMetaType>

#include "trainlinestyle.h"

class TrainName;


/**
 * pyETRC中无对应抽象
 * 封装与列车类型有关的所有数据
 * UI作为const指针，全程只能指向同一个对象，但可以修改内容
 */
class TrainType
{
    QString _name;
    TrainLineStyle _line_style;
    bool _passenger;
public:

    /**
     * 2026.10.16: For compatibility with old codes: we generate the line style object with the QPen here.
     */
    TrainType(const QString& name, const QPen& pen, bool passenger=false);

    TrainType(const QString& name, const TrainLineStyle& line_style, bool passenger=false);

    // for test...
    TrainType(const TrainType& other) = default;

    const TrainLineStyle& line_style()const{return _line_style;}
    TrainLineStyle& line_style() { return _line_style; }

	const QPen& pen()const { return _line_style.line_pen(); }

    bool isPassenger()const { return _passenger; }
    void setIsPassenger(bool p) { _passenger = p; }

    const QString& name()const { return _name; }

    bool operator==(const TrainType& other)const;
    bool operator!=(const TrainType& other)const;

    void swap(TrainType& other);

    ~TrainType()noexcept=default;
};


Q_DECLARE_METATYPE(std::shared_ptr<TrainType>)


