//
// Created by ankpath on 10/5/22.
//
#include <numeric>
#include "Reduce.h"


std::string Reduce::reduce(const std::string &key, const std::vector<std::string> &values) {
    std::string result;
    for(const std::string &value: values)
        result.append(" " + value);
    return result;
}