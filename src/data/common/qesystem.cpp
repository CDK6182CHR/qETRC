#include "qesystem.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

SystemJson SystemJson::instance;   //默认构造

void SystemJson::saveFile()
{
    static const QString filename = "system.json";
    QFile file(filename);
    file.open(QFile::WriteOnly);
    if (!file.isOpen()) {
        qDebug() << "SystemJson::saveFile: WARNING: open file " << filename << " failed." << Qt::endl;
        return;
    }
    QJsonDocument doc(toJson());
    file.write(doc.toJson());
    file.close();
}

void SystemJson::addHistoryFile(const QString& name)
{
    history.removeAll(name);
    if (history.size() >= history_count)
        history.pop_back();
    history.push_front(name);
    last_file = name;
}

SystemJson::~SystemJson()
{
    saveFile();
}

SystemJson::SystemJson()
{
    static const QString filename = "system.json";
    QFile file(filename);
    file.open(QFile::ReadOnly);
    if (!file.isOpen()) {
        qDebug() << "SystemJson::SystemJson: WARNING: system configuration file " << filename <<
            " not read. Use default." << Qt::endl;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    fromJson(doc.object());
    file.close();
}

void SystemJson::fromJson(const QJsonObject& obj)
{
    last_file = obj.value("last_file").toString();
    default_file = obj.value("default_file").toString(default_file);
    default_raildb_file = obj.value("default_raildb_file")
            .toString(default_raildb_file);
    table_row_height = obj.value("table_row_height").toInt(table_row_height);
    show_train_tooltip = obj.value("show_train_tooltip").toBool(show_train_tooltip);
    ribbon_style = obj.value("ribbon_style").toInt(1);
    weaken_unselected = obj.value("weaken_unselected").toBool(true);
    use_central_widget = obj.value("use_central_widget").toBool(true);
    auto_highlight_on_selected = obj.value("auto_highlight_on_selected").toBool(true);
    show_start_page = obj.value("show_start_page").toBool(true);

    const QJsonArray& arhis = obj.value("history").toArray();
    for (const auto& p : arhis) {
        history.append(p.toString());
    }
}

QJsonObject SystemJson::toJson() const
{
    QJsonArray ar;
    for (const auto& p : history)
        ar.append(p);
    return QJsonObject{
        {"last_file",last_file},
        {"default_file",default_file},
        {"default_raildb_file",default_raildb_file},
        {"ribbon_style",ribbon_style},
        {"history",ar},
        {"table_row_height",table_row_height},
        {"show_train_tooltip",show_train_tooltip},
        {"weaken_unselected",weaken_unselected},
        {"use_central_widget",use_central_widget},
        {"auto_highlight_on_selected",auto_highlight_on_selected},
        {"show_start_page",show_start_page},
    };
}

