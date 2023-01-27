#pragma once


#include <QColor>
#include <QString>
#include <QMap>
#include <QSet>
#include <QJsonObject>
#include <memory>
//#include "data/rail/ruler.h"


struct MarginConfig {
    int
        left_white = 15,
        right_white = 10,
        up = 90,
        down = 90,
        label_width = 80,
        mile_label_width = 40,
        ruler_label_width = 80,
        count_label_width = 60;
private:
    int left = 275,
        right = 230;

    friend struct Config;
    friend class ConfigDialog;

public:
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
    
    // 默认客车和货车线宽，好像也没用了
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
    bool end_label_name = true;

    bool show_ruler_bar = true,
        show_mile_bar = true,
        show_count_bar = true;

    MarginConfig margins{};

    //排图标尺，现在改为Railway的性质
    //std::weak_ptr<Ruler> ordinate;
    QSet<QString> not_show_types;

    bool fromJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    /**
     * 图幅总宽度 （第一至最后的时间线之间的距离）
     */
    double diagramWidth()const;

    /**
     * 24小时的完整图幅宽度 （0点至24点时间线总宽度）
     */
    inline double fullWidth()const {
        return 24 * 3600.0 / seconds_per_pix;
    }

    /**
     * 图幅左边界（最左侧时间线）与图边的总距离
     * 取代margins.left
     */
    double totalLeftMargin()const;

    /**
     * 排图标尺栏的开始坐标
     */
    double rulerBarX()const;

    /**
     * 延长公里栏开始坐标
     */
    double mileBarX()const;

    /**
     * 最后一条时间线至右侧边界的距离
     * 取代margins.right
     */
    double totalRightMargin()const;

    /**
     * 左侧站名栏开始的横坐标
     */
    double leftStationBarX()const;

    /**
     * 左侧表头总宽度，就是半透明rect的宽度
     */
    double leftRectWidth()const;
    double rightRectWidth()const;

    /**
     * 左侧排图标尺和延长公里栏的总宽度
     */
    double leftTitleRectWidth()const;
};
