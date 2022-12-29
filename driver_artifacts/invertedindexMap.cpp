//
// Created by ankpath on 10/2/22.
//


#include "Map.h"
#include <sstream>

void Map::map(const std::string &lineOffset, const std::string &line, pairs &keyValuePairs) {
    std::stringstream stringStream(line);
    std::string word;
    while(getline(stringStream, word, ' ')) {
        ll position = std::stoi(lineOffset) + stringStream.tellg() - word.size();
        if(stringStream.eof())
            position = std::stoi(lineOffset) + line.size() - word.size() + 1;
        keyValuePairs.push_back({word, std::to_string(position)});
    }
}
