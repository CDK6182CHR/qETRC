#pragma once
#include "data/rail/railcategory.h"

/**
 * @brief The RailDB class
 * RailCategory的顶层封装，用于数据库的根。
 * pyETRC.linedb.LineLib
 */
class RailDB : public RailCategory
{
    QString _filename;
public:
    using RailCategory::RailCategory;

    /**
     * @brief parseJson
     * @param filename
     * 读取文件，同时保存文件名
     */
    bool parseJson(const QString& filename);

    bool save()const;

    /**
     * @brief saveAs
     * @param filename
     * 另存为，同时记录文件名
     */
    bool saveAs(const QString& filename);

    const  auto& filename()const{return _filename;}
};

