#include <utility>
#include <stack>
#include <map>
#include <list>
#include <cstring>
#include <memory>
#include <stdexcept>

namespace cxx {
    template <typename K, typename V>
    class stack {
    private:
        bool referenced;
        struct stack_element_t;
        struct list_element_t;
        using stack_iter = typename std::list<stack_element_t>::reverse_iterator;
        using r_list_iter = typename std::list<list_element_t>::reverse_iterator;
        struct list_element_t {
            typename std::map<K,std::list<stack_element_t>>::iterator it;

            list_element_t(const K &key, std::map<K, std::list<stack_element_t>> &map) : it(map.find(key)) {}
        };

        struct stack_element_t {
            V value;
            typename std::list<list_element_t>::iterator it;

            stack_element_t(const V &val, std::list<list_element_t> &list) : value(val), it(list.begin()) {}
        };
        
        //stack is represented by a list of map iterators to maintain the order of elements with different keys
        //and a map of lists (stacks) to maintain the order of elements with the same key
        //values are only stored in the access_map and can be accessed from the list using map iterator
        
        std::shared_ptr<std::list<list_element_t>> actual_stack;
        std::shared_ptr<std::map<K, std::list<stack_element_t>>> access_map;
        
        
        //copies the stack by pushing all the elements onto a new stack
        static void copy(stack &this_stack, stack const &s){
            stack new_stack;
            std::map<K, stack_iter> iters;
        	for(r_list_iter rit = s.actual_stack->rbegin(); rit != s.actual_stack->rend(); ++rit){
                const K& key = rit->it->first;
                auto it = iters.find(key);
                if (it == iters.end()) {
                    it = iters.insert({key, rit->it->second.rbegin()}).first;
                }
                new_stack.push(key, it->second->value);
        		it->second++;
        	}
            this_stack.actual_stack = new_stack.actual_stack;
            this_stack.access_map = new_stack.access_map;
        }

        
    public:
        stack() {
            referenced = false;
            actual_stack = std::make_shared<std::list<list_element_t>>();
            access_map = std::make_shared<std::map<K, std::list<stack_element_t>>>();
        } 
        
		//only copies the stack if the user may use the reference to the top element of the stack
        stack(stack const &s) {
            referenced = false;
            if (s.referenced) {
                copy(*this,s);
            } else {
                actual_stack = s.actual_stack;
                access_map = s.access_map;
            }

        }
        
        stack(stack && s) noexcept {
            referenced = std::move(s.referenced);
            actual_stack = std::move(s.actual_stack);
            access_map = std::move(s.access_map);
        }	
        
        
        stack & operator= (stack s) noexcept {
            actual_stack = s.actual_stack;
            access_map = s.access_map;
            referenced = s.referenced;
        	return *this;
        }
        
        
        void push(K const &key, V const &value) {
       		//copy-on-write semantics
            if (actual_stack.use_count() > 1){
                stack s;
                copy(s, *this);
                s.push(key, value);
                access_map = s.access_map;
                actual_stack = s.actual_stack;
            } else {
            	//exception safe push
            	//if push throws an exception, changes made to the stack are reverted
                bool inserted = false;
                auto map_it = access_map->find(key);
                if (map_it == access_map->end()) {
                    map_it = access_map->insert({key, std::list<stack_element_t>()}).first;
                    inserted = true;
                }
                try {
                    actual_stack->push_front(list_element_t(key, *access_map));
                    auto it = actual_stack->begin();
                    try {
                        map_it->second.push_front(stack_element_t(value, *actual_stack));
                    } catch (...) {
                        actual_stack->erase(it);
                        throw;
                    }
                }
                catch (...) {
                    if (inserted) {
                        access_map->erase(map_it);
                    }
                    throw;
                }
            }
            referenced = false;

        }

		
        void pop() {
            if (size() == 0) {
                throw std::invalid_argument("");
            }
            //copy-on-write semantics
            if (actual_stack.use_count() > 1){
                stack s;
                copy(s, *this);
                s.pop();
                access_map = s.access_map;
                actual_stack = s.actual_stack;
            } else {
                auto stack_it = actual_stack->front().it;
                actual_stack->pop_front();
                stack_it->second.pop_front();
                if (stack_it->second.empty()) {
                    access_map->erase(stack_it);
                }
            }
            referenced = false;
        }

