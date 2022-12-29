//
// Created by ankpath on 10/5/22.
//
#include <numeric>
#include "Reduce.h"


std::string Reduce::reduce(const std::string &key, const std::vector<std::string> &values) {
    long sum = 0;
    for (std::string value: values)
        sum += std::stoi(value);
    return std::to_string(sum);
}