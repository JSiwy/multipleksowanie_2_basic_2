#ifndef NASTAWNIK_H
#define NASTAWNIK_H

#include <Arduino.h>
#include <Vector.h>


class Nastawnik {
  private:
  // Prywatna właściwość wektor przechowujący pozycje
    Vector<Vector<long>> positions;

  public:
  // Dodanie publicznych pól do klasy Nastawnik
  // Konstruktor klasy Nastawnik
    Nastawnik();
  // Metoda dodająca pozycję do listy pozycji razem z indeksem  
    void addPosition(int index, long values[]);
  // Metoda zwracająca pozycję indeks pozycji na podstawie wartości
    int getPos(long value);
};
#endif