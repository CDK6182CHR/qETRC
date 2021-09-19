#pragma once

#include <QList>
#include <memory>
#include <QString>
#include <QJsonObject>

class Railway;

/**
 * @brief The RailCategory class
 * 一组列车的集合。一张运行图Diagram中可以有一个本对象。
 * DiagramPage中暂定还是直接QList
 * 暂时简单的实现为一个容器
 * 主要动因是在Railway基本信息编辑的窗口里检测线名修改的合法性
 * 
 * 2021.09.13：进一步支持带子类的情况。为线路数据库铺路。
 * 暂定不带parent指针看行不行。这样是为了方便Trivial的移动构造等。
 */
class RailCategory: public std::enable_shared_from_this<RailCategory>
{
    QString _name;
    //std::weak_ptr<RailCategory> _parent;
    QList<std::shared_ptr<RailCategory>> _subcats;
    QList<std::shared_ptr<Railway>> _railways;

    /**
     * 浅拷贝的构造函数，作为private访问。
     * 目前仅允许用于导出子数据库操作。
     */
    explicit RailCategory(const RailCategory& other) = default;
public:
    RailCategory()=default;
    RailCategory(const QString& name);
    RailCategory(RailCategory&&)=default;
    RailCategory& operator=(const RailCategory&) = delete;
    RailCategory& operator=(RailCategory&&)=default;

    //auto parent() { return _parent; }

    auto& railways(){return _railways;}
    const auto& railways()const{return _railways;}
    auto& subCategories() { return _subcats; }
    const auto& subCategories()const { return _subcats; }
    const auto& name()const { return _name; }
    void setName(const QString& name) { _name = name; }
    auto& nameRef() { return _name; }

    bool railNameIsValid(const QString& name, std::shared_ptr<const Railway> rail)const;
    bool categoryNameIsValid(const QString& name, std::shared_ptr<const RailCategory> cat)const;

    /**
     * seealso railNameIsValid
     * 但这个版本递归所有子类
     */
    bool railNameIsValidRec(const QString& name, std::shared_ptr<const Railway> rail)const;
    bool categoryNameIsValidRec(const QString& name, std::shared_ptr<const RailCategory> cat)const;

    QString validRailwayName(const QString& prefix)const;
    QString validRailwayNameRec(const QString& prefix)const;

    QString validCategoryNameRec(const QString& prefix)const;

    int getRailwayIndex(std::shared_ptr<const Railway> rail)const;

    int getRailwayIndex(const Railway& rail)const;

    void fromJson(const QJsonObject& obj);

    QJsonObject toJson()const;

    void clear();

    bool isNull()const;

    /**
     * 浅拷贝操作  仅复制了指针
     * 慎用！
     */
    RailCategory shallowCopy()const;

private:
    /**
     * pyETRC.lineDB.Category.isLineDict
     * 判定Json输入的类型
     */
    static bool isRailway(const QJsonObject& obj);
};

