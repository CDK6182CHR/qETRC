/*
 * 行别枚举类
 */
#pragma once

#include <cstdint>

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
}

