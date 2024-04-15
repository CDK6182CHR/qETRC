#include "typemanager.h"
#include "traintype.h"
#include "trainname.h"
#include "data/common/qesystem.h"
#include <QJsonObject>

TypeManager::TypeManager():
    defaultPen(QColor(0,128,0),1.0), defaultPenPassenger(QColor(0,128,0),1.5),
    defaultType(std::make_shared<TrainType>(QObject::tr("其他_"),defaultPen))
{
}

TypeManager& TypeManager::operator=(const TypeManager& another)
{
    _types.clear();
    _regs.clear();
    // 2023.12.23: special processing for default type
    defaultType = std::make_shared<TrainType>(*another.defaultType);
    _types.insert(defaultType->name(), defaultType);
    for (auto p=another._types.begin();p!=another._types.end();++p) {
        if (p.value() != another.defaultType) {
            _types.insert(p.key(), std::make_shared<TrainType>(*(p.value())));
        }
    }
    for (const auto& p : another._regs) {
        _regs.append(qMakePair(p.first, findOrCreate(p.second->name())));
    }
    defaultPen = another.defaultPen;
    //defaultType = std::make_shared<TrainType>(*(another.defaultType));   // 2023.12.24  moved above!
    //qDebug() << "========== After copy assign show ==========";
    //show();
    return *this;
}

void TypeManager::readForDefault(const QJsonObject& obj)
{
    bool flag = fromJson(obj, true);
    if (!flag) {
        initDefaultTypes();
    }
}

void TypeManager::readForDiagram(const QJsonObject& obj, const TypeManager& defaultManager)
{
    _types.clear();
    _regs.clear();
    if (!obj.contains("type_regex")) {
        //没有regex的旧版图，先应用默认的，再在上面修改
        operator=(defaultManager);
    }
    bool flag = fromJson(obj, false);
    if (!flag||isNull()) {
        qDebug() << "TypeManager::readForDiagram: WARNING: load type configuration for Diagram failed."
            << " Default TypeManager will be used. " << Qt::endl;
        operator=(defaultManager);
    }
}

std::shared_ptr<TrainType> TypeManager::addType(const QString& name, const QPen& pen)
{
    auto t = std::make_shared<TrainType>(name, pen);
    _types.insert(name, t);
    return t;
}

void TypeManager::appendRegex(const QRegularExpression& reg, const QString& name)
{
    _regs.append(qMakePair(reg, findOrCreate(name)));
}

std::shared_ptr<TrainType> TypeManager::appendRegex(const QRegularExpression& reg, const QString& name, bool passenger)
{
    auto t = findOrCreate(name, passenger);
    _regs.append(qMakePair(reg, t));
    return t;
}

std::shared_ptr<TrainType> TypeManager::fromRegex(const TrainName& name) const
{
    for (const auto& p : _regs) {
        //if (p.first.indexIn(name.full()) == 0)
        if (p.first.match(name.full()).hasMatch())
            return p.second;
    }
    return defaultType;
}

std::shared_ptr<TrainType> TypeManager::findOrCreate(const QString& name)
{
    if (_types.contains(name))
        return _types.value(name);
    else {
        auto t = std::make_shared<TrainType>(name, defaultPen);
        //qDebug() << "TypeManager::findOrCreate(QString): create type " << name << " @ " << t.get();
        _types.insert(name, t);
        return t;
    }
}

std::shared_ptr<TrainType> TypeManager::findOrCreate(const QString& name, bool passenger)
{
    if (_types.contains(name)) {
        auto t = _types.value(name);
        t->setIsPassenger(passenger);
        return t;
    }

    else {
        auto t = std::make_shared<TrainType>(name, passenger ?
            defaultPenPassenger : defaultPen, passenger);
        //qDebug() << "TypeManager::findOrCreate(QString, bool): create type " << name <<
        //    " @ " << t.get();
        _types.insert(name, t);
        return t;
    }
}

std::shared_ptr<TrainType> TypeManager::findOrCreate(std::shared_ptr<TrainType> tp)
{
    if (_types.contains(tp->name()))
        return _types.value(tp->name());
    else {
        auto it = _types.insert(tp->name(), std::make_shared<TrainType>(*tp));
        //qDebug() << "TypeManager::findOrCreate(TrainType*): create type " << tp->name() <<
        //    " @ " << it.value().get();
        return it.value();
    }
}

void TypeManager::swapForRegex(TypeManager& other)
{
    std::swap(_types, other._types);
    std::swap(_regs, other._regs);
}

//TypeManager::~TypeManager() noexcept
//{
//    _types.clear();
//    qDebug()<<"typeManager delete"<<Qt::endl;
//}

