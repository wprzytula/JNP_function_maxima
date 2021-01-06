#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

#include <utility>
#include <set>
#include <memory>
#include <cassert>

class InvalidArg : std::exception {
public:
    [[nodiscard]] char const* what() const noexcept override {
        return "invalid argument value";
    }
};

template<typename A, typename V>
class FunctionMaxima {
public:
    class point_type {
    private:
        std::shared_ptr<A> arg_ptr;
        // To jest mutable celem umożliwienia modyfikacji poprzez replace_value
        // będąc wewnątrz zbioru (czyli będąc const).
        mutable std::shared_ptr<V> value_ptr;
        // Przekazanie shared_ptr do konstruktora poprzez wartość,
        // zgodnie z https://stackoverflow.com/a/17369971.
        explicit point_type(std::shared_ptr<A> arg, std::shared_ptr<V> value)
            : arg_ptr(arg), value_ptr(value) {}
        // Modyfikuje wartość wewnątrz point_type oznaczonego jako const;
        // wykorzystywane do zmiany wewnątrz zbioru.
        void replace_value(std::shared_ptr<V> const& new_value) const {
            value_ptr = new_value;
        }
        friend class FunctionMaxima;
    public:
        point_type(point_type const&) = default;
        point_type& operator=(point_type const&) = default;
        // Zwraca argument funkcji.
        A const& arg() const noexcept {
            return *arg_ptr;
        }
        // Zwraca wartość funkcji w tym punkcie.
        V const& value() const noexcept {
            return *value_ptr;
        }
        // Zadaje porządek liniowy na point_type identyczny z porządkiem na argumentach.
        bool operator<(point_type const& other) const {
            return *arg_ptr < *other.arg_ptr;
        }
    };
private:
    // Funktor umożliwiający porównania obiektów wewnątrz zbioru reprezentującego
    // dziedzinę funkcji wraz z przypisanymi argumentom wartościami.
    struct argument_order;
    using function_set = std::set<point_type, argument_order>;

    // Funktor umożliwiający porównania obiektów wewnątrz zbioru maksimów.
    struct maxima_order;
    using maxima_set = std::set<point_type, maxima_order>;

    // Funktor umożliwiający porównania obiektów wewnątrz zbioru wartości.
    struct range_order;
    using range_set = std::set<std::weak_ptr<V>, range_order>;
    using rg_iterator = typename range_set::const_iterator;

public:
    // Typ iterator zachowujący się jak bidirectional_iterator,
    // iterujący po po punktach funkcji.
    using iterator = typename function_set::const_iterator;

    // Iterator wskazujący na pierwszy punkt.
    iterator begin() const noexcept {
        return fun.cbegin();
    }

    // Iterator wskazujący za ostatni punkt.
    iterator end() const noexcept {
        return fun.cend();
    }

    // Iterator, który wskazuje na punkt funkcji o argumencie a lub end(),
    // jeśli takiego argumentu nie ma w dziedzinie funkcji.
    iterator find(A const& a) const {
        return fun.find(a);
    }

    // Typ mx_iterator zachowujący się znów jak bidirectional_iterator,
    // iterujący po lokalnych maksimach funkcji.
    using mx_iterator = typename maxima_set::const_iterator;

    // Iterator wskazujący na pierwsze lokalne maksimum.
    mx_iterator mx_begin() const noexcept {
        return maxima.cbegin();
    }

    // Iterator wskazujący za ostatnie lokalne maksimum.
    mx_iterator mx_end() const noexcept {
        return maxima.cend();
    }
private:
    // Iterator, który wskazuje na maksimum funkcji w zadanym punkcie lub end(),
    // jeśli dany punkt nie jest lokalnym maksimum funkcji.
    mx_iterator mx_find(point_type const& pt) const {
        return maxima.find(pt);
    }

    // Iterator wskazujący za największą wartość funkcji.
    rg_iterator rg_end() const noexcept {
        return range.cend();
    }

    // Iterator, który wskazuje na zadaną wartość funkcji lub rg_end(),
    // jeśli funkcja nie przyjmuje takiej wartości dla żadnego argumentu.
    rg_iterator rg_find(V const& v) const {
        return range.find(v);
    }

public:
    // Typ size_type reprezentujący rozmiar dziedziny i funkcja zwracająca ten rozmiar:
    using size_type = typename function_set::size_type;

    size_type size() const noexcept {
        return fun.size();
    }

    FunctionMaxima() = default;
    FunctionMaxima(FunctionMaxima const&) = default;
    FunctionMaxima& operator=(FunctionMaxima const&) = default;

