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

template<typename A, typename V>
class FunctionMaxima {
public:
    // Nie powinno być możliwe bezpośrednie konstruowanie obiektów typu
    // point_type, ale zezwalamy na ich kopiowanie i przypisywanie.
    class point_type {
    private:
        A _arg;
        V _value;
        explicit point_type(A arg, V value) : _arg(arg), _value(value) {}
    public:
        point_type(const point_type&) = default;
        point_type(point_type&&) noexcept = default;
        point_type& operator= (const point_type&) = default;
        point_type& operator= (point_type&&) noexcept = default;
        // Zwraca argument funkcji.
        A const& arg() const {
            return _arg;
        }
        // Zwraca wartość funkcji w tym punkcie.
        V const& value() const {
            return _value;
        }
        // Zadaje porządek liniowy na point_type identyczny z porządkiem na argumentach.
        bool operator< (point_type const& other) {
            return _arg < other._arg;
        }
    };

    //TOREAD
    //Ogólnie chyba stosowanie mapy zamiast setu znacząco ułatwi nam zadanie
    //wtedy wystarczy zrobić:
    //  using point_type = pair<A, V>;
    //  using arg() = first; //lub czegoś podobnego, to chyba nie zadziała przez ()
    //  using value = second;
    //* to odziedziczy operator porównania z pair
    //* wszystkie iteratory zadziałąja dobrze, bo takie rzeczy trzyma mapa
    //* wszystkie funkcje zmiany i dostepu beda natychmiast dziedziczone z mapy
    // tymsamym rozwiazuje problem pod TOCHECK

    template<typename T>
    bool same(const T& a, const T& b){
        return !(a < b) && !(b < a);
    }


    using function_set = std::set<point_type, std::less<point_type>>;

    struct maxima_order {
        bool operator() (const point_type& x, const point_type& y) const {
            return x.value() > y.value() || (!(y.value() < x.value()) && x.arg() < y.arg());
        }
    };

    using maxima_set = std::set<point_type, maxima_order>;

    class InvalidArg : std::exception {
        virtual const char* what() const throw(){
            return "invalid argument value"; 
        }
    };

    // Konstruktor bezparametrowy (tworzy funkcję o pustej dziedzinie),
    //  konstruktor kopiujący i operator=. Dwa ostatnie powinny mieć
    //  sensowne działanie.

    //Proponuje by funkcje kopiujące dzieliły fun i maxima,
    //a będziemy kopiować te rzeczy jak któryś coś w nich zmieni
    FunctionMaxima() = default; //TODO
    FunctionMaxima(const FunctionMaxima&) = default; //TODO
    FunctionMaxima(FunctionMaxima&&) noexcept = default; //TODO
    FunctionMaxima& operator=(const FunctionMaxima&) = default; //TODO
    FunctionMaxima& operator=(FunctionMaxima&&) noexcept = default; //TODO

    // Zwraca wartość w punkcie a, rzuca wyjątek InvalidArg, jeśli a nie
    // należy do dziedziny funkcji. Złożoność najwyżej O(log n).
    V const& value_at(A const& a) const { //TODO 
        
        V temp; 

    }

    // Zmienia funkcję tak, żeby zachodziło f(a) = v. Jeśli a nie należy do
    // obecnej dziedziny funkcji, jest do niej dodawany. Najwyżej O(log n).
    void set_value(A const& a, V const& v) { //TODO (pamietać o aktualizowaniu maxima)
        //zrobić kopie w zależności of counta na share_prt
    }

    // Usuwa a z dziedziny funkcji. Jeśli a nie należało do dziedziny funkcji,
    // nie dzieje się nic. Złożoność najwyżej O(log n).
    void erase(A const& a) { //TODO (pamietać o aktualizowaniu maxima)
        //zrobić kopie w zależności of counta na share_prt
    }

//    Typ iterator zachowujący się tak jak bidirectional_iterator
//    (http://www.cplusplus.com/reference/iterator/BidirectionalIterator),
//            iterujący po punktach funkcji.
//    Dla zmiennej it typu wyrażenie *it powinno być typu point_type const&.
    using iterator = typename function_set::const_iterator;

//    Metody dające dostęp do punktów funkcji:
    // iterator wskazujący na pierwszy punkt
    iterator begin() const noexcept {
        return fun.begin();
    }

    // iterator wskazujący za ostatni punkt
    iterator end() const noexcept {
        return fun.end();
    }

    // Iterator, który wskazuje na punkt funkcji o argumencie a lub end(),
    // jeśli takiego argumentu nie ma w dziedzinie funkcji.
    iterator find(A const& a) const noexcept {
        return fun.find(a); //to chyba nie działa, bo fun zapieroa point_type, a szukasz A
        //najłatwiej byłoby chyba zrobić point (a, cokolwiek) np. //TOCHECK
        V temp;
        return fun.find(point_type(a, temp));
    }

    // Typ mx_iterator zachowujący się znów jak bidirectional_iterator,
    // iterujący po lokalnych maksimach funkcji.
    // Dla zmiennej it typu wyrażenie *it powinno być typu point_type const&.

    using mx_iterator = typename maxima_set::const_iterator;

    // iterator wskazujący na pierwsze lokalne maksimum
    mx_iterator mx_begin() const noexcept {
        return maxima.begin();
    }

    // iterator wskazujący za ostatnie lokalne maksimum
    mx_iterator mx_end() const noexcept {
        return maxima.end();
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
private:
    function_set fun; // proponuję tu trzymać elementy stosując komparator argumentu
    maxima_set maxima; // tu zaś stosujemy komparator odwrotnie leksykograficzny,
                       // począwszy od drugiej współrzędnej
};
#endif //MAKSIMA_FUNCTION_MAXIMA_H