std::pair<QMap<QString, std::shared_ptr<TrainType>>, QVector<QPair<std::shared_ptr<TrainType>, std::shared_ptr<TrainType>>>> 
TypeManager::updateTypeSetTo(const TypeManager& other)
{
    QMap<QString, std::shared_ptr<TrainType>> data;
    QVector<QPair<std::shared_ptr<TrainType>, std::shared_ptr<TrainType>>> modified;
    for (auto itr=other.types().cbegin();itr!=other.types().cend();++itr) {
        if (auto p = _types.find(itr.key()); p != _types.end()) {
            // in this case, the required type exists in current object. 
            data.insert(itr.key(), p.value());
            // check whether modified
            if (itr.value()->operator!=(*p.value())) {
                // data is modified. However, we cannot put the object in `itr` directly in `modified`; 
                // instead, create a new object.
                auto ntype = std::make_shared<TrainType>(*itr.value());   // copy construct
                modified.emplace_back(p.value(), ntype);
            }
        }
        else {
            // the required type does not exist; create a new type
            auto ntype = std::make_shared<TrainType>(*itr.value());   // copy construct
            data.insert(itr.key(), ntype);
        }
    }
    return std::make_pair(data, modified);
}

void TypeManager::show() const
{
    qDebug() << "TypeManager @ " << this;
    qDebug() << "default type " << defaultType->name() << " @ " << defaultType.get();
    for (auto itr = _types.begin(); itr != _types.end(); ++itr) {
        qDebug() << "type " << itr.value()->name() << " @ " << itr.value().get();
    }
}

bool TypeManager::fromJson(const QJsonObject& obj, bool ignore_transparent)
{
    if (obj.isEmpty())
        return false;

    transparent_types = obj.value("transparent_types").toBool(false);
    if (transparent_types && SystemJson::instance.transparent_config && !ignore_transparent) {
        return false;
    }

    //先读取正则
    const QJsonArray& arreg = obj.value("type_regex").toArray();
    double pw = obj.value("default_keche_width").toDouble(1.0),
        fw = obj.value("default_huoche_width").toDouble(0.5);
    for (const auto& p:arreg) {
        const QJsonArray tp = p.toArray();
        auto t = appendRegex(QRegularExpression(tp.at(1).toString()), tp.at(0).toString(), tp.at(2).toBool());
        if (t->isPassenger())
            t->pen().setWidthF(pw);
        else
            t->pen().setWidthF(fw);
    }

    //读取颜色
    const QJsonObject& obcolor = obj.value("default_colors").toObject();
    for (auto p = obcolor.begin(); p != obcolor.end(); ++p) {
        if (p.key() == "default") {
            defaultType->pen().setColor(QColor(p.value().toString()));
        }
        else {
            auto tp = findOrCreate(p.key());
            tp->pen().setColor(QColor(p.value().toString()));
        }
    }

    //读取线型 新增
    const QJsonObject& ls = obj.value("line_styles").toObject();
    for (auto p = ls.begin(); p != ls.end(); ++p) {
        const QJsonArray& arsub = p.value().toArray();
        if (p.key() == "default") {
            defaultType->pen().setStyle(static_cast<Qt::PenStyle>(arsub.at(0).toInt()));
            defaultType->pen().setWidthF(arsub.at(1).toDouble());
        }
        else {
            auto tp = findOrCreate(p.key());
            tp->pen().setStyle(static_cast<Qt::PenStyle>(arsub.at(0).toInt()));
            tp->pen().setWidthF(arsub.at(1).toDouble());
        }
    }

    if (!_types.contains(defaultType->name())) {
        _types.insert(defaultType->name(), defaultType);
    }

    return _types.size() > 1;
}

