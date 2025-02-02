#pragma once

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <deque>
#include <memory>
#include <type_traits>

namespace xtl
{
    /**
     * 多重邻接表实现有向图。使用映射结构来保存结点。
     * 按STL风格命名
     */
    template <typename Key, typename VData, typename EData>
    class di_graph {
    public:
        using key_type = Key;
        using vertex_data_type = VData;
        using edge_data_type = EData;

        struct edge;

        struct vertex {
            VData data;
            std::shared_ptr<edge> in_edge;   // 入边链表
            std::shared_ptr<edge> out_edge;   // 出边链表

            template <class = std::enable_if_t<std::is_default_constructible_v<VData>>>
            vertex() :data() {}
            vertex(const VData& data) :data(data) {}
            vertex(VData&& data) : data(std::forward<VData>(data)) {}

            template <typename... Args>
            vertex(Args&&... args) : data(std::forward<Args>(args)...) {}

            int out_degree()const;
            int in_degree()const;
        };

        struct edge {
            EData data;
            std::weak_ptr<vertex> from, to;
            std::shared_ptr<edge> next_out;   // 下一出边
            std::shared_ptr<edge> next_in;    // 下一入边

            template <class = std::enable_if_t<std::is_default_constructible_v<EData>>>
            edge(std::weak_ptr<vertex> from, std::weak_ptr<vertex> to) :data(), from(from), to(to) {}
            edge(std::weak_ptr<vertex> from, std::weak_ptr<vertex> to,
                const edge_data_type& data) : data(data), from(from), to(to){}
            edge(std::weak_ptr<vertex> from, std::weak_ptr<vertex> to,
                edge_data_type&& data) :data(std::forward<edge_data_type>(data)),
                from(from), to(to)
                 {}

            template <typename... Args>
            edge(const std::weak_ptr<vertex>& from, const std::weak_ptr<vertex>& to,
                Args&&... args) :  data(std::forward<Args>(args)...),
                from(from), to(to)
                {}
        };

    private:
        std::map<key_type, std::shared_ptr<vertex>> _vertices;

    public:
        di_graph() = default;

        size_t size()const { return _vertices.size(); }
        bool empty()const { return _vertices.empty(); }

        auto& vertices() { return _vertices; }
        const auto& vertices()const { return _vertices; }

        std::shared_ptr<vertex> find_vertex(const key_type& key) {
            if (auto itr = _vertices.find(key); itr != _vertices.end())
                return itr->second;
            else return nullptr;
        }

        std::shared_ptr<const vertex> find_vertex(const key_type& key)const {
            if (auto itr = _vertices.find(key); itr != _vertices.end())
                return itr->second;
            else return nullptr;
        }

        std::shared_ptr<vertex> insert_vertex(const key_type& key, const VData& data) {
            auto [itr, _] = _vertices.insert({ key, std::make_shared<vertex>(data) });
            return itr->second;
        }

        template <typename = std::enable_if_t<std::is_default_constructible_v<VData>>>
        std::shared_ptr<vertex> insert_vertex(const key_type& key) {
            auto [itr, _] = _vertices.insert({ key,std::make_shared<vertex>() });
            return itr->second;
        }

        template <typename _K, typename... Args>
        std::shared_ptr<vertex> emplace_vertex(_K&& key, Args&&... args) {
            auto [itr, _] = _vertices.emplace(std::forward<_K>(key),
                std::make_shared<vertex>(std::forward<Args>(args)...));
            return itr->second;
        }

        std::shared_ptr<edge> insert_edge(std::shared_ptr<vertex> from, std::shared_ptr<vertex> to,
            const EData& data) {
            auto e = std::make_shared<edge>(from, to, data);
            e->next_in = to->in_edge;
            to->in_edge = e;
            e->next_out = from->out_edge;
            from->out_edge = e;
            return e;
        }

        std::shared_ptr<edge> insert_edge(const std::shared_ptr<vertex>& from,
            const std::shared_ptr<vertex>& to,
            EData&& data) {
            auto e = std::make_shared<edge>(from, to, std::forward<EData>(data));
            e->next_in = to->in_edge;
            to->in_edge = e;
            e->next_out = from->out_edge;
            from->out_edge = e;
            return e;
        }

        template <typename... Args>
        std::shared_ptr<edge> emplace_edge(const std::shared_ptr<vertex>& from,
            const std::shared_ptr<vertex>& to, Args&&... args)
        {
            auto e = std::make_shared<edge>(from, to, std::forward<Args>(args)...);
            e->next_in = to->in_edge;
            to->in_edge = e;
            e->next_out = from->out_edge;
            from->out_edge = e;
            return e;
        }

        void clear() {
            _vertices.clear();
        }


        template <typename Val>
        struct sssp_ret_t {
            std::unordered_map<std::shared_ptr<const vertex>, Val> distance;
            std::unordered_map<std::shared_ptr<const vertex>, std::shared_ptr<const edge>> path;
        };

        /**
         * 2021.09.24  尝试第二个版本
         * 将标记完成的集合si改成待定序列的集合，避免反复查找si
         */
        template <typename Func,
            typename Val = decltype(std::declval<Func>()(std::declval<EData>())),
            typename = std::enable_if_t<std::is_arithmetic_v<Val>>
        >
            sssp_ret_t<Val> sssp(std::shared_ptr<const vertex> source, Func func)const;


