#include "qesystem.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

#include "filepaths.h"

//SystemJson SystemJson::get();   //默认构造

SystemJson& SystemJson::get()
{
    static SystemJson instance;
	return instance;
}

void SystemJson::saveFile()
{
    static const QString filename = "system.json";
	const QString full_name = qeutil::getSystemFileFullPath(filename);
    QFile file(full_name);
    file.open(QFile::WriteOnly);
    if (!file.isOpen()) {
        qDebug() << "SystemJson::saveFile: WARNING: open file " << full_name << " failed." << Qt::endl;
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
    const QString full_name = qeutil::getSystemFileFullPath(filename);
	qDebug() << "SystemJson::SystemJson: reading system configuration file " << full_name << Qt::endl;
    QFile file(full_name);
    file.open(QFile::ReadOnly);
    if (!file.isOpen()) {
        qDebug() << "SystemJson::SystemJson: WARNING: system configuration file " << full_name <<
            " not read. Use default." << Qt::endl;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    fromJson(doc.object());
    file.close();
}

void SystemJson::fromJson(const QJsonObject& obj)
{
    language = static_cast<QLocale::Language>(obj.value("language").toInt(language));
    last_file = obj.value("last_file").toString();
    default_file = obj.value("default_file").toString(default_file);
    default_raildb_file = obj.value("default_raildb_file")
            .toString(default_raildb_file);
    table_row_height = obj.value("table_row_height").toInt(table_row_height);
    show_train_tooltip = obj.value("show_train_tooltip").toBool(show_train_tooltip);
    ribbon_style = obj.value("ribbon_style").toInt(18);
    // 2024.03.28: for old version
    if (ribbon_style == 0) ribbon_style = 17;
    else if (ribbon_style == 1) ribbon_style = 18;
    ribbon_theme = obj.value("ribbon_theme").toInt(0);
    ribbon_align_center = obj.value("ribbon_align_center").toBool(false);
    weaken_unselected = obj.value("weaken_unselected").toBool(true);
    use_central_widget = obj.value("use_central_widget").toBool(true);
    auto_highlight_on_selected = obj.value("auto_highlight_on_selected").toBool(true);
    show_start_page = obj.value("show_start_page").toBool(true);
    transparent_config = obj.value("transparent_config").toBool(true);
    inform_dragging = obj.value("inform_dragging").toBool(true);

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
        {"language", static_cast<int>(language)},
        {"last_file",last_file},
        {"default_file",default_file},
        {"default_raildb_file",default_raildb_file},
        {"ribbon_style",ribbon_style},
        {"ribbon_theme",ribbon_theme},
        {"ribbon_align_center", ribbon_align_center},
        {"history",ar},
        {"table_row_height",table_row_height},
        {"show_train_tooltip",show_train_tooltip},
        {"weaken_unselected",weaken_unselected},
        {"use_central_widget",use_central_widget},
        {"auto_highlight_on_selected",auto_highlight_on_selected},
        {"show_start_page",show_start_page},
        {"transparent_config", transparent_config},
        {"inform_dragging", inform_dragging},
    };
}

