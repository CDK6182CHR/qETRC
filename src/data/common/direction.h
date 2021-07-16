/*
 * 行别枚举类
 */
#pragma once

#include <cstdint>
#include <QString>
#include <QObject>

enum class Direction : std::int8_t {
	Down, Up, Undefined
};

namespace DirFunc {
	inline constexpr Direction reverse(Direction d) {
		switch (d)
		{
		case Direction::Down:
			return Direction::Up;
		case Direction::Up:
			return Direction::Down;
		default:
			return Direction::Undefined;
		}
	}

	inline constexpr bool isDown(Direction d) {
		return d == Direction::Down;
	}

	inline constexpr bool isValid(Direction d) {
		return d != Direction::Undefined;
	}

	inline constexpr Direction fromIsDown(bool isDown) {
		return isDown ? Direction::Down:Direction::Up;
	}

    inline QString dirToString(Direction dir){
        switch (dir) {
        case Direction::Down:return QObject::tr("下行");
        case Direction::Up:return QObject::tr("上行");
        default: return QObject::tr("无效方向");
        }
    }
}

