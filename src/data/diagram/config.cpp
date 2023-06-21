#include "config.h"
#include <QJsonArray>

#define FROM_OBJ(_key,_type) \
_key = obj.value(#_key).to##_type(_key)

//名称不同的key
#define FROM_OBJ_NAME(_key,_strkey,_type) \
_key = obj.value(#_strkey).to##_type(_key)

void MarginConfig::fromJson(const QJsonObject& obj)
{
    FROM_OBJ(left_white, Int);
    FROM_OBJ(right_white, Int);
    FROM_OBJ(left, Int);
    FROM_OBJ(right, Int);
    FROM_OBJ(up, Int);
    FROM_OBJ(down, Int);
    FROM_OBJ(label_width, Int);
    FROM_OBJ(mile_label_width, Int);
    FROM_OBJ(ruler_label_width, Int);
    FROM_OBJ(title_row_height, Int);
    FROM_OBJ(first_row_append, Int);
    FROM_OBJ(gap_between_railways, Int);
}

#define TO_OBJ(_key) {#_key,_key},

#define TO_OBJ_NAME(_key,_strkey) {#_strkey,_key},

QJsonObject MarginConfig::toJson() const
{
    return QJsonObject{
        TO_OBJ(left_white)
        TO_OBJ(right_white)
        TO_OBJ(left)
        TO_OBJ(right)
        TO_OBJ(up)
        TO_OBJ(down)
        TO_OBJ(label_width)
        TO_OBJ(mile_label_width)
        TO_OBJ(ruler_label_width)
        TO_OBJ(title_row_height)
        TO_OBJ(first_row_append)
        TO_OBJ(gap_between_railways)
    };
}



bool Config::fromJson(const QJsonObject& obj)
{
    if (obj.empty())
        return false;
    FROM_OBJ(seconds_per_pix, Double);
    FROM_OBJ(seconds_per_pix_y, Double);
    FROM_OBJ(pixels_per_km, Double);

    //color这里暴力一下，默认值直接用hard-code
    grid_color = QColor(obj.value("grid_color").toString("#AAAA7F"));
    text_color = QColor(obj.value("text_color").toString("#0000FF"));
    
    FROM_OBJ_NAME(default_passenger_width, default_keche_width, Double);
    FROM_OBJ_NAME(default_freight_width, default_huoche_width, Double);
    FROM_OBJ(default_db_file, String);
    FROM_OBJ(start_hour, Int);
    FROM_OBJ(end_hour, Int);
    
    FROM_OBJ(minutes_per_vertical_line, Double);
    FROM_OBJ(minute_mark_gap_pix, Double);

    FROM_OBJ(minutes_per_vertical_second, Double);
    FROM_OBJ(minutes_per_vertical_bold, Double);
    FROM_OBJ(dash_as_second_level_vline, Bool);
    
    FROM_OBJ(bold_line_level, Int);
    FROM_OBJ(show_station_level, Int);
    FROM_OBJ(show_line_in_station, Bool);
    FROM_OBJ(start_label_height, Int);
    FROM_OBJ(end_label_height, Int);
    FROM_OBJ(table_row_height, Int);
    FROM_OBJ(link_line_height, Int);

    FROM_OBJ(auto_paint, Bool);
    FROM_OBJ_NAME(show_full_train_name, showFullCheci, Bool);

    FROM_OBJ(show_time_mark, Int);
    FROM_OBJ(max_passed_stations, Int);

    FROM_OBJ(avoid_cover, Bool);
    FROM_OBJ(base_label_height, Int);
    FROM_OBJ(step_label_height, Int);

    FROM_OBJ(default_grid_width, Double);
    FROM_OBJ(bold_grid_width, Double);
    FROM_OBJ(valid_width, Int);

    FROM_OBJ_NAME(end_label_name, end_label_checi, Bool);

    FROM_OBJ(show_mile_bar, Bool);
    FROM_OBJ(show_ruler_bar, Bool);
    FROM_OBJ(show_count_bar, Bool);

    margins.fromJson(obj.value("margins").toObject());

    not_show_types.clear();
    const QJsonArray& artypes = obj.value("not_show_types").toArray();
    for (const auto& p : artypes) {
        not_show_types.insert(p.toString());
    }
    return true;
}

QJsonObject Config::toJson() const
{
    //先把能够一行转换的写了，其他的再后面插
    QJsonObject obj{
        TO_OBJ(seconds_per_pix)
        TO_OBJ(seconds_per_pix_y)
        TO_OBJ(pixels_per_km)
        TO_OBJ_NAME(default_passenger_width,
            default_keche_width)
        TO_OBJ_NAME(default_freight_width,
            default_huoche_width)
        TO_OBJ(default_db_file)
        TO_OBJ(start_hour)
        TO_OBJ(end_hour)
        TO_OBJ(minutes_per_vertical_line)
        TO_OBJ(minutes_per_vertical_bold)
        TO_OBJ(minutes_per_vertical_second)
        TO_OBJ(dash_as_second_level_vline)
        TO_OBJ(minute_mark_gap_pix)
        TO_OBJ(bold_line_level)
        TO_OBJ(show_station_level)
        TO_OBJ(show_line_in_station)
        TO_OBJ(start_label_height)
        TO_OBJ(end_label_height)
        TO_OBJ(table_row_height)
        TO_OBJ(link_line_height)
        TO_OBJ(auto_paint)
        TO_OBJ_NAME(show_full_train_name,
            showFullCheci)
        TO_OBJ(show_time_mark)
        TO_OBJ(max_passed_stations)
        TO_OBJ(avoid_cover)
        TO_OBJ(base_label_height)
        TO_OBJ(step_label_height)
        TO_OBJ(default_grid_width)
        TO_OBJ(bold_grid_width)
        TO_OBJ(valid_width)
        TO_OBJ_NAME(end_label_name,end_label_checi)
        TO_OBJ(show_mile_bar)
        TO_OBJ(show_ruler_bar)
        TO_OBJ(show_count_bar)
    };
    obj.insert("grid_color", grid_color.name());
    obj.insert("text_color", text_color.name());
    obj.insert("margins", margins.toJson());
    QJsonArray ns;
    for (auto p = not_show_types.begin(); p != not_show_types.end(); ++p) {
        ns.append(*p);
    }
    obj.insert("not_show_types", ns);
    return obj;
}

double Config::diagramWidth() const
{
    int he = end_hour;
    if (he <= start_hour)he += 24;
    return (he - start_hour) * 3600.0 / seconds_per_pix;
}

double Config::totalLeftMargin() const
{
    double res = margins.left;
    if (!show_ruler_bar)
        res -= margins.ruler_label_width;
    if (!show_mile_bar)
        res -= margins.mile_label_width;
    return res;
}

double Config::rulerBarX() const
{
    return margins.left_white;
}

double Config::mileBarX() const
{
    double res = margins.left_white;
    if (show_ruler_bar)res += margins.ruler_label_width;
    return res;
}

double Config::totalRightMargin() const
{
    double res = margins.right;
    if (!show_count_bar)res -= margins.count_label_width;
    return res;
}

double Config::leftRectWidth() const
{
    double res = margins.label_width;
    if (show_ruler_bar)res += margins.ruler_label_width;
    if (show_mile_bar)res += margins.mile_label_width;
    return res;
}

double Config::rightRectWidth() const
{
    double res = margins.label_width;
    if (show_count_bar)res += margins.count_label_width;
    return res;
}

double Config::leftTitleRectWidth() const
{
    double res = 0;
    if (show_mile_bar)res += margins.mile_label_width;
    if (show_ruler_bar)res += margins.ruler_label_width;
    return res;
}

double Config::leftStationBarX() const
{
    double res = margins.left_white;
    if (show_mile_bar)
        res += margins.mile_label_width;
    if (show_ruler_bar)
        res += margins.ruler_label_width;
    return res;
}

