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
        // TODO: do usunięcia?
//        explicit point_type(A arg, V value)
//            : arg_ptr(std::make_shared(arg)), value_ptr(std::make_shared(value)) {}
        // Przekazanie shared_ptr do konstruktora poprzez wartość,
        // zgodnie z https://stackoverflow.com/a/17369971.
        explicit point_type(std::shared_ptr<A> const arg, std::shared_ptr<V> const value)
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

private:
    // Dodaje do funkcji punkt (a, v). Zakłada, że argument a
    // nie należał przedtem do dziedziny funkcji.
    void add_value(A const& a, V const& v);

    // Zmienia poprzednią wartość funkcji dla argumentu a na v.
    // Zakłada, że argument a należał już przedtem do dziedziny funkcji.
    void modify_value(A const& a, V const& v);

public:
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

    // nie moje wersje ~Wojciech
    //Pomocnicze funkcje, określające czy
    //w danym miejscu jest maksimum lokalne
    bool left_checker(iterator it){
        if (it == fun.cbegin()) return true;
        iterator left = std::prev(it);
        return !(it->value() < left->value()); //możliwy wyjątek w >
    }
    bool right_checker(iterator it){
        iterator right = std::next(it);
        if (right == fun.cend()) return true;
        return !(it->value() < right->value()); //możliwy wyjątek w >
    }

    bool maximum_check(iterator it){
        return left_checker(it) && right_checker(it); //dziedziczy wyjatki
    }


    // Moje wersje ~Wojciech
    bool left_check(iterator it, iterator to_erase) const {
        if (it == fun.begin())
            return true;
        iterator left = std::prev(it);
        if (left == to_erase) {
            if (left == fun.begin())
                return true;
            else
                --left;
        }
        return !(it->value() < left->value()); //możliwy wyjątek w <
    }
    bool right_check(iterator it, iterator to_erase) const {
        iterator right = std::next(it);
        if (right == fun.end())
            return true;
        if (right == to_erase) {
            if (++right == fun.end())
                return true;
        }
        return !(it->value() < right->value()); //możliwy wyjątek w <
    }
    bool is_maximum(iterator it, iterator to_erase = nullptr) const {
        return left_check(it, to_erase) && right_check(it, to_erase);
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
void FunctionMaxima<A, V>::add_value(A const& a, V const& v) {

}

template<typename A, typename V>
void FunctionMaxima<A, V>::modify_value(A const& a, V const& v) {

}

template <typename A, typename V>
void FunctionMaxima<A, V>::set_value(A const& a, V const& v) {
    iterator it = find(a);
    bool found = it != end();
    mx_iterator max_position = mx_end();
    if(found)
        max_position = mx_find(*it);
    //v = stara wartosc
    if (found && !(it->value() < v) 
        && !(v < it->value())) return ;

    rg_iterator new_val_it = rg_find(v);
    bool new_value = new_val_it != rg_end();
    
    rg_iterator v_ptr_old = rg_end();
    if(found)
        v_ptr_old = rg_find(it->value());

    std::shared_ptr<A> a_ptr = found
            ? it->arg_ptr
            : std::make_shared<A>(static_cast<A>(a));
    std::shared_ptr<V> v_ptr = new_value
            ? new_val_it->lock()
            : std::make_shared<V>(static_cast<V>(v));

    auto val_ins = range.insert(v_ptr);
    new_val_it = val_ins.first;
    bool val_ins_true = val_ins.second;
    
    //f(a) = v
    try {
        if (found)
            it->replace_value(v_ptr);
        else
            it = fun.insert(point_type{a_ptr, v_ptr}).first;
    } catch(...) {
        if(val_ins_true)
            range.erase(new_val_it);
        throw;
    }

    iterator left = end(), right = end();
    bool left_exist = it != begin();
    bool right_exist = std::next(it) != end();
    
    if(left_exist) left = std::prev(it);
    if(right_exist) right = std::next(it);

    bool will_be_max = maximum_check(it);
    bool will_be_max_l = left_exist ? maximum_check(left) : false;
    bool will_be_max_r = right_exist ? maximum_check(right) : false;

    
    mx_iterator max_position_r = mx_end();
    mx_iterator max_position_l = mx_end();
    bool was_maximum = max_position != mx_end();
    bool was_maximum_l = false, was_maximum_r = false;

    try{
        if (left_exist){
            max_position_l = mx_find(*left);
            was_maximum_l = max_position_l != mx_end();
        }
        if (right_exist){
            max_position_r = mx_find(*right);
            was_maximum_r = max_position_r != mx_end();
        }
    }
    catch(...){
        if (found)
            it->replace_value(v_ptr_old->lock());
        else
            fun.erase(it);

        if(val_ins_true)
            range.erase(new_val_it);
        throw;
    }
    
    //określanie co ma się zdarzyć
    bool should_be_erased = false;
    bool should_be_erased_l = false;
    bool should_be_erased_r = false;

    bool should_be_inserted = false;
    bool should_be_inserted_l = false;
    bool should_be_inserted_r = false;

    //jeżeli było i będzie maksimum
    //to mówi, by usunąć na koniec starą wartość
    bool specific_erase =false;

    if (will_be_max){
        if (was_maximum){
            should_be_inserted = true;
            specific_erase = true;
        }
        else
            should_be_inserted = true;}
    else if (was_maximum)
        should_be_erased = true;

    if(left_exist){
        if (will_be_max_l && !was_maximum_l)
            should_be_inserted_l = true;
        else if (!will_be_max_l && was_maximum_l)
            should_be_erased_l = true;
    }
    if(right_exist){
        if (will_be_max_r && !was_maximum_r)
            should_be_inserted_r = true;
        else if (!will_be_max_r && was_maximum_r)
            should_be_erased_r = true;
    }

    //iteratory do bezpiecznego cofania insercji
    mx_iterator inserted = mx_end();
    mx_iterator inserted_l = mx_end();
    mx_iterator inserted_r = mx_end();

    bool done = false;
    bool done_l = false;
    bool done_r = false;
                
    //insercje maximów
    try {//aktualizowanie głównego
        if(should_be_inserted){
            inserted = maxima.insert(*it).first;
            done = true;
        }    
        try{//aktualizowanie lewego
            if(should_be_inserted_l){
                inserted_l = maxima.insert(*left).first;
                done_l = true;
            }  
            try{//aktualizowanie prawego
                if(should_be_inserted_r){
                    inserted_r = maxima.insert(*right).first;
                    done_r = true;
                }  
            }
            catch(...){
                //cofanie prawego
                 if(should_be_inserted_r && done_r)
                     maxima.erase(inserted_r);
                throw;
            }
        }
        catch(...)
        {   //cofanie lewego
            if(should_be_inserted_l && done_l)
                maxima.erase(inserted_l);
            throw;
        }
    }
    catch (...) {
        //cofanie głównego
        if(should_be_inserted && done)
            maxima.erase(inserted);

        if (will_be_max && was_maximum)
            max_position->replace_value(v_ptr_old->lock());

        //przywracanie wartości
        if (found)
            it->replace_value(v_ptr_old->lock());
        else
            fun.erase(it);

        if(val_ins_true)
            range.erase(new_val_it);

        throw;
    }

    //erasy maximow
    if(should_be_erased)
        maxima.erase(max_position);
    if(should_be_erased_l)
        maxima.erase(max_position_l);
    if(should_be_erased_r)
        maxima.erase(max_position_r);

    if(specific_erase)
        maxima.erase(max_position);

    //usuwanie ze zbioru wartości
    if (found && v_ptr_old->expired())
            range.erase(v_ptr_old);
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
            if (rg_it == rg_end())
                assert(false);
            if (to_erase != begin()) {
                iterator left = std::prev(to_erase);
                if (is_maximum(left, to_erase))
                    left_mx = maxima.insert(*left).first;
                else {
                    left_mx = maxima.find(*left);
                    if (left_mx != mx_end())
                        should_erase_left_mx = true;
                }
            }
            iterator right = std::next(to_erase);
            if (right != end()) {
                if (is_maximum(right, to_erase))
                    right_mx = maxima.insert(*right).first;
                else {
                    right_mx = maxima.find(*right);
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
        if (rg_it->expired())
            range.erase(rg_it);
        if (should_erase_left_mx)
            maxima.erase(left_mx);
        if (should_erase_right_mx)
            maxima.erase(right_mx);
    }
}

#endif // FUNCTION_MAXIMA_H