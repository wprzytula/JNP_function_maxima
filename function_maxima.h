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

namespace detail {
    template<typename elem_type, typename set_type>
    class InsertionGuard {
    private:
        using iterator = typename set_type::const_iterator;
        set_type &set;
        elem_type const *const element;
        bool revert;
        bool const perform;
        iterator it;
    public:
        InsertionGuard(set_type &set, elem_type const *const element, bool const perform)
                : set(set), element(element), revert(true), perform(perform) {
            if (perform)
                it = set.insert(*element).first;
        }

        ~InsertionGuard() noexcept {
            if (perform && revert)
                set.erase(it);
        }

        void done() noexcept {
            revert = false;
        }
    };
}

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
public:
    FunctionMaxima() = default;
    FunctionMaxima(FunctionMaxima const&) = default;
    FunctionMaxima& operator=(FunctionMaxima const&);
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
    // iterujący po punktach funkcji.
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

    /* Poniższe funkcje przyjmują parametr to_erase wskazujący na punkt, którego przyszłe usunięcie
     ma zostać uwzględnione w sprawdzaniu zachodzenia kryteriów. Jeśli żaden punkt nie ma zostać
     usunięty, należy przekazać jako argument end().*/
    // Sprawdza lewe kryterium istnienia maksimum funkcji w punkcie wskazywanym przez it.
    bool left_max_check(iterator it, iterator to_erase) const;
    // Sprawdza prawe kryterium istnienia maksimum funkcji w punkcie wskazywanym przez it.
    bool right_max_check(iterator it, iterator to_erase) const;
    // Sprawdza, czy w punkcie wskazywanym przez it istnieje maksimum funkcji.
    bool is_maximum(iterator it, iterator to_erase) const {
        return left_max_check(it, to_erase) && right_max_check(it, to_erase);
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

template<typename A, typename V>
FunctionMaxima<A, V>& FunctionMaxima<A, V>::operator=(FunctionMaxima const& another) {
    FunctionMaxima temporary{another};
    std::swap(fun, temporary.fun);
    std::swap(maxima, temporary.maxima);
    std::swap(range, temporary.range);
    return *this;
}

template <typename A, typename V>
void FunctionMaxima<A, V>::set_value(A const& a, V const& v) {
    iterator arg_pos = find(a);
    bool const arg_was_present = arg_pos != end();
    mx_iterator max_pos = arg_was_present ? mx_find(*arg_pos) : mx_end();

    if (arg_was_present && !(arg_pos->value() < v)
        && !(v < arg_pos->value()))
        return; // Danemu argumentowi jest już przypisana dana wartość.

    rg_iterator new_val_pos = rg_find(v);
    bool const val_was_present = new_val_pos != rg_end();
    
    rg_iterator old_val_pos = arg_was_present ? rg_find(arg_pos->value()) : rg_end();
    auto old_val_handle = arg_was_present ? old_val_pos->lock() : std::shared_ptr<V>{};

    std::shared_ptr<A> a_ptr = arg_was_present
            ? arg_pos->arg_ptr
            : std::make_shared<A>(static_cast<A>(a));
    std::shared_ptr<V> val_ptr = val_was_present
            ? new_val_pos->lock()
            : std::make_shared<V>(static_cast<V>(v));
    // Koniec fragmentu, który nie powoduje żadnych modyfikacji stanu.

    detail::InsertionGuard value_guard{range, &val_ptr, !val_was_present};
    // f(a) = v
    if (arg_was_present)
        arg_pos->replace_value(val_ptr);
    else
        arg_pos = fun.insert(point_type{a_ptr, val_ptr}).first;

    // Inicjalizacje będące no-throw.
    bool const left_arg_exist = arg_pos != begin();
    bool const right_arg_exist = std::next(arg_pos) != end();
    iterator left_arg = left_arg_exist ? std::prev(arg_pos) : end();
    iterator right_arg = right_arg_exist ? std::next(arg_pos) : end();
    mx_iterator right_mx_pos = mx_end();
    mx_iterator left_mx_pos = mx_end();
    bool was_mx = max_pos != mx_end(), was_left_mx = false, was_right_mx = false,
         should_erase_left_mx, should_erase_right_mx;

    try {
        bool const will_be_mx = is_maximum(arg_pos, end());
        bool const will_be_left_mx = left_arg_exist ? is_maximum(left_arg, end()) : false;
        bool const will_be_right_mx = right_arg_exist ? is_maximum(right_arg, end()) : false;
        if (left_arg_exist) {
            left_mx_pos = mx_find(*left_arg);
            was_left_mx = left_mx_pos != mx_end();
        }
        if (right_arg_exist) {
            right_mx_pos = mx_find(*right_arg);
            was_right_mx = right_mx_pos != mx_end();
        }
        should_erase_left_mx = left_arg_exist && !will_be_left_mx && was_left_mx;
        should_erase_right_mx = right_arg_exist && !will_be_right_mx && was_right_mx;
        bool const should_insert_left_mx = left_arg_exist && will_be_left_mx && !was_left_mx;
        bool const should_insert_right_mx = right_arg_exist && will_be_right_mx && !was_right_mx;

        // Insercje maksimów.
        detail::InsertionGuard max_guard{maxima, &*arg_pos, will_be_mx};
        detail::InsertionGuard max_l_guard{maxima, left_arg_exist ? &*left_arg : nullptr,
                                           should_insert_left_mx};
        detail::InsertionGuard max_r_guard{maxima, right_arg_exist ? &*right_arg : nullptr,
                                           should_insert_right_mx};
        max_guard.done();
        max_l_guard.done();
        max_r_guard.done();
        value_guard.done();

    } catch (...) {
        // Przywracanie poprzedniej wartości.
        if (arg_was_present)
            arg_pos->replace_value(old_val_handle);
        else
            fun.erase(arg_pos);
        throw;
    }
    // Usunięcia nieaktualnych maksimów.
    if (was_mx)
        maxima.erase(max_pos);
    if (should_erase_left_mx)
        maxima.erase(left_mx_pos);
    if (should_erase_right_mx)
        maxima.erase(right_mx_pos);
    // Usunięcie starej wartości ze zbioru wartości.
    old_val_handle.reset();
    if (arg_was_present && old_val_pos->expired())
        range.erase(old_val_pos);
}

template <typename A, typename V>
void FunctionMaxima<A, V>::erase(A const& a) {
    iterator arg_pos = find(a);
    if (arg_pos != end()) {
        rg_iterator val_pos = rg_find(arg_pos->value());
        if (val_pos == rg_end())
            assert(false);
        mx_iterator left_mx = mx_end(), right_mx = mx_end();
        mx_iterator mx_it = maxima.find(*arg_pos);
        bool const left_arg_exists = arg_pos != begin(),
                   right_arg_exists = std::next(arg_pos) != end();
        bool was_left_mx = false, was_right_mx = false, will_be_left_mx = false,
             will_be_right_mx = false, should_insert_left_mx, should_insert_right_mx,
             should_erase_left_mx, should_erase_right_mx;
        iterator left_arg, right_arg;
        if (left_arg_exists) {
            left_arg = std::prev(arg_pos);
            left_mx = maxima.find(*left_arg);
            was_left_mx = left_mx != mx_end();
            will_be_left_mx = is_maximum(left_arg, arg_pos);
        }
        if (right_arg_exists) {
            right_arg = std::next(arg_pos);
            right_mx = maxima.find(*right_arg);
            was_right_mx = right_mx != mx_end();
            will_be_right_mx = is_maximum(right_arg, arg_pos);
        }
        should_erase_left_mx = left_arg_exists && was_left_mx && !will_be_left_mx;
        should_erase_right_mx = right_arg_exists && was_right_mx && !will_be_right_mx;
        should_insert_left_mx = left_arg_exists && !was_left_mx && will_be_left_mx;
        should_insert_right_mx = right_arg_exists && !was_right_mx && will_be_right_mx;

        detail::InsertionGuard left_guard{maxima, left_arg_exists ? &*left_arg : nullptr,
                                          should_insert_left_mx};
        detail::InsertionGuard right_guard{maxima, right_arg_exists ? &*right_arg : nullptr,
                                           should_insert_right_mx};

        left_guard.done();
        right_guard.done();

        if (mx_it != mx_end())
            maxima.erase(mx_it);
        fun.erase(arg_pos);
        if (should_erase_left_mx)
            maxima.erase(left_mx);
        if (should_erase_right_mx)
            maxima.erase(right_mx);
        if (val_pos->expired())
            range.erase(val_pos);
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