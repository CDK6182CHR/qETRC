#pragma once
#include <QMetaType>
#include <memory>

class Routing;
class Train;

Q_DECLARE_METATYPE(std::shared_ptr<Routing>)

Q_DECLARE_METATYPE(std::shared_ptr<Train>)

Q_DECLARE_METATYPE(std::shared_ptr<const Train>)