        /**
         * 2021.09.24  尝试第二个版本
         * 将标记完成的集合si改成待定序列的集合，避免反复查找si
         */
        template <typename Val = EData, typename = std::enable_if_t<std::is_arithmetic_v<Val>>>
        sssp_ret_t<Val> sssp(std::shared_ptr<const vertex> source)const;


        using path_t = std::deque<std::shared_ptr<const edge>>;

        /**
         * 由sssp计算结果给出路径，以边的序列表示。
         * 如果不可达或者source, target一样，返回空
         */
        template <typename Val>
        std::deque<std::shared_ptr<const edge>>
            dump_path(const std::shared_ptr<const vertex>& source,
                const std::shared_ptr<const vertex>& target,
                const sssp_ret_t<Val>& res)const
        {
            if (auto itr = res.distance.find(target); itr == res.distance.end()) {
                return {};
            }
            std::deque<std::shared_ptr<const edge>> path;
            std::shared_ptr<const vertex> cur = target;
            while (cur != source) {
                if (auto itr = res.path.find(cur); itr != res.path.end()) {
                    // 找到上一步的路径
                    path.emplace_front(itr->second);
                    cur = itr->second->from.lock();
                }
                else {
                    break;
                }
            }
            if (cur == source) {
                // 成功找到路径
                return path;
            }
            else {
                return {};
            }
        }

    private:

        template <typename Val>
        std::shared_ptr<const vertex> get_min_dist(
            const std::unordered_map<std::shared_ptr<const vertex>, Val>& choice,
            Val MAX)const
        {
            Val curmin = MAX;
            std::shared_ptr<const vertex> cur{};
            for (auto p = choice.begin(); p != choice.end(); ++p) {
                if (p->second < curmin) {
                    curmin = p->second;
                    cur = p->first;
                }
            }
            return cur;
        }
    };

    template<typename Key, typename VData, typename EData>
    inline int di_graph<Key, VData, EData>::vertex::out_degree() const
    {
        int res = 0;
        auto e = out_edge;
        while (e) {
            e = e->next_out;
            res++;
        }
        return res;
    }

    template<typename Key, typename VData, typename EData>
    inline int di_graph<Key, VData, EData>::vertex::in_degree() const
    {
        int res = 0;
        auto e = in_edge;
        while (e) {
            e = e->next_in;
            res++;
        }
        return res;
    }


    template<typename Key, typename VData, typename EData>
    template<typename Func, typename Val, typename>
    typename di_graph<Key, VData, EData>::template sssp_ret_t<Val>
        di_graph<Key, VData, EData>::sssp(
            std::shared_ptr<const vertex> source,
            Func func) const
    {
        constexpr Val MAX = std::numeric_limits<Val>::max() - 1;
        sssp_ret_t<Val> ret{};

        // 候选项：可达但还没被固定下来的结点及其距离
        std::unordered_map<std::shared_ptr<const vertex>, Val> choice;
        ret.distance.emplace(source, (Val)0);
        choice.emplace(source, (Val)0);

        // 注：数值为空表示不可达/无穷大
        for (size_t i = 0; i < size(); i++) {
            auto mi = get_min_dist<Val>(choice, MAX);
            if (mi) {
                // 此节点被固定
                for (auto e = mi->out_edge; e; e = e->next_out) {
                    auto mj = e->to.lock();
                    Val dnew = ret.distance.at(mi) + func(e->data);
                    if (auto itr = ret.distance.find(mj);
                        itr == ret.distance.end() || dnew < itr->second) {
                        // 原来无路径，或是新的路径更短
                        choice[mj] = dnew;
                        ret.distance[mj] = dnew;
                        ret.path[mj] = e;
                    }
                }
                choice.erase(mi);
            }
            else {
                // 没有可达的了
                break;
            }
        }
        return ret;
    }


    template<typename Key, typename VData, typename EData>
    template<typename Val, typename>
    typename di_graph<Key, VData, EData>::template sssp_ret_t<Val>
        di_graph<Key, VData, EData>::sssp(std::shared_ptr<const vertex> source) const
    {
        constexpr Val MAX = std::numeric_limits<Val>::max() - 1;
        sssp_ret_t<Val> ret{};

        // 候选项：可达但还没被固定下来的结点及其距离
        std::unordered_map<std::shared_ptr<const vertex>, Val> choice;
        ret.distance.emplace(source, (Val)0);
        choice.emplace(source, (Val)0);

        // 注：数值为空表示不可达/无穷大
        for (int i = 0; i < size(); i++) {
            auto mi = get_min_dist<Val>(choice, MAX);
            if (mi) {
                // 此节点被固定
                for (auto e = mi->out_edge; e; e = e->next_out) {
                    auto mj = e->to.lock();
                    Val dnew = ret.distance.at(mi) + e->data;
                    if (auto itr = ret.distance.find(mj);
                        itr == ret.distance.end() || dnew < itr->second) {
                        // 原来无路径，或是新的路径更短
                        choice[mj] = dnew;
                        ret.distance[mj] = dnew;
                        ret.path[mj] = e;
                    }
                }
                choice.erase(mi);
            }
            else {
                // 没有可达的了
                break;
            }
        }

        return ret;
    }

}
