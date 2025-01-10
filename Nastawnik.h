#ifndef NASTAWNIK_H
#define NASTAWNIK_H

#include <Arduino.h>
#include <Vector.h>


class Nastawnik {
  private:
    Vector<Vector<long>> positions;

  public:
    Nastawnik();

    void addPosition(int index, long values[]);

    int getPos(long value);
};
#endif