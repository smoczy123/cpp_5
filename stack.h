
#include <utility>
#include <stack>
#include <map>
#include <list>

namespace cxx {
    template <typename K, typename V>
    class stack {
    private:
        struct stack_element_t;
        struct list_element_t;
        struct list_element_t{
            typename std::map<K, std::stack<stack_element_t>>::iterator it;

            list_element_t();
            list_element_t(K& key, std::map<K, std::stack<stack_element_t>>& map) {
                it = map.find(key);
            }
        };
        struct stack_element_t {
            V value;
            typename std::list<list_element_t>::iterator it;

            stack_element_t();
            stack_element_t(V& val, std::list<list_element_t>& list) {
                value = val;
                it = list.begin();
            }
        };
        std::list<list_element_t> actual_stack;
        std::map<K, std::stack<stack_element_t>> access_map;
    public:
        stack() noexcept {
            actual_stack = std::list<map_iter>();
            access_map = std::map<K, std::stack<std::pair<list_iter, V>>>();
        }

        stack(stack const &) noexcept;

        stack(stack &&) noexcept;

        stack & operator=(stack);

        void push(K const & key, V const & value) {
            if (access_map.count(key) == 0) {
                access_map.insert({key, std::stack<std::pair<list_iter, V>>()});
            }

            actual_stack.push_front(access_map.find(key));
            access_map.at(key).push({actual_stack.begin(), value});
        }

        void pop() {
            auto iter = actual_stack.front();
            actual_stack.pop_front();
            iter->second.pop();
        }

        void pop(K const & key) {
            std::pair<list_iter, V> pair = access_map.at(key).top();
            actual_stack.erase(pair.first);
            access_map.at(key).pop();
        }

        std::pair<K const &, V &> front() {
            auto iter = actual_stack.front();
            return std::pair<K const &, V&>({iter.first, iter.second.top().second});
        }
        std::pair<K const &, V const &> front() const {
            auto iter = actual_stack.front();
            return std::pair<K const &, V const &>({iter.first, iter.second.top().second});
        }

        V & front(K const & key) {
            return access_map.at(key).top();
        }
        V const & front(K const & key) const {
            return access_map.at(key).top();
        }

        std::size_t size() const {
            return actual_stack.size();
        }

        std::size_t count(K const & key) const {
            return access_map.at(key).size();
        }

        void clear() {
            actual_stack.clear();
            access_map.clear();
        }



        class const_iterator {
            using iterator_category = std::forward_iterator_tag;
            using value_type = K;
            using pointer = K*;
            using reference = K&;
            cmap_iter iter;

            const_iterator(cmap_iter i) : iter(i) {}

        public:
            const_iterator &operator++() noexcept {
                iter++;
                return *this;
            }

            const_iterator operator++(int) noexcept {
                const_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            reference operator*() const {
                return iter->first;
            }

            pointer operator->() const {
                return &(iter->first);
            }
            friend bool operator== (const const_iterator& a, const const_iterator& b) { return a.iter == b.iter; };
            friend bool operator!= (const const_iterator& a, const const_iterator& b) { return a.iter != b.iter; };
        };

        stack<K, V>::const_iterator cbegin() {
            return const_iterator(access_map.cbegin());
        }
        stack<K, V>::const_iterator cend() {
            return const_iterator(access_map.cend());
        }
    };
}