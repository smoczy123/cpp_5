#include <utility>
#include <stack>
#include <map>
#include <list>
#include <cstring>
#include <deque>
#include <memory>

namespace cxx {
    template <typename K, typename V>
    class stack {
    private:
        bool referenced;
        struct stack_element_t;
        struct list_element_t;

        struct list_element_t {
            typename std::map<K,std::deque<stack_element_t>>::iterator it;

            list_element_t(const K &key, std::map<K, std::deque<stack_element_t>> &map) {
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

        std::shared_ptr<std::list<list_element_t>> actual_stack;
        std::shared_ptr<std::map<K, std::deque<stack_element_t>>> access_map;
        
        void copy(stack const &s){
        	actual_stack = std::make_shared<std::list<list_element_t>>();
            access_map = std::make_shared<std::map<K, std::deque<stack_element_t>>>();
            std::map<K, int> iters;
            for(auto el : (*s.access_map)){
            	iters.insert(std::make_pair(el.first,0));
            }
        	for(typename std::list<list_element_t>::reverse_iterator rit = (*s.actual_stack).rbegin(); rit != (*s.actual_stack).rend(); ++rit){
        		this->push(rit->it->first,rit->it->second[rit->it->second.size()-iters[rit->it->first]-1].value);
        		iters.at(rit->it->first)++;
        	}
        }
        
        void check_if_shared(){
        	if(actual_stack.use_count() > 1){
        		stack s=*this;
        		*this=s;
        	}
        }
        
    public:
        stack() noexcept {
            referenced = false;
            actual_stack = std::make_shared<std::list<list_element_t>>();
            access_map = std::make_shared<std::map<K, std::deque<stack_element_t>>>();
        } 

        stack(stack const &s) noexcept{
            referenced = false;
        	copy(s);
        }
        
        stack(stack && s) noexcept{
            referenced = false;
        	if(this != &s) {
        		actual_stack = std::move(s.actual_stack);
        		access_map = std::move(s.access_map);
        	}
        }	
        
        
        stack & operator = (stack const & s) noexcept{
            referenced = false;
        	if(&s!=this){
                  actual_stack = s.actual_stack;
          		  access_map = s.access_map;
        	}
        	return * this;
        }
        
        void push(K const &key, V const &value) {
            referenced = false;
        	check_if_shared();
            if ((*access_map).count(key) == 0) {
                (*access_map).insert({key, std::deque<stack_element_t>()});
            }

            (*actual_stack).push_front(list_element_t(key, *access_map));
            (*access_map).at(key).push_front(stack_element_t(value, *actual_stack));
        }

        void pop() {
            referenced = false;
        	check_if_shared();
            auto elem = (*actual_stack).front();
            (*actual_stack).pop_front();
            elem.it->second.pop_front();
        }

        void pop(K const &key) {
            referenced = false;
        	check_if_shared();
            stack_element_t pair = (*access_map).at(key).front();
            (*actual_stack).erase(pair.it);
            (*access_map).at(key).pop_front();
        }

        std::pair<K const &, V &> front() {
            referenced = true;
            list_element_t elem = (*actual_stack).front();
            return std::pair<K const &, V &>({elem.it->first, elem.it->second.front().value});
        }

        std::pair<K const &, V const &> front() const {
            list_element_t elem = (*actual_stack).front();
            return std::pair<K const &, V const &>({elem.it->first, elem.it->second.front().value});
        }

        V &front(K const &key) {
            referenced = true;
            return (*access_map).at(key).front().value;
        }

        V const &front(K const &key) const {
            return (*access_map).at(key).front().value;
        }

        std::size_t size() const {
            return (*actual_stack).size();
        }

        std::size_t count(K const &key) const {
        	if ((*access_map).count(key)==0)	return 0;
            return (*access_map).at(key).size();
        }

        void clear() {
        	check_if_shared();
            (*actual_stack).clear();
            (*access_map).clear();
        }



        class const_iterator {
            using pointer = const K*;
            using reference = const K&;
            typename std::map<K, std::deque<stack_element_t>>::const_iterator iter;



        public:
            const_iterator(typename std::map<K, std::deque<stack_element_t>>::const_iterator i) : iter(i) {}
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
            return const_iterator((*access_map).cbegin());
        }
        stack<K, V>::const_iterator cend() {
            return const_iterator((*access_map).cend());
        }

    };
}
