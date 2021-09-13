#include "raildb.h"

#include <QFile>
#include <QJsonDocument>


bool RailDB::parseJson(const QString &filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly);
    if(!file.isOpen()){
        qDebug()<<"RailDB::parseJson: WARNING: open file "<<filename<<" failed."<<
                  Qt::endl;
        return false;
    }
    QJsonDocument doc=QJsonDocument::fromJson(file.readAll());
    fromJson(doc.object());
    if (!isNull()){
        this->_filename=filename;
        return true;
    }else return false;
}

bool RailDB::save() const
{
    QFile file(_filename);
    file.open(QFile::WriteOnly);
    if (!file.isOpen()) {
        qDebug() << "Diagram::save: WARNING: open file " << _filename << " failed. Nothing todo."
            << Qt::endl;
        return false;
    }
    QJsonDocument doc(toJson());
    file.write(doc.toJson());
    file.close();
    return true;
}

bool RailDB::saveAs(const QString &filename)
{
    _filename=filename;
    return save();
}
