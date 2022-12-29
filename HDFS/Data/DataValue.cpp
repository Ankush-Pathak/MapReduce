//
// Created by ankpath on 9/13/22.
//

#include "DataValue.h"

DataValue::DataValue(const std::string &data, const int flags, const unsigned long expiryTime,
                     const unsigned long dataLength) : flags(flags), expiryTime(expiryTime), dataLength(dataLength), data(data) {
}


const std::string DataValue::getData() const {
    return data;
}

void DataValue::setData(const std::string &data) {
    this->data = data;
}

int DataValue::getFlags() const {
    return flags;
}

void DataValue::setFlags(int flags) {
    DataValue::flags = flags;
}

unsigned long DataValue::getExpiryTime() const {
    return expiryTime;
}

void DataValue::setExpiryTime(unsigned long expiryTime) {
    DataValue::expiryTime = expiryTime;
}

unsigned long DataValue::getDataLength() const {
    return dataLength;
}

void DataValue::setDataLength(unsigned long dataLength) {
    DataValue::dataLength = dataLength;
}

DataValue::DataValue() {

}

void DataValue::appendData(const std::string &data) {
    this->data.append(data);
    dataLength = this->data.length();

}
