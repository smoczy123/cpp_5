
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

        struct list_element_t {
            typename std::map<K, std::stack<stack_element_t>>::iterator it;

            list_element_t(const K &key, std::map<K, std::stack<stack_element_t>> &map) {
                it = map.find(key);
            }
        };

        struct stack_element_t {
            V value;
            typename std::list<list_element_t>::iterator it;

            stack_element_t(const V &val, std::list<list_element_t> &list) {
                value = val;
                it = list.begin();
            }
        };

        std::list<list_element_t> actual_stack;
        std::map<K, std::stack<stack_element_t>> access_map;
    public:
        stack() noexcept {
            actual_stack = std::list<list_element_t>();
            access_map = std::map<K, std::stack<stack_element_t>>();
        }

        //stack(stack const &) noexcept;

        //stack(stack &&) noexcept;

        //stack &operator=(stack);

        void push(K const &key, V const &value) {
            if (access_map.count(key) == 0) {
                access_map.insert({key, std::stack<stack_element_t>()});
            }

            actual_stack.push_front(list_element_t(key, access_map));
            access_map.at(key).push(stack_element_t(value, actual_stack));
        }

        void pop() {
            auto elem = actual_stack.front();
            actual_stack.pop_front();
            elem.it->second.pop();
        }

        void pop(K const &key) {
            stack_element_t pair = access_map.at(key).top();
            actual_stack.erase(pair.it);
            access_map.at(key).pop();
        }

        std::pair<K const &, V &> front() {
            list_element_t elem = actual_stack.front();
            return std::pair<K const &, V &>({elem.it->first, elem.it->second.top().value});
        }

        std::pair<K const &, V const &> front() const {
            list_element_t elem = actual_stack.front();
            return std::pair<K const &, V const &>({elem.it->first, elem.it->second.top().value});
        }

        V &front(K const &key) {
            return access_map.at(key).top().value;
        }

        V const &front(K const &key) const {
            return access_map.at(key).top().value;
        }

        std::size_t size() const {
            return actual_stack.size();
        }

        std::size_t count(K const &key) const {
            return access_map.at(key).size();
        }

        void clear() {
            actual_stack.clear();
            access_map.clear();
        }



        class const_iterator {
            using pointer = const K*;
            using reference = const K&;
            typename std::map<K, std::stack<stack_element_t>>::const_iterator iter;



        public:
            const_iterator(typename std::map<K, std::stack<stack_element_t>>::const_iterator i) : iter(i) {}
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