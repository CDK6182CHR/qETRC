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
    template <typename _Key, typename _VData, typename _EData>
    class di_graph {
    public:
        using key_type = _Key;
        using vertex_data_type = _VData;
        using edge_data_type = _EData;

        struct edge;

        struct vertex {
            _VData data;
            std::shared_ptr<edge> in_edge;   // 入边链表
            std::shared_ptr<edge> out_edge;   // 出边链表

            template <class = std::enable_if_t<std::is_default_constructible_v<_VData>>>
            vertex():data() {}
            vertex(const _VData& data) :data(data) {}
            vertex(_VData&& data): data(std::forward<_VData>(data)) {}

            template <typename... Args>
            vertex(Args&&... args):data(std::forward<Args>(args)...){}
        };

        struct edge {
            _EData data;
            std::weak_ptr<vertex> from, to;
            std::shared_ptr<edge> next_out;   // 下一出边
            std::shared_ptr<edge> next_in;    // 下一入边

            template <class = std::enable_if_t<std::is_default_constructible_v<_EData>>>
            edge(std::weak_ptr<vertex> from,std::weak_ptr<vertex> to):data(),from(from),to(to) {}
            edge(std::weak_ptr<vertex> from, std::weak_ptr<vertex> to,
                const edge_data_type& data) :from(from),to(to),  data(data) {}
            edge(std::weak_ptr<vertex> from, std::weak_ptr<vertex> to,
                edge_data_type&& data):from(from),to(to), data(std::forward<edge_data_type>(data)) {}
            
            template <typename... Args>
            edge(const std::weak_ptr<vertex>& from, const std::weak_ptr<vertex>& to,
                Args&&... args):from(from),to(to),
                data(std::forward<Args>(args)...){}
        };

    private:
        std::map<key_type, std::shared_ptr<vertex>> _vertices;

    public:
        di_graph() = default;

        size_t size()const { return _vertices.size(); }

        auto& vertices() { return _vertices; }
        const auto& vertices()const { return _vertices; }

        std::shared_ptr<vertex> find_vertex(const key_type& key) {
            if (auto itr = _vertices.find(key); itr != _vertices.end())
                return itr->second;
            else return nullptr;
        }

        std::shared_ptr<vertex> insert_vertex(const key_type& key, const _VData& data) {
            auto [itr, _] = _vertices.insert({ key, std::make_shared<vertex>(data) });
            return itr->second;
        }

        template <typename = std::enable_if_t<std::is_default_constructible_v<_VData>>>
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
            const _EData& data) {
            auto e = std::make_shared<edge>(from, to, data);
            e->next_in = to->in_edge;
            to->in_edge = e;
            e->next_out = from->out_edge;
            from->out_edge = e;
            return e;
        }

        std::shared_ptr<edge> insert_edge(const std::shared_ptr<vertex>& from,
            const std::shared_ptr<vertex>& to,
            _EData&& data) {
            auto e = std::make_shared<edge>(from, to, std::forward<_EData>(data));
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

        void clear(){
            _vertices.clear();
        }


        template <typename _Val>
        struct sssp_ret_t {
            std::unordered_map<std::shared_ptr<vertex>, _Val> distance;
            std::unordered_map<std::shared_ptr<vertex>, std::shared_ptr<edge>> path;
        };


        /**
         * Solve the single-source-shortest-path problem
         * The path is expressed by edge
         * This version is provided for simple type
         * see: https://stackoverflow.com/questions/56117241/no-type-named-type-in-struct-stdenable-iffalse-void
         */
        template <typename _T=_EData, typename=std::enable_if_t<std::is_arithmetic_v<_EData>>>
        sssp_ret_t<_EData> sssp(std::shared_ptr<vertex> source)
        {
            using _Val = _EData;
            constexpr _Val MAX = std::numeric_limits<_Val>::max() - 1;
            sssp_ret_t<_Val> ret{};

            std::unordered_set<std::shared_ptr<vertex>> si;
            ret.distance.emplace(source, (_Val)0);

            // 注：数值为空表示不可达/无穷大
            for (int i = 0; i < size(); i++) {
                auto mi = get_min_dist<_Val>(ret, si, MAX);
                if (mi) {
                    si.insert(mi);
                    for (auto e = mi->out_edge; e; e = e->next_out) {
                        auto mj = e->to.lock();
                        _Val dnew = ret.distance.at(mi) + e->data;
                        if (auto itr = ret.distance.find(mj);
                            itr == ret.distance.end() || dnew<itr->second) {
                            // 原来无路径，或是新的路径更短
                            ret.distance[mj] = dnew;
                            ret.path[mj] = e;
                        }
                    }
                }
                else {
                    // 没有可达的了
                    break;
                }
            }

            return ret;
        }

        template <typename _Func,
            typename _Val= decltype(std::declval<_Func>()(std::declval<_EData>())),
            typename =std::enable_if_t<std::is_arithmetic_v<_Val>>
        >
        sssp_ret_t<_Val> sssp(std::shared_ptr<vertex> source, _Func func)
        {
            constexpr _Val MAX = std::numeric_limits<_Val>::max() - 1;
            sssp_ret_t<_Val> ret{};

            std::unordered_set<std::shared_ptr<vertex>> si;
            ret.distance.emplace(source, (_Val)0);

            // 注：数值为空表示不可达/无穷大
            for (int i = 0; i < size(); i++) {
                auto mi = get_min_dist<_Val>(ret, si, MAX);
                if (mi) {
                    si.insert(mi);
                    for (auto e = mi->out_edge; e; e = e->next_out) {
                        auto mj = e->to.lock();
                        _Val dnew = ret.distance.at(mi) + func(e->data);
                        if (auto itr = ret.distance.find(mj);
                            itr == ret.distance.end() || dnew < itr->second) {
                            // 原来无路径，或是新的路径更短
                            ret.distance[mj] = dnew;
                            ret.path[mj] = e;
                        }
                    }
                }
                else {
                    // 没有可达的了
                    break;
                }
            }

            return ret;
        }


        /**
         * 由sssp计算结果给出路径，以边的序列表示。
         * 如果不可达或者source, target一样，返回空
         */
        template <typename _Val>
        std::deque<std::shared_ptr<edge>>
                dump_path(const std::shared_ptr<vertex>& source,
                          const std::shared_ptr<vertex>& target,
                          const sssp_ret_t<_Val>& res)
        {
            if (auto itr=res.distance.find(target);itr==res.distance.end()){
                return {};
            }
            std::deque<std::shared_ptr<edge>> path;
            std::shared_ptr<vertex> cur=target;
            while(cur!=source){
                if (auto itr=res.path.find(cur);itr!=res.path.end()){
                    // 找到上一步的路径
                    path.emplace_front(itr->second);
                    cur=itr->second->from.lock();
                }else{
                    break;
                }
            }
            if(cur==source){
                // 成功找到路径
                return path;
            }else{
                return {};
            }
        }

    private:

        template <typename _Val>
        std::shared_ptr<vertex> get_min_dist(
            const sssp_ret_t<_Val>& ret,
            const std::unordered_set<std::shared_ptr<vertex>>& si,
            _Val MAX)
        {
            _Val curmin = MAX;
            std::shared_ptr<vertex> cur{};
            for (auto p = ret.distance.begin(); p != ret.distance.end(); ++p) {
                if (auto itr = si.find(p->first); itr == si.end()) {
                    // note not in si
                    if (p->second < curmin) {
                        curmin = p->second;
                        cur = p->first;
                    }
                }
            }
            return cur;
        }
    };
}