        void pop(K const &key) {
            if (count(key) == 0) {
                throw std::invalid_argument("");
            }
            //copy-on-write semantics
            if (actual_stack.use_count() > 1){
                stack s;
                copy(s, *this);
                s.pop(key);
                access_map = s.access_map;
                actual_stack = s.actual_stack;
            }
            else {
                // potential exception
                auto stack_it = access_map->find(key);
                stack_element_t pair = stack_it->second.front();
                stack_it->second.pop_front();
                if (stack_it->second.empty()) {
                    access_map->erase(stack_it);
                }
                actual_stack->erase(pair.it);
            }
            referenced = false;
        }

        std::pair<K const &, V &> front() {
            if (size() == 0) {
                throw std::invalid_argument("");
            }
            //copy-on-write semantics
            if (actual_stack.use_count() > 1){
                stack s;
                copy(s, *this);
                std::pair<K const &, V &> ret_val = s.front();
                access_map = s.access_map;
                actual_stack = s.actual_stack;
                referenced = true;
                return ret_val;
            } else {
                list_element_t elem = actual_stack->front();
                std::pair<K const &, V &> ret_val =
                        std::pair<K const &, V &>({elem.it->first, elem.it->second.front().value});
                referenced = true;
                return ret_val;
            }
        }

        std::pair<K const &, V const &> front() const {
            if (size() == 0) {
                throw std::invalid_argument("");
            }
            list_element_t elem = actual_stack->front();
            return std::pair<K const &, V const &>({elem.it->first, elem.it->second.front().value});
        }

        V &front(K const &key) {
            if (count(key) == 0) {
                throw std::invalid_argument("");
            }
            //copy-on-write semantics
            if (actual_stack.use_count() > 1){
                stack s;
                copy(s, *this);
                V& ret_val = s.front(key);
                access_map = s.access_map;
                actual_stack = s.actual_stack;
                referenced = true;
                return ret_val;
            } else {
                V& ret_val = access_map->at(key).front().value;
                referenced = true;
                return ret_val;
            }
        }

        V const &front(K const &key) const {
            if (count(key) == 0) {
                throw std::invalid_argument("");
            }
            return access_map->at(key).front().value;
        }

        std::size_t size() const noexcept {
            return actual_stack->size();
        }

        std::size_t count(K const &key) const {
        	if (access_map->count(key) == 0)	return 0;
            return access_map->at(key).size();
        }

        void clear() {
        	//copy-on-write semantics
            if(actual_stack.use_count() > 1){
                stack s;
                copy(s, *this);
                actual_stack = s.actual_stack;
                access_map = s.access_map;
            }
            actual_stack->clear();
            access_map->clear();
        }


		//iterator satisfying the std::forward_iterator concept
		//used to traverse keys of elements on the stack
        class const_iterator {
            using c_iter =  typename std::map<K, std::list<stack_element_t>>::const_iterator;
            c_iter iter;

        public:
            using difference_type = std::ptrdiff_t;
            using value_type = const K;
            using pointer = const K*;
            using reference = const K&;
            const_iterator() : iter(c_iter()) {}
            const_iterator(c_iter i) : iter(i) {}
            const_iterator &operator++() noexcept {
                iter++;
                return *this;
            }

            const_iterator operator++(int) noexcept {
                const_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            reference operator*() const noexcept {
                return iter->first;
            }

            pointer operator->() const noexcept {
                return &(iter->first);
            }
            friend bool operator== (const const_iterator& a, const const_iterator& b) { return a.iter == b.iter; };
            friend bool operator!= (const const_iterator& a, const const_iterator& b) { return a.iter != b.iter; };
        };

        stack<K, V>::const_iterator cbegin() const noexcept {
            return const_iterator((*access_map).cbegin());
        }
        stack<K, V>::const_iterator cend() const noexcept {
            return const_iterator((*access_map).cend());
        }

    };
}