    // Zwraca wartość w punkcie a, rzuca wyjątek InvalidArg, jeśli a nie
    // należy do dziedziny funkcji.
    V const& value_at(A const& a) const {
        iterator it = find(a);
        if (it == fun.end())
            throw InvalidArg();
        else
            return it->value();
    }

    // Zmienia funkcję tak, żeby zachodziło f(a) = v. Jeśli a nie należy do
    // obecnej dziedziny funkcji, jest do niej dodawany.
    void set_value(A const& a, V const& v);

    // Usuwa a z dziedziny funkcji. Jeśli a nie należało do dziedziny funkcji,
    // nie dzieje się nic.
    void erase(A const&);

private:
    function_set fun;
    maxima_set maxima;
    range_set range;

    bool left_max_check(iterator it, iterator to_erase) const;
    bool right_max_check(iterator it, iterator to_erase) const;
    bool is_maximum(iterator it, iterator to_erase) const {
        return left_max_check(it, to_erase) && right_max_check(it, to_erase);
    }

    //TODO: delete
public:
    void print() const noexcept {
        std::cout << "\nfun: ";
        for (iterator it = begin(); it != end(); ++it) {
            std::cout << it->arg() << " -> " << it->value() << ", ";
        }
        std::cout << '\n';
        std::cout << "maxima: ";
        for (mx_iterator it = mx_begin(); it != mx_end(); ++it) {
            std::cout << it->arg() << " -> " << it->value() << ", ";
        }
        std::cout << '\n';
        std::cout << "range: ";
        for (rg_iterator it = range.cbegin(); it != rg_end(); ++it) {
            std::cout << *it->lock() << ", ";
        }
        std::cout << '\n';
    }
};

template <typename A, typename V>
struct FunctionMaxima<A, V>::argument_order {
    // https://www.fluentcpp.com/2017/06/09/search-set-another-type-key
    using is_transparent = void;
    bool operator()(point_type const& x, point_type const& y) const {
        return x < y;
    }
    bool operator()(point_type const& x, A const& y) const {
        return *x.arg_ptr < y;
    }
    bool operator()(A const& x, point_type const& y) const {
        return x < *y.arg_ptr;
    }
};

template <typename A, typename V>
struct FunctionMaxima<A, V>::maxima_order {
    bool operator()(point_type const& x, point_type const& y) const {
        return y.value() < x.value() ||
        (!(x.value() < y.value()) && !(y.value() < x.value()) && x.arg() < y.arg());
    }
};

template <typename A, typename V>
struct FunctionMaxima<A, V>::range_order {
    using is_transparent = void;
    bool operator()(std::weak_ptr<V> const x, std::weak_ptr<V> const y) const {
        assert(!x.expired() && !y.expired());
        return *x.lock() < *y.lock();
    }
    bool operator()(V const& x, std::weak_ptr<V> const y) const {
        assert(!y.expired());
        return x < *y.lock();
    }
    bool operator()(std::weak_ptr<V> const x, V const& y) const {
        assert (!x.expired());
        return *x.lock() < y;
    }
};

template <typename elem_type, typename set_type>
class InsertionGuard {
private:
    using iterator = typename set_type::const_iterator;
    set_type& set;
    elem_type& element;
    bool revert, perform;
    iterator it;
public:
    InsertionGuard(set_type& set, elem_type& element, bool perform)
        : set(set), element(element), revert(true), perform(perform) {
        if (perform)
            it = set.insert(element).first;
    }
    ~InsertionGuard() noexcept {
        if (perform && revert)
            set.erase(it);
    }
    void done() noexcept {
        revert = false;
    }
};

