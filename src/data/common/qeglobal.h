#pragma once
#include <QMetaType>
#include <memory>

class Routing;
class Train;

Q_DECLARE_METATYPE(std::shared_ptr<Routing>)

Q_DECLARE_METATYPE(std::shared_ptr<Train>)

Q_DECLARE_METATYPE(std::shared_ptr<const Train>)

template <typename _Ty>
uint qHash(const std::shared_ptr<_Ty>& key, uint seed)
{
    return qHash(key.get(), seed);
}
