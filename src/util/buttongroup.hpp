#pragma once

#include <QHBoxLayout>
#include <QBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <array>
#include <type_traits>
#include <initializer_list>

#include <cassert>


template <size_t _Num, typename _Layout=QHBoxLayout, typename _Button=QPushButton>
class ButtonGroup:
        public _Layout
{
    static_assert (std::is_base_of<QLayout, _Layout>::value, "Invalid Layout");
protected:
    std::array<_Button*,_Num> buttons;
public:
    ButtonGroup(const std::array<const char*,_Num>& labels){
        for(size_t i=0;i<_Num;i++){
            this->buttons[i]=new _Button(QObject::tr(labels[i]));
            this->addWidget(buttons[i]);
        }
    }

    _Button* get(int i)const{return buttons[i];}
    void setMinimumWidth(int w);
    void setFixedWidth(int w);
    void setMaximumWidth(int w);

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

    template <typename _TySignal,typename _TySlot>
    void connectFront(_TySignal _signal, QObject* _target,
        std::initializer_list<_TySlot> _slots)
    {
        assert(_slots.size() <= _Num);
        //static_assert(_slots.size() == _Num, "connectAll: Invalid size of slots");
        auto sl = _slots.begin();
        auto bt = buttons.begin();
        for (; bt != buttons.end()&&sl!=_slots.end(); ++bt,++sl) {
            QObject::connect(*bt, _signal, _target, *sl);
        }
    }

    /**
     * 将每个按钮的_signal都绑定到同一个_slot
     */
    template <typename _TySignal, typename _TySlot>
    void connectAllTo(_TySignal _signal, QObject* _target, _TySlot _slot)
    {
        auto bt = buttons.begin();
        for (; bt != buttons.end() ; ++bt) {
            QObject::connect(*bt, _signal, _target, _slot);
        }
    }
};


template<size_t _Num, typename _Layout, typename _Button>
void ButtonGroup<_Num, _Layout, _Button>::setMinimumWidth(int w)
{
    for(size_t i=0;i<_Num;i++)
        buttons[i]->setMinimumWidth(w);
}

template<size_t _Num, typename _Layout, typename _Button>
void ButtonGroup<_Num, _Layout, _Button>::setFixedWidth(int w)
{
    for(size_t i=0;i<_Num;i++)
        buttons[i]->setFixedWidth(w);
}

template<size_t _Num, typename _Layout, typename _Button>
void ButtonGroup<_Num, _Layout, _Button>::setMaximumWidth(int w)
{
    for(size_t i=0;i<_Num;i++)
        buttons[i]->setMaximumWidth(w);
}


/**
 * 增加添加到QButtonGroup的逻辑。
 * 2022.03.14：增加group()接口；且其中Button的ID保证从0开始。
 */
template <size_t _Num, typename _Layout = QHBoxLayout, typename _Button = QRadioButton>
class RadioButtonGroup :
    public ButtonGroup<_Num, _Layout, _Button>
{
    QButtonGroup* _group;
public:
    RadioButtonGroup(const std::array<const char*, _Num>& labels, QWidget* parent) :
        ButtonGroup<_Num,_Layout,_Button>(labels), _group(new QButtonGroup(parent))
    {
        for (size_t i = 0; i < _Num; i++) {
            _group->addButton(this->buttons[i], i);
        }
    }

    auto* group(){return _group;}
};