void TypeManager::initDefaultTypes()
{
    // 2024.02.28: update default pen width for new implementation (w/o stroker)
    _types.insert(QObject::tr("其他_"), defaultType);   // 2023.12.23  fix name!
    addType(QObject::tr("快速"), QPen(QColor(255, 0, 0), 1.5));
    addType(QObject::tr("特快"), QPen(QColor(0, 0, 255), 1.5));
    addType(QObject::tr("直达特快"), QPen(QColor(255, 0, 255), 1.5));
    addType(QObject::tr("动车组"), QPen(QColor(128, 64, 0), 1.5));
    addType(QObject::tr("动车"), QPen(QColor(128, 64, 0), 1.5));
    addType(QObject::tr("高速"), QPen(QColor(255, 0, 190), 1.5));
    addType(QObject::tr("城际"), QPen(QColor(255, 51, 204), 1.5));
    addType(QObject::tr("市郊"), QPen(QColor(133, 46, 255), 1.5));
    addType(QObject::tr("临客"), QPen(QColor(128, 128, 128), 1.5));

    appendRegex(QRegularExpression(R"(^G\d+)"), QObject::tr("高速"), true);
    appendRegex(QRegularExpression(R"(^D\d+)"), QObject::tr("动车组"), true);
    appendRegex(QRegularExpression(R"(^C\d+)"), QObject::tr("城际"), true);
    appendRegex(QRegularExpression(R"(^Z\d+)"), QObject::tr("直达特快"), true);
    appendRegex(QRegularExpression(R"(^T\d+)"), QObject::tr("特快"), true);
    appendRegex(QRegularExpression(R"(^K\d+)"), QObject::tr("快速"), true);
    appendRegex(QRegularExpression(R"(^S\d+)"), QObject::tr("市郊"), true);
    appendRegex(QRegularExpression(R"(^[1-5]\d{3}$)"), QObject::tr("普快"), true);
    appendRegex(QRegularExpression(R"(^[1-5]\d{3}\D)"), QObject::tr("普快"), true);
    appendRegex(QRegularExpression(R"(^6\d{3}$)"), QObject::tr("普客"), true);
    appendRegex(QRegularExpression(R"(^6\d{3}\D)"), QObject::tr("普客"), true);
    appendRegex(QRegularExpression(R"(^7[0-5]\d{2}$)"), QObject::tr("普客"), true);
    appendRegex(QRegularExpression(R"(^7[0-5]\d{2}\D)"), QObject::tr("普客"), true);
    appendRegex(QRegularExpression(R"(^7\d{3}$)"), QObject::tr("通勤"), true);
    appendRegex(QRegularExpression(R"(^7\d{3}\D)"), QObject::tr("通勤"), true);
    appendRegex(QRegularExpression(R"(^8\d{3}$)"), QObject::tr("通勤"), true);
    appendRegex(QRegularExpression(R"(^8\d{3}\D)"), QObject::tr("通勤"), true);
    appendRegex(QRegularExpression(R"(^Y\d+)"), QObject::tr("旅游"), true);
    appendRegex(QRegularExpression(R"(^57\d+)"), QObject::tr("路用"), true);
    appendRegex(QRegularExpression(R"(^X1\d{2})"), QObject::tr("特快行包"), true);
    appendRegex(QRegularExpression(R"(^DJ\d+)"), QObject::tr("动检"), true);
    appendRegex(QRegularExpression(R"(^0[GDCZTKY]\d+)"), QObject::tr("客车底"), true);
    appendRegex(QRegularExpression(R"(^L\d+)"), QObject::tr("临客"), true);
    appendRegex(QRegularExpression(R"(^0\d{4})"), QObject::tr("客车底"), true);
    appendRegex(QRegularExpression(R"(^X\d{3}\D)"), QObject::tr("行包"), false);
    appendRegex(QRegularExpression(R"(^X\d{3}$)"), QObject::tr("行包"), false);
    appendRegex(QRegularExpression(R"(^X\d{4})"), QObject::tr("班列"), false);
    appendRegex(QRegularExpression(R"(^1\d{4})"), QObject::tr("直达"), false);
    appendRegex(QRegularExpression(R"(^2\d{4})"), QObject::tr("直货"), false);
    appendRegex(QRegularExpression(R"(^3\d{4})"), QObject::tr("区段"), false);
    appendRegex(QRegularExpression(R"(^4[0-4]\d{3})"), QObject::tr("摘挂"), false);
    appendRegex(QRegularExpression(R"(^4[5-9]\d{3})"), QObject::tr("小运转"), false);
    appendRegex(QRegularExpression(R"(^5[0-2]\d{3})"), QObject::tr("单机"), false);
    appendRegex(QRegularExpression(R"(^5[3-4]\d{3})"), QObject::tr("补机"), false);
    appendRegex(QRegularExpression(R"(^55\d{3})"), QObject::tr("试运转"), false);
}

void TypeManager::toJson(QJsonObject& obj) const
{
    obj.insert("transparent_types", transparent_types);
    if (SystemJson::instance.transparent_config && transparent_types) {
        return;
    }
    //颜色
    QJsonObject objcolor;
    //LineStype二元组：线型，粗细。新增
    QJsonObject ls;
    for (auto p = _types.begin(); p != _types.end(); ++p) {
        if (p.value() == defaultType) {
            continue;

        }
        else {
            objcolor.insert(p.key(), p.value()->pen().color().name());
            ls.insert(p.key(),
                QJsonArray{(int) p.value()->pen().style(),p.value()->pen().widthF() });
        }
    }
    // 保证default必须有，否则pyETRC会崩
    objcolor.insert("default", defaultType->pen().color().name());
    ls.insert("default", QJsonArray{ (int)defaultType->pen().style(),
        defaultType->pen().widthF() });
    obj.insert("default_colors", objcolor);
    obj.insert("line_styles", ls);
    //正则
    QJsonArray arreg;
    for (auto p = _regs.begin(); p != _regs.end(); ++p) {
        arreg.append(
            QJsonArray{ p->second->name(), p->first.pattern(),p->second->isPassenger() });
    }
    obj.insert("type_regex", arreg);
}
