#include <string>
#include "format.h"
#include <iomanip>

using std::string;
string Format::ElapsedTime(long seconds) {
  int HH = seconds / 3600;
  int MM = (seconds % 3600) / 60;
  int SS = (seconds % 3600) % 60;

  std::ostringstream stream;
  stream << std::setw(2) << std::setfill('0') << HH << ":" <<
      std::setw(2) << std::setfill('0') << MM << ":" <<
      std::setw(2) << std::setfill('0') << SS;
  return stream.str();
}