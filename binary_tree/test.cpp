#include <cstdio>
#include <chrono>
#include <vector>
#include <random>
#include <memory>
#include <functional>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <thread>
#include <utility>
#include <stack>


using namespace std::chrono_literals;


struct rng_t
{
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> distrib;

    rng_t() :
        gen(rd()),
        distrib(0.0, 1.0)
    {}

    void register_queue(void* handler)
    {
        queues.emplace(handler, std::queue<float>());
    }

    float next(void* handler)
    {
        auto& q = queues.find(handler)->second;
        if (q.size() == 0) {
            for (int i = 0; i < 100; i++) {
                float v = distrib(gen);
                for (auto& [_, out_q] : queues) {
                    out_q.push(v);
                }
            }
        }
        float v = q.front();
        q.pop();
        return v;
    }

    std::unordered_map<void*, std::queue<float>> queues;
};



template<typename T>
struct btree_node
{
    btree_node<T>* left = nullptr;
    btree_node<T>* right = nullptr;
    T value;

    template<class NodeT, void (*f) (NodeT*)>
    void traverse()
    {
        if (left) {
            static_cast<NodeT*>(left)->template traverse<NodeT, f>();
        }
        f(static_cast<NodeT*>(this));
        if (right) {
            static_cast<NodeT*>(right)->template traverse<NodeT, f>();
        }
    }

    T sum()
    {
        T s = value;
        if (left) {
            s += left->sum();
        }
        if (right) {
            s += right->sum();
        }
        return s;
    }
};

template<typename T>
T btree_sum(const btree_node<T>* node)
{
    std::stack<std::pair<const btree_node<T>*, int>> stack;

    T S = 0;
    int step = 0;
    while(true) {
        if (step == 0) {
            if (node->left) {
                stack.push({node, 1});
                node = node->left;
                step = 0;
                continue;
            } else {
                step = 1;
            }
        }
        if (step == 1) {
            if (node->right) {
                stack.push({node, 2});
                node = node->right;
                step = 0;
                continue;
            } else {
                step = 2;
            }
        }

        S += node->value;

        if (stack.size() == 0) {
            break;
        }

        node = stack.top().first;
        step = stack.top().second;
        stack.pop();
    }

    return S;
}


template<typename T>
struct b3_node : public btree_node<T>
{
    b3_node<T>* parent = nullptr;
};


template<typename T>
T b3_sum(const b3_node<T>* node)
{
    T S = 0;

    int step = 0;

    while (node) {

        if (step == 0) {
            if (node->left) {
                node = static_cast<const b3_node<T>*>(node->left);
            } else {
                step = 1;
            }
        }

        if (step == 1) {
            if (node->right) {
                step = 0;
                node = static_cast<const b3_node<T>*>(node->right);
            } else {
                step = 2;
            }
        }

        if (step == 2) {
            S += node->value;

            if (node->parent) {
                if (node == node->parent->left) {
                    step = 1;
                }
                if (node == node->parent->right) {
                    step = 2;
                }
            }

            node = node->parent;
        }
    }

    return S;
}



typedef btree_node<uint64_t> btree_node_t;
typedef b3_node<uint64_t> b3_node_t;


template<class T>
struct tree_gen
{
    tree_gen(rng_t& r) : r(r)
    {
        r.register_queue(this);
    }

    rng_t& r;

    std::vector<std::unique_ptr<T>> nodes;
    float single = 1.0;
    float empty = 1.0;
    float decay = 1.0;

    size_t depth = 0;
    size_t max_depth = 0;
    size_t depth_limit = 1000;
    int node_id = 0;

    std::queue<float> randoms;

    float next()
    {
        return r.next(this);
    }

