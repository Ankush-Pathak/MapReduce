//
// Created by ankpath on 9/13/22.
//

#ifndef ASSIGNMENT1_DATAVALUE_H
#define ASSIGNMENT1_DATAVALUE_H

#include <boost/serialization/vector.hpp>
#include <boost/container/string.hpp>
#include <iostream>
#include <cstddef>

BOOST_CLASS_IMPLEMENTATION(boost::container::string, boost::serialization::primitive_type)

class DataValue {
    std::string data;
    int flags;
    unsigned long expiryTime, dataLength;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive &archive, const unsigned int version) {
        archive & data;
        archive & flags;
        archive & expiryTime;
        archive & dataLength;
    }

public:
    DataValue(const std::string &data, const int flags, const unsigned long expiryTime, const unsigned long dataLength);

    const std::string getData() const;

    void setData(const std::string &data);

    void appendData(const std::string &data);

    int getFlags() const;

    void setFlags(int flags);

    unsigned long getExpiryTime() const;

    void setExpiryTime(unsigned long expiryTime);

    unsigned long getDataLength() const;

    void setDataLength(unsigned long dataLength);

    DataValue();


};


#endif //ASSIGNMENT1_DATAVALUE_H
