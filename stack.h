
#include <utility>

namespace cxx {
    template <typename K, typename V>
    class stack {
        stack();

        stack(stack const &);

        stack(stack &&);

        stack & operator=(stack);

        void push(K const &, V const &);

        void pop();

        void pop(K const &);

        std::pair<K const &, V &> front();
        std::pair<K const &, V const &> front() const;

        V & front(K const &);
        V const & front(K const &) const;

        std::size_t size() const;

        std::size_t count(K const &) const;

        void clear();

    };
}