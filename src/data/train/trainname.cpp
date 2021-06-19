#include "trainname.h"

#include <QtCore>

TrainName::TrainName(const QString& full):
    _full(full)
{
    parseFullName();
}

void TrainName::fromJson(const QJsonArray &ar)
{
    _full=ar.at(0).toString();
    _down=ar.at(1).toString();
    _up=ar.at(2).toString();
}

QJsonArray TrainName::toJson() const
{
    return QJsonArray{_full,_down,_up};
}

TrainName::TrainName(const QString& full, const QString& down, const QString& up):
    _full(full),_down(down),_up(up)
{
}

void TrainName::show() const
{
    qDebug() << "TrainName [" << full() << "]: " << down() << "/" << up() << Qt::endl;
}

void TrainName::parseFullName()
{
    //这里其实不需要关心字头的事
    QStringList lst=_full.split('/');
    if(lst.size()<=1){
        Direction d = nameDirection(_full);
        if (DirFunc::isValid(d))
            dirNameValid(d) = _full;   //copy assign
    }
    else {
        QString&& a = lst.at(0).simplified(), &&b = lst.at(1).simplified();
        //第二段从左边开始，取到最后一个数字
        auto t = b.begin();
        for (; t != b.end(); ++t) {
            if (!t->isDigit()) {
                break;
            }
        }
        auto len = t - b.begin();   //有效数字长度
        if (len > a.length())
            len = std::min(a.length(), b.length());
        QString&& common_head = a.chopped(len);
        QString&& common_tail = b.right(b.length() - len);
        QString name1 = a + common_tail;
        QString name2 = common_head + b;
        if (nameDirection(name1) == Direction::Down) {
            _down = name1;
            _up = name2;
        }
        else {
            _down = name2;
            _up = name1;
        }
    }
}

Direction TrainName::nameDirection(const QString& s) const
{
    for (auto p = s.crbegin(); p != s.crend(); ++p) {
        auto t = *p;
        if (t.isDigit()) {
            return t.digitValue() % 2 == 0 ? Direction::Up : Direction::Down;
        }
    }
    return Direction::Undefined;
}


