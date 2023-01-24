#pragma once

#include <QList>
#include <QString>
#include <QJsonObject>


/**
 * PyETRC中system.json 默认配置文件的抽象
 */
class SystemJson {
public:
    static constexpr int history_count = 20;
    static SystemJson instance;

    QString last_file;
    QString default_file = "sample.pyetgr";
    QString default_raildb_file = "CRPassengerMileage.pyetlib";
    QString app_style = "windowsvista";
    int table_row_height = 25;

    bool show_train_tooltip = true;

    int ribbon_style = 1;

    bool weaken_unselected = true;

    bool use_central_widget = true;

    bool auto_highlight_on_selected = true;

    bool show_start_page = true;

    //todo: dock show..

    /**
     * 新增  历史记录  从front加，back出
     */
    QList<QString> history;

    void saveFile();

    /**
     * 添加历史记录文件；同时记录为上一次的文件
     */
    void addHistoryFile(const QString& name);

    ~SystemJson();

private:
    /**
     * 构造函数直接读文件
     */
    SystemJson();

    void fromJson(const QJsonObject& obj);

    QJsonObject toJson()const;
};

