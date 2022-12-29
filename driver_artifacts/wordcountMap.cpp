//
// Created by ankpath on 10/2/22.
//


#include "Map.h"
#include <sstream>

void Map::map(const std::string &lineOffset, const std::string &line, pairs &keyValuePairs) {
    std::stringstream stringStream(line);
    std::string word;
    while(getline(stringStream, word, ' ')) {
        keyValuePairs.push_back({word, "1"});
    }
}