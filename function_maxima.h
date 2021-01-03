#ifndef MAKSIMA_FUNCTION_MAXIMA_H
#define MAKSIMA_FUNCTION_MAXIMA_H

//Zakładamy, że
//        * klasy A i V mają konstruktory kopiujące;
//* dostępne są:
//bool operator<(A const& x, A const &y);
//bool operator<(V const& x, V const &y);
//* w powyższym opisie równość a i b oznacza !(a < b) && !(b < a);
//* generalnie, o porządku < zakładamy to samo, co domyślnie zakłada np. std::set.

//Dodatkowo:
//* Wszystkie operacje na funkcji powinny gwarantować silną odporność
//na wyjątki, a tam gdzie jest to możliwe i pożądane, powinny być no-throw.
//* Klasa powinna być przezroczysta na wyjątki, czyli powinna przepuszczać
//wszelkie wyjątki zgłaszane przez wywoływane przez nią funkcje i przez
//operacje na jej składowych.
//* Uznajemy, że argumenty i wartości funkcji mogą być obiektami niemałych
//        rozmiarów. Reprezentacja funkcji powinna zatem utrzymywać jak najmniej
//        kopii argumentów i wartości. W szczególności, dobrze byłoby, aby funkcja
//będąca kopią innej funkcji współdzieliła z nią argumenty i wartości.
//* Wycieki pamięci są zabronione. :)
//* Klasa InvalidArg powinna dziedziczyć po std::exception.

#include <utility>
#include <set>
#include <memory>
#include <cassert>

class InvalidArg : std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "invalid argument value";
    }
};

template<typename A, typename V>
class FunctionMaxima {
public:
    // Nie powinno być możliwe bezpośrednie konstruowanie obiektów typu
    // point_type, ale zezwalamy na ich kopiowanie i przypisywanie.
    class point_type {
    private:
        std::shared_ptr<A> arg_ptr;
        std::shared_ptr<V> value_ptr;
        explicit point_type(A arg, V value)
            : arg_ptr(std::make_shared(arg)), value_ptr(std::make_shared(value)) {}
        explicit point_type(std::shared_ptr<A> arg, std::shared_ptr<V> value)
            : arg_ptr(arg), value_ptr(value) {}
        friend class FunctionMaxima;
//        friend V const& FunctionMaxima::value_at(A const&);
    public:
        point_type(const point_type&) = default;
        point_type& operator= (const point_type&) = default;
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

    using function_set = std::set<point_type, std::less<point_type>>;

    struct maxima_order { // TODO: czy możemy używać operatora > ?
        bool operator()(const point_type& x, const point_type& y) const {
            return *x.value() > *y.value() || (!(*y.value() < *x.value()) && *x.arg() < *y.arg());
        }
    };

    using maxima_set = std::set<point_type, maxima_order>;

//    Typ iterator zachowujący się tak jak bidirectional_iterator
//    (http://www.cplusplus.com/reference/iterator/BidirectionalIterator),
//            iterujący po punktach funkcji.
//    Dla zmiennej it typu wyrażenie *it powinno być typu point_type const&.
    using iterator = typename function_set::const_iterator;

//    Metody dające dostęp do punktów funkcji:
    // iterator wskazujący na pierwszy punkt
    iterator begin() const noexcept {
        return fun.cbegin();
    }

    // iterator wskazujący za ostatni punkt
    iterator end() const noexcept {
        return fun.cend();
    }

    // Iterator, który wskazuje na punkt funkcji o argumencie a lub end(),
    // jeśli takiego argumentu nie ma w dziedzinie funkcji.
    iterator find(A const& a) const {
        std::shared_ptr<A> a_ptr = std::make_shared<A>(static_cast<A>(a));
        return fun.find(point_type{a_ptr, std::shared_ptr<V>{}});
    }

    // Typ mx_iterator zachowujący się znów jak bidirectional_iterator,
    // iterujący po lokalnych maksimach funkcji.
    // Dla zmiennej it typu wyrażenie *it powinno być typu point_type const&.

    using mx_iterator = typename maxima_set::const_iterator;

    // iterator wskazujący na pierwsze lokalne maksimum
    mx_iterator mx_begin() const noexcept {
        return maxima.cbegin();
    }

    // iterator wskazujący za ostatnie lokalne maksimum
    mx_iterator mx_end() const noexcept {
        return maxima.cend();
    }

//    Jeśli przez k oznaczymy rozmiar zbioru lokalnych maksimów, to powyższe
//            metody powinny działać w czasie nie gorszym niż O(log k).
//    Przejście (nieprzeplatane z modyfikacją funkcji) po wszystkich maksimach
//            funkcji powinno odbywać się w czasie O(k), w kolejności malejących wartości.
//    W szczególności oznacza to, że w przypadku funkcji F o niepustej dziedzinie
//            F.mx_begin()->value() jest największą wartością tej funkcji.
//
//    W przypadku takich samych wartości dwóch maksimów, najpierw powinniśmy
//    dotrzeć do punktu o mniejszym argumencie.

//    Typ size_type reprezentujący rozmiar dziedziny i funkcja zwracająca ten
//            rozmiar:
    using size_type = typename function_set::size_type;

