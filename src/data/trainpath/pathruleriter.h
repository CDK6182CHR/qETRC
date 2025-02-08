#pragma once

#include <memory>

class PathRuler;
class RulerNode;

/**
 * 2025.02.08  experimental
 * Stateful iterator for a PathRuler, each step for a RailInterval.
 * Standard way for iterating a path ruler.
 * Precondition: the PathRuler is valid and never expired during the iteration.
 * Typically, do not store this iterator!
 * The value type is std::shared_ptr<RulerNode> (is this proper?)
 * The end() iterator is that m_seg_index == m_ruler->segments().size() and m_cur_node == nullptr.
 */
class PathRulerIterator
{
	PathRuler* m_ruler;
	size_t m_seg_index;
	std::shared_ptr<RulerNode> m_cur_node;

	friend class PathRuler;

	/**
	 * Full-data constructor, for PathRuler only
	 */
	PathRulerIterator(PathRuler* ruler, size_t seg_index, std::shared_ptr<RulerNode> cur_node);

public:
	using value_type = std::shared_ptr<RulerNode>;

	PathRulerIterator();

	PathRulerIterator(const PathRulerIterator&) = default;
	PathRulerIterator(PathRulerIterator&&)noexcept = default;

	PathRulerIterator& operator=(const PathRulerIterator&) = default;
	PathRulerIterator& operator=(PathRulerIterator&&)noexcept = default;
	
	value_type& operator*() { return m_cur_node; }
	const value_type& operator*()const { return m_cur_node; }

	/**
	 * Postfix increacemental operation. The iterator must be valid.
	 * (i.e., not end() and not default-constructed one)
	 */
	PathRulerIterator& operator++();
	PathRulerIterator operator++(int);

	PathRulerIterator& operator--();
	PathRulerIterator operator--(int);

	bool operator==(const PathRulerIterator& rhs)const;
	bool operator!=(const PathRulerIterator& rhs)const;

private:

	/**
	 * Switch to the next step. Internally called only.
	 */
	void next();

	void prev();

	/**
	 * Go to the first node of the segment specified by m_cur_index.
	 */
	void toSegFirstNode();

	void toSegLastNode();

	void checkSameRuler(const PathRulerIterator& rhs)const;
};