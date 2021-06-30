#pragma once

#include <QHBoxLayout>
#include <QBoxLayout>
#include <QPushButton>
#include <array>
#include <type_traits>
#include <initializer_list>

#include <cassert>


template <size_t _Num, typename _Layout=QHBoxLayout, typename _Button=QPushButton>
class ButtonGroup:
        public _Layout
{
    static_assert (std::is_base_of<QLayout,_Layout>::value,"Invalid Layout");
    std::array<_Button*,_Num> buttons;
public:
    //ButtonGroup(const std::array<QString,_Num>& labels);
    ButtonGroup(const std::array<const char*,_Num>& labels);

    _Button* get(int i)const{return buttons[i];}
    void setMinimumWidth(int w);

    /**
     * 将每个按钮的_signal分别绑定到target的_slots[i]
     */
    template <typename _TySignal,typename _TySlot>
    void connectAll(_TySignal _signal, QObject* _target,
        std::initializer_list<_TySlot> _slots)
    {
        assert(_slots.size() == _Num);
        //static_assert(_slots.size() == _Num, "connectAll: Invalid size of slots");
        auto sl = _slots.begin();
        auto bt = buttons.begin();
        for (; bt != buttons.end()&&sl!=_slots.end(); ++bt,++sl) {
            QObject::connect(*bt, _signal, _target, *sl);
        }
    }
};

//template<size_t _Num, typename _Layout, typename _Button>
//ButtonGroup<_Num, _Layout, _Button>::ButtonGroup(const std::array<QString, _Num> &labels)
//{
//    for(int i=0;i<_Num;i++){
//        buttons[i]=new _Button(labels[i]);
//        addWidget(buttons[i]);
//    }
//}

template<size_t _Num, typename _Layout, typename _Button>
ButtonGroup<_Num, _Layout, _Button>::ButtonGroup(const std::array<const char *, _Num> &labels)
{
    for(int i=0;i<_Num;i++){
        buttons[i]=new _Button(QObject::tr(labels[i]));
        addWidget(buttons[i]);
    }
}

template<size_t _Num, typename _Layout, typename _Button>
void ButtonGroup<_Num, _Layout, _Button>::setMinimumWidth(int w)
{
    for(int i=0;i<_Num;i++)
        buttons[i]->setMinimumWidth(w);
}