    size_type size() const noexcept {
        return fun.size();
    }

    // Konstruktor bezparametrowy (tworzy funkcję o pustej dziedzinie),
    //  konstruktor kopiujący i operator=. Dwa ostatnie powinny mieć
    //  sensowne działanie.
    FunctionMaxima() = default;
    FunctionMaxima(const FunctionMaxima&) = default;
    FunctionMaxima& operator=(const FunctionMaxima&) = default;

    // Zwraca wartość w punkcie a, rzuca wyjątek InvalidArg, jeśli a nie
    // należy do dziedziny funkcji. Złożoność najwyżej O(log n).
    V const& value_at(A const& a) const {
        iterator it = find(a);
        if (it == fun.end())
            throw InvalidArg();
        else
            return it->value();
    }

    // Zmienia funkcję tak, żeby zachodziło f(a) = v. Jeśli a nie należy do
    // obecnej dziedziny funkcji, jest do niej dodawany. Najwyżej O(log n).
    void set_value(A const& a, V const& v) {
        function_set fun_save = fun;
        maxima_set maxima_save = maxima;

        std::shared_ptr<A> a_ptr = std::make_shared<A>(static_cast<A>(a));
        std::shared_ptr<V> v_ptr = std::make_shared<V>(static_cast<V>(v));
        
        iterator here = fun.find(point_type{a_ptr, std::shared_ptr<V>{}});
        bool found = here != fun.end();
        //mx_iterator in_maxima = maxima.find(point_type{a_ptr, std::shared_ptr<V>{}});
        //bool was_maximum = in_maxima != maxima.end();
        //old_value = *here;
        auto new_value = point_type{a_ptr, v_ptr};
        
        if (found)
            *here = new_value;
        else 
            here = fun.insert(new_value);

        try{
            set_maximum(here);

            if(here != fun.begin()){
                iterator left = here;
                --left;
                set_maximum(left);
            }

            if(here != fun.end()){
                iterator right = here;
                ++here;
                set_maximum(right);
            }
        }
        catch(...){
            fun = fun_save;
            maxima = maxima_save;
            throw;
        }
            
    }

    // TODO
    // Usuwa a z dziedziny funkcji. Jeśli a nie należało do dziedziny funkcji,
    // nie dzieje się nic. Złożoność najwyżej O(log n).
    void erase(A const& a) {
        iterator it = find(a);
        if (it != end()) {
            mx_iterator mx_it = maxima.find(*it);
            try {
                if (mx_it != mx_end())
                    maxima.erase(mx_it);
                fun.erase(it);
            } catch (...) {

            }
        }
    }
private:
    function_set fun;
    maxima_set maxima;


    // nie moje wersje ~Wojciech
    //Pomocnicze funkcje, określające czy
    //w danym miejscu jest maximum lokalne
    bool left_check(iterator it){
        if(it == fun.begin()) return true;
        iterator left = it; --left;
        return !(it->value() < left->value()); //możliwy wyjątek w >
    }
    bool right_check(iterator it){
        if(it == fun.end()) return true;
        iterator right = it; ++right;
        return !(it->value() < right->value()); //możliwy wyjątek w >
    }

    bool maximum_check(iterator it){
        return left_check(it) && right_check(it); //dziedziczy wyjatki
    }

    //może miotać wyjątkami na prawo i lewo
    void set_maximum(iterator it){
        if (maximum_check(it)){
            auto here = maxima.find(*it);
            if (here == maxima.end())
                maxima.insert(*it);
            else
                *here = *it;
        }
        else
            maxima.erase(*it);
    }


    // Moje wersje ~Wojciech
    bool left_check(iterator it, iterator to_erase) {
        if (it == fun.begin())
            return true;
        iterator left = it; --left;
        if (left == to_erase) {
            if (left == fun.begin())
                return true;
            else
                --left;
        }
        return !(it->value() < left->value()); //możliwy wyjątek w <
    }
    bool right_check(iterator it, iterator to_erase) {
        iterator right = it; ++right;
        if (right == fun.end())
            return true;
        if (right == to_erase) {
            if (++right == fun.end())
                return true;
        }
        return !(it->value() < right->value()); //możliwy wyjątek w <
    }
    bool is_maximum(iterator it, iterator to_erase = nullptr) {
        return left_check(it, to_erase) && right_check(it, to_erase);
    }
};
#endif //MAKSIMA_FUNCTION_MAXIMA_H