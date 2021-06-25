#pragma once

#include <QColor>
#include <QString>
#include <QMap>
#include <QSet>
#include <QJsonObject>
#include <memory>
#include "data/rail/ruler.h"


struct MarginConfig {
    int
        left_white = 15,
        right_white = 10,
        left = 275,
        up = 90,
        down = 90,
        right = 150,
        label_width = 80,
        mile_label_width = 40,
        ruler_label_width = 80;

    //这两项从pyETRC.GraphicsWidget中来
    int
        title_row_height = 40,
        first_row_append = 15;

    /**
     * @brief gap_between_railways
     * 新增，相邻两线路之间的高度差
     */
    int gap_between_railways = 150;

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;
};

/**
 * @brief 运行图绘制相关配置
 * Graph.UIConfigData()  字典
 * 类型系统重新抽象；这边的I/O另外写来兼容旧有文件
 */
struct Config
{
    /**
     * 绘图比例
     */
    double
        seconds_per_pix=15.0,
        seconds_per_pix_y=8.0,
        pixels_per_km=4.0;
    QColor
        grid_color=QColor(170,170,127),  // #AAAA7F
        text_color=QColor(0,0,255);      // #0000FF
    double
        default_passenger_width = 1.5,
        default_freight_width = 0.75;
    QString default_db_file = "linesNew.pyetlib";
    int
        start_hour = 0,
        end_hour = 24;
    double
        minutes_per_vertical_line = 10.0,
        minute_mark_gap_pix = 200;
    int bold_line_level = 2;
    bool show_line_in_station = true;
    int start_label_height = 30;
    int end_label_height = 15;
    int table_row_height = 30;
    int link_line_height = 10;

    bool auto_paint = true;
    bool show_full_train_name = false;

    /**
     * 是否显示停点数字。
     * 0-一律不；1-选中车次显示；2-全部
     */
    int show_time_mark = 1;

    /**
     * 自动安排TrainItem配置时，最多允许间隔的站数。超过将分成两个Item
     */
    int max_passed_stations = 3;

    bool avoid_cover = true;
    int base_label_height = 15;
    int step_label_height = 20;

    double default_grid_width = 1.0;
    double bold_grid_width = 2.5;

    /**
     * 有效选择宽度
     */
    int valid_width = 3;

    /**
     * 原end_label_checi
     * 结束标签是否显示车次。
     */
    int end_label_name = true;

    //todo 类型

    MarginConfig margins{};

    //排图标尺，现在改为Railway的性质
    //std::weak_ptr<Ruler> ordinate;
    QSet<QString> not_show_types;

    void fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    double diagramWidth()const;

    inline double fullWidth()const {
        return 24 * 3600.0 / seconds_per_pix;
    }

};