template <typename A, typename V>
void FunctionMaxima<A, V>::set_value(A const& a, V const& v) {
    iterator it = find(a);
    bool arg_was_present = it != end();
    mx_iterator max_pos = arg_was_present ? mx_find(*it) : mx_end();

    if (arg_was_present && !(it->value() < v)
        && !(v < it->value()))
        return; // Danemu argumentowi jest już przypisana dana wartość.

    rg_iterator new_val_it = rg_find(v);
    bool val_was_present = new_val_it != rg_end();
    
    rg_iterator old_val_it = arg_was_present ? rg_find(it->value()) : rg_end();

    std::shared_ptr<A> a_ptr = arg_was_present
            ? it->arg_ptr
            : std::make_shared<A>(static_cast<A>(a));
    std::shared_ptr<V> v_ptr = val_was_present
            ? new_val_it->lock()
            : std::make_shared<V>(static_cast<V>(v));

    InsertionGuard value_guard {range, v_ptr, !val_was_present};
    
    // f(a) = v
    if (arg_was_present)
        it->replace_value(v_ptr);
    else
        it = fun.insert(point_type{a_ptr, v_ptr}).first;

    // Ustalenia będące no-throw.
    bool left_arg_exist = it != begin();
    bool right_arg_exist = std::next(it) != end();
    iterator left = left_arg_exist ? std::prev(it) : end();
    iterator right = right_arg_exist ? std::next(it) : end();

    bool will_be_max = is_maximum(it, end());
    bool will_be_max_l = left_arg_exist ? is_maximum(left, end()) : false;
    bool will_be_max_r = right_arg_exist ? is_maximum(right, end()) : false;
    
    mx_iterator max_pos_r = mx_end();
    mx_iterator max_pos_l = mx_end();
    bool was_max = max_pos != mx_end();
    bool was_max_l = false, was_max_r = false, should_be_erased_l, should_be_erased_r;

    try {
        if (left_arg_exist) {
            max_pos_l = mx_find(*left);
            was_max_l = max_pos_l != mx_end();
        }
        if (right_arg_exist) {
            max_pos_r = mx_find(*right);
            was_max_r = max_pos_r != mx_end();
        }
        should_be_erased_l = left_arg_exist && !will_be_max_l && was_max_l;
        should_be_erased_r = right_arg_exist && !will_be_max_r && was_max_r;
        bool should_be_inserted_l = left_arg_exist && will_be_max_l && !was_max_l;
        bool should_be_inserted_r = right_arg_exist && will_be_max_r && !was_max_r;

        // Insercje maximów.
        InsertionGuard max_guard {maxima, *it, will_be_max};
        InsertionGuard max_l_guard {maxima, *left, should_be_inserted_l};
        InsertionGuard max_r_guard {maxima, *right, should_be_inserted_r};

        max_guard.done();
        max_l_guard.done();
        max_r_guard.done();
        value_guard.done();
    } catch (...) {
        // Przywracanie poprzedniej wartości.
        if (arg_was_present)
            it->replace_value(old_val_it->lock());
        else
            fun.erase(it);
        throw;
    }

    // Usunięcia nieaktualnych maksimów.
    if (was_max)
        maxima.erase(max_pos);
    if (should_be_erased_l)
        maxima.erase(max_pos_l);
    if (should_be_erased_r)
        maxima.erase(max_pos_r);

    // Usunięcie starej wartości ze zbioru wartości.
    if (arg_was_present && old_val_it->expired())
        range.erase(old_val_it);
}

template <typename A, typename V>
void FunctionMaxima<A, V>::erase(A const& a) {
    iterator to_erase = find(a);
    if (to_erase != end()) {
        bool should_erase_left_mx = false, should_erase_right_mx = false;
        rg_iterator rg_it = rg_find(to_erase->value());
        mx_iterator left_mx = mx_end(), right_mx = mx_end();
        mx_iterator mx_it = maxima.find(*to_erase);
        try {
            if (rg_it == rg_end()) {
                print();
                assert(false);
            }
            if (to_erase != begin()) {
                iterator left_arg = std::prev(to_erase);
                if (is_maximum(left_arg, to_erase))
                    left_mx = maxima.insert(*left_arg).first;
                else {
                    left_mx = maxima.find(*left_arg);
                    if (left_mx != mx_end())
                        should_erase_left_mx = true;
                }
            }
            iterator right_arg = std::next(to_erase);
            if (right_arg != end()) {
                if (is_maximum(right_arg, to_erase))
                    right_mx = maxima.insert(*right_arg).first;
                else {
                    right_mx = maxima.find(*right_arg);
                    if (right_mx != mx_end())
                        should_erase_right_mx = true;
                }
            }
        } catch (...) {
            if (left_mx != mx_end()) {
                maxima.erase(left_mx);
                if (right_mx != mx_end())
                    maxima.erase(right_mx);
            }
            throw;
        }
        if (mx_it != mx_end())
            maxima.erase(mx_it);
        fun.erase(to_erase);
        if (should_erase_left_mx)
            maxima.erase(left_mx);
        if (should_erase_right_mx)
            maxima.erase(right_mx);
        if (rg_it->expired())
            range.erase(rg_it);
    }
}

template<typename A, typename V>
bool FunctionMaxima<A, V>::left_max_check(iterator it, iterator to_erase) const {
    if (it == begin())
        return true;
    iterator left = std::prev(it);
    if (left == to_erase) {
        if (left == begin())
            return true;
        else
            --left;
    }
    return !(it->value() < left->value());
}

template<typename A, typename V>
bool FunctionMaxima<A, V>::right_max_check(iterator it, iterator to_erase) const {
    iterator right = std::next(it);
    if (right == end())
        return true;
    if (right == to_erase) {
        if (++right == end())
            return true;
    }
    return !(it->value() < right->value());
}

#endif // FUNCTION_MAXIMA_H