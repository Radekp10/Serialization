//Program prezentujacy dzialanie serializacji obiektow w pliku tekstowym
//z wykorzystaniem biblioteki boost

//Struktura programu: mamy klase Sklep zawierajaca kontener obiektow klasy Produkt
//Najpierw tworzony jest obiekt klasy Sklep i dodawane sa do niego produkty
//po czym nastepuje zapis danych sklepu do pliku produkty.txt i nastepnie odczytanie ich
//dla kolejnego nowo utworzonego obiektu klasy Sklep

#include <fstream>
#include <iostream>
#include <string>

#include <boost/archive/text_oarchive.hpp>      //zapisywanie archwium w postaci tekstowej
#include <boost/archive/text_iarchive.hpp>

#include <boost/serialization/deque.hpp>        //obsluga kontenera (listy)

#include <boost/serialization/split_free.hpp>   //nag³owek zawieraj¹cy metode(makro) do rozdzielenia funkcji serializacji
#include <exception>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Produkt
{
private:
    string nazwa;                       //nazwa produktu
    string firma;                       //nazwa firmy
    double cena;                        //cena produktu
    int dostepne; 					    //liczba produktow dostepnych w sklepie
public:
    Produkt(const string & n = "brak", const string & f = "brak", double c = 0.0, int dost = 0)      //konstruktor
    {
        nazwa=n; firma=f; cena=c; dostepne=dost;        //inicjalizacja skladowych
    }

    ~Produkt() {}        		        //destruktor
    void wyswietl() const               //funkcja do wyswietlania cech produktu
    {
        cout << "Produkt: " << nazwa << endl;
        cout << "Firma: " << firma << endl;
        cout << "Cena: " << cena << " zl" << endl;
        cout << "Liczba dostepnych produktow: " << dostepne << endl << endl;
    }

    string& pobierznazwe() { return nazwa; }        //funkcje zwracajace dane skladowe
	string& pobierzfirme() { return firma; }
	double  pobierzcene() { return cena;}
	int pobierzdostepne() { return dostepne;}

	void ustawnazwe(const string& n) { nazwa = n; } //funkcje ustawiajace dane skladowe
	void ustawfirme(const string& f) {firma = f; }
	void ustawcene (double c) {cena = c;}
	void ustawdostepne (int d) {dostepne = d;}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace boost
{
	namespace serialization
	{                                               //metoda serializacji umo¿liwiajaca zapisz/odczyt atrybutów prywatnych
                                                    //najpierw trzeba rozczepic domyœlna metode serialize na dwie : save i load, a nastêpnie po³¹czyc je w ca³oœæ
		template<class Archive>
		void save(Archive & ar,  const Produkt& p, const unsigned int wersja)
		{
			Produkt p2 = const_cast<Produkt&>(p);	// usuniêcie const, aby mo¿na by³o wywo³aæ metodê pobierz
			ar << p2.pobierznazwe();                //pobranie danych i zapisanie do archiwum tekstowego
			ar << p2.pobierzfirme();
			ar << p2.pobierzcene();
			ar << p2.pobierzdostepne();
		}

		template<class Archive>
		void load(Archive & ar, Produkt& p, const unsigned int wersja)
		{
			string s1, s2;
			double d;
			int i;
			ar >> s1;                               //odczyt z archiwum
			ar >> s2;
			ar >> d;
			ar >> i;
			p.ustawnazwe(s1);                       //ustawienie odczytanych danych
			p.ustawfirme(s2);
			p.ustawcene(d);
			p.ustawdostepne(i);
		}

		template<class Archive>
		inline void serialize(Archive & ar,	Produkt& p, const unsigned int wersja)
		{
			split_free(ar, p, wersja);     // z³¹czenie load/save -> serialize
		}

	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Sklep
{
private:
    string nazwa_sklepu;                                    //pole z nazwa sklepu
    deque<Produkt *> produkty;                              //kontener (lista) przechowujacy produkty dostepne w sklepie

	friend class boost::serialization::access;              //metoda serializacji ma dostêp do atrybutów prywatnych

	template <class Archive>                                //wzorzec
	void serialize(Archive& ar, const unsigned int wersja)	//serializacji
    {
		ar & nazwa_sklepu;		                            //dodawanie poszczególnych pol do archiwum
		ar & produkty;			                            //poprzez u¿ycie operatora '&' na archiwum
    }

public:
    Sklep() {}                                              //konstruktor domyslny
    Sklep(const string & n)
    {
        nazwa_sklepu=n;
    }

    ~Sklep() {}                                             //destruktor

    void dodajprodukt(Produkt* const p)                     //funkcja dodajaca nowy produkt do sklepu
	{
		produkty.push_back(p);
	}

    void przeglad_stanu_sklepu() const                      //metoda umozliwiajaca przeglad wszystkich dostepnych produktow w sklepie
    {
        cout << "\t\t***" << nazwa_sklepu << "***" << endl;
        cout << "Dostepne produkty: " << endl << endl;
        int i;
        int liczba_produktow = produkty.size();             //okreslenie liczby elementow kontenera, czyli liczby produktow
        for (i=0; i<liczba_produktow; i++)
        {
            produkty[i]->wyswietl();                        //wyswietlenie informacji o produkcie
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void zapisz_sklep(const Sklep &s, const char* filename)     //zapisywanie stanu sklepu do pliku
{
	try
	{
		ofstream ofs(filename);                             // stworzenie archiwum
		boost::archive::text_oarchive oa(ofs);
		oa << s;                                            // zapisanie w postaci archiwum tekstowego
	}
	catch(exception e) { cout << e.what() << endl; exit(1); }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void wczytaj_sklep(Sklep &s, const char* filename)          //oczytywanie stanu sklepu z pliku
{
	try
	{
		ifstream ifs(filename);                             // otworzenie archiwum
		boost::archive::text_iarchive ia(ifs);
		ia >> s;                                            // przywrocenie danych z uprzednio zapisanego archiwum
	}
	catch(exception e) {
		cout << e.what() << endl; exit(1); }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main()                                                  //testowanie serializacji obiektow
{
	const char* filename = "produkty.txt";                  //plik tekstowy do ktorego beda zapisywane dane

    Produkt p1("Sok jablkowy", "Hortex", 4.99, 200);        //utworzenie obiektow - produkty
	Produkt p2("Herbata mrozona", "Nestea", 3.20, 100);
    Produkt p3("Woda mineralna", "Zywiec Zdroj", 2.59,500);

    Produkt * pp1 = &p1;			                        //utworzenie wskaznikow na obiekty
	Produkt * pp2 = &p2;
	Produkt * pp3 = &p3;

	Sklep s1("Sklep spozywczy Obiekt");                     //utworzenie obiektu sklep
	s1.dodajprodukt(pp1);                                   //dodanie produktow do sklepu
	s1.dodajprodukt(pp2);
	s1.dodajprodukt(pp3);

    zapisz_sklep(s1, filename);                         //zapis do archiwum tekstowego

	Sklep s2;                                           //utworzenie nowego obiektu sklep

    wczytaj_sklep(s2, filename);                        //odczyt z archiwum tekstowego i zapisanie danych do nowo utworzonego obiektu

	s2.przeglad_stanu_sklepu();                         //wyswietlenie produktow dostepnych w sklepie s2
                                                        //jesli odczyt z archiwum byl poprawny to powinnismy otrzymac te same produkty co byly w sklepie s1
	return 0;
}
