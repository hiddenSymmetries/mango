#include<string>
#include "mango.h"

bool mango::string_to_algorithm(std::string str, mango::algorithm_type* algorithm) {
  bool found_match = false;
  algorithm_properties properties;

  for (int j = 0; j < NUM_ALGORITHMS; j++) {
    get_algorithm_properties((algorithm_type) j, &properties);
    if (properties.name_string.compare(str) == 0) {
      found_match = true;
      *algorithm = (algorithm_type) j;
    }
  }

  return found_match;
}
