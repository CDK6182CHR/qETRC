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
}



void Config::fromJson(const QJsonObject& obj)
{
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
    
    FROM_OBJ(bold_line_level, Int);
    FROM_OBJ(show_line_in_station, Bool);
    FROM_OBJ(start_label_height, Int);
    FROM_OBJ(end_label_height, Int);
    FROM_OBJ(table_row_height, Int);
    FROM_OBJ(link_line_height, Int);

    FROM_OBJ(show_time_mark, Int);
    FROM_OBJ(max_passed_stations, Int);

    FROM_OBJ(avoid_cover, Bool);
    FROM_OBJ(base_label_height, Int);
    FROM_OBJ(step_label_height, Int);

    FROM_OBJ(default_grid_width, Double);
    FROM_OBJ(bold_grid_width, Double);

    FROM_OBJ(auto_paint, Bool);

    FROM_OBJ_NAME(end_label_name, end_label_checi, Bool);

    margins.fromJson(obj.value("margins").toObject());

    not_show_types.clear();
    const QJsonArray& artypes = obj.value("not_show_types").toArray();
    for (const auto& p : artypes) {
        not_show_types.insert(p.toString());
    }
}

