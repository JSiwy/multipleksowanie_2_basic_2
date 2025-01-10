#include "Nastawnik.h"

Nastawnik::Nastawnik() {
  for (int i = 0; i < 49; i++) {
    positions[i].push_back({ -1 });
  }
}

void Nastawnik::addPosition(int index, long values[]) {
  if (index >= 0 && index < positions.size()) {
    Vector<long> val;
    for (int i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
      val.push_back(values[i]);
    }
    positions[index] = val;
  }
}

int Nastawnik::getPos(long value) {
  for (int i = 0; i < positions.size(); i++) {
    for (int j = 0; j < positions[i].size(); j++) {
      if (value == positions[i][j]) {
        return i; // Return the index of the position
      }
    }
  }
  return -1;
}