#pragma once

#include <memory>
#include <cassert>
#include <type_traits>

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
 * The template argument TyNode may be RulerNode or const-qualified node.
 */
template <typename TyNode>
class PathRulerIteratorGen
{
	static_assert(std::is_same_v<std::remove_const_t<TyNode>, RulerNode>,
		"PathRulerIteratorGen is only provided for ruler node");

	using TyPathRuler = std::conditional_t<std::is_const_v<TyNode>, const PathRuler, PathRuler>;

	TyPathRuler* m_ruler;
	size_t m_seg_index;
	std::shared_ptr<TyNode> m_cur_node;

	friend class PathRuler;

	/**
	 * Full-data constructor, for PathRuler only
	 */
	PathRulerIteratorGen(TyPathRuler* ruler, size_t seg_index, std::shared_ptr<TyNode> cur_node);

	static PathRulerIteratorGen<TyNode> makeBegin(TyPathRuler* ruler);
	static PathRulerIteratorGen<TyNode> makeEnd(TyPathRuler* ruler);
	

public:
	using value_type = std::shared_ptr<TyNode>;

	PathRulerIteratorGen();

	PathRulerIteratorGen(const PathRulerIteratorGen<TyNode>&) = default;
	PathRulerIteratorGen(PathRulerIteratorGen<TyNode>&&)noexcept = default;

	PathRulerIteratorGen<TyNode>& operator=(const PathRulerIteratorGen<TyNode>&) = default;
	PathRulerIteratorGen<TyNode>& operator=(PathRulerIteratorGen<TyNode>&&)noexcept = default;

	template <typename TyNodeSelf = TyNode,
		typename = std::enable_if_t<!std::is_const_v<TyNodeSelf>>>
	operator PathRulerIteratorGen<const TyNodeSelf>()
	{
		return *static_cast<PathRulerIteratorGen<const TyNodeSelf*>>(this);
	}
	
	value_type& operator*() { return m_cur_node; }
	const value_type& operator*()const { return m_cur_node; }

	/**
	 * Postfix increacemental operation. The iterator must be valid.
	 * (i.e., not end() and not default-constructed one)
	 */
	PathRulerIteratorGen& operator++();
	PathRulerIteratorGen operator++(int);

	PathRulerIteratorGen& operator--();
	PathRulerIteratorGen operator--(int);

	bool operator==(const PathRulerIteratorGen<TyNode>& rhs)const;
	bool operator!=(const PathRulerIteratorGen<TyNode>& rhs)const;

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

	template <typename TyNode2>
	void checkSameRuler([[maybe_unused]] const PathRulerIteratorGen<TyNode2>& rhs)const {
		assert(m_ruler == rhs.m_ruler);
	}
};

using PathRulerMutableIterator = PathRulerIteratorGen<RulerNode>;
using PathRulerConstIterator = PathRulerIteratorGen<const RulerNode>;
