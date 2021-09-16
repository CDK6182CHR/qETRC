#include "railinfonote.h"


void RailInfoNote::fromJson(const QJsonObject& obj)
{
    author = obj.value("author").toString();
    version = obj.value("version").toString();
    note = obj.value("note").toString();
}

QJsonObject RailInfoNote::toJson() const
{
    return QJsonObject({
       {"author",author},
       {"version",version},
       {"note",note}
                       });
}

bool RailInfoNote::operator==(const RailInfoNote &other) const
{
    return author==other.author && version==other.version &&
            note==other.note;
}

bool RailInfoNote::operator!=(const RailInfoNote &other) const
{
    return ! operator==(other);
}
