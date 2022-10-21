#pragma once
#include <QAction>
#include <type_traits>

namespace qeutil{

/**
 * 2022.10.21  create and return a QAction,
 * with given parent and also add to parent.
 * Also connect to given slot
 */
template <
        typename _Ty,
        typename _Func,
        typename=std::enable_if_t<std::is_base_of_v<QObject, _Ty>>,
        typename=std::void_t<
            decltype (QObject::connect((QAction*)nullptr, &QAction::triggered,
            (_Ty*)nullptr, std::declval<_Func>))>
>
QAction* make_action_at(QWidget* parent,
                        const QString& text,
                        _Ty* slot_obj,
                        _Func&& slot_func
                        )
{
    auto* act=new QAction(text, parent);
    parent->addAction(act);
    QObject::connect(act, &QAction::triggered,
                     slot_obj, slot_func);
    return act;
}

}