    T* generate(T* parent)
    {
        if (depth >= depth_limit) {
            return nullptr;
        }

        depth++;
        if (max_depth < depth) {
            max_depth = depth;
        }

        nodes.emplace_back(std::make_unique<T>());
        T* node = nodes.back().get();

        if constexpr (std::is_same_v<T, b3_node_t>) {
            node->parent = parent;
        }

        float v = next();
        node->value = node_id++;//v * 1000'000'000;

        float depth_denom = depth*decay/depth_limit;
        //printf("%lu %f %lu\n", depth, depth_denom, nodes.size());

        if (v > (single + depth_denom)) {

            node->left = generate(node);
            node->right = generate(node);

        } else if (v > (empty + depth_denom)) {

            if (next() >= 0.5) {
                node->left = generate(node);
            } else {
                node->right = generate(node);
            }
        }

        depth--;
        return node;
    }

    auto generate(float single_prob, float empty_prob, float _decay,
        size_t _depth_limit, size_t min_nodes)
    {
        single = single_prob;
        empty = empty_prob;
        depth_limit = _depth_limit;
        decay = _decay;

        while (nodes.size() < min_nodes) {
            depth = 0;
            max_depth = 0;
            node_id = 0;
            nodes.clear();

            generate(nullptr);
        }

        return std::move(nodes);
    }
};


void print_arrow(const void* from, const void* to)
{
    printf("_0x%p -> _0x%p\n", from, to);
}

void print_node(btree_node_t* node)
{
    if (node->left) {
        print_arrow(node, node->left);
    }
    if (node->right) {
        print_arrow(node, node->right);
    }
}

void print_node(b3_node_t* node)
{
    if (node->parent) {
        print_arrow(node, node->parent);
    }
    print_node(static_cast<btree_node_t*>(node));
}


template<class T>
void print_tree(std::vector<std::unique_ptr<T>>& nodes)
{
    for (const auto& ptr : nodes) {
        printf("_0x%p [label=\"%lu\"]\n", ptr.get(), ptr->value);
    }

    nodes[0]->template traverse<T, print_node>();
}



using fsecs_t = std::chrono::duration<double, std::chrono::seconds::period>;

int main(int argc, const char* argv[])
{
    bool dot = false;

    float single_prob = std::strtof(argv[1], 0);
    float empty_prob = std::strtof(argv[2], 0);
    float decay = std::strtof(argv[3], 0);
    size_t depth_limit = std::strtol(argv[4], 0, 0);

    rng_t rng;
    tree_gen<btree_node_t> btree_gen(rng);
    tree_gen<b3_node_t> b3_gen(rng);

    auto btree_nodes = btree_gen.generate(single_prob, empty_prob, decay, depth_limit, 3);
    auto b3_nodes = b3_gen.generate(single_prob, empty_prob, decay, depth_limit, 3);

    if (dot) {
        printf("digraph G {\n");
        print_tree(btree_nodes);
        print_tree(b3_nodes);
        printf("}\n");
    } else {
        printf("btree: nodes: %lu, max_depth: %lu\n", btree_nodes.size(), btree_gen.max_depth);
        printf("b3: nodes: %lu, max_depth: %lu\n", b3_nodes.size(), b3_gen.max_depth);

        auto t_start = std::chrono::steady_clock::now();
        uint64_t sum = btree_nodes[0]->sum();
        auto t_stop = std::chrono::steady_clock::now();
        double dt = std::chrono::duration_cast<fsecs_t>(t_stop - t_start).count();
        printf("btree recursive traverse: T=%fms, t=%fns/node, sum=%lu\n",
            dt*1000, dt*1000'000'000/btree_nodes.size(), sum);

        t_start = std::chrono::steady_clock::now();
        sum = btree_sum(btree_nodes[0].get());
        t_stop = std::chrono::steady_clock::now();
        dt = std::chrono::duration_cast<fsecs_t>(t_stop - t_start).count();
        printf("btree cycle traverse: T=%fms, t=%fns/node, sum=%lu\n",
            dt*1000, dt*1000'000'000/btree_nodes.size(), sum);

        t_start = std::chrono::steady_clock::now();
        sum = b3_sum(b3_nodes[0].get());
        t_stop = std::chrono::steady_clock::now();
        dt = std::chrono::duration_cast<fsecs_t>(t_stop - t_start).count();
        printf("b3 cycle traverse: T=%fms, t=%fns/node, sum=%lu\n",
            dt*1000, dt*1000'000'000/b3_nodes.size(), sum);
    }



    return 0;
}