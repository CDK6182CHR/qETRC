#pragma once
#include <QMetaType>
#include <memory>

class Routing;
class Train;
class Railway;

Q_DECLARE_METATYPE(std::shared_ptr<Routing>)

#if QT_VERSION_MAJOR < 6
    // this seems causes error in Qt6, the reason is not clear
Q_DECLARE_METATYPE(std::shared_ptr<Train>)
#endif

Q_DECLARE_METATYPE(std::shared_ptr<const Train>)

Q_DECLARE_METATYPE(std::shared_ptr<Railway>)

template <typename Ty>
uint qHash(const std::shared_ptr<Ty>& key, uint seed)
{
    return qHash(key.get(), seed);
}
