//
// Created by ankpath on 10/1/22.
//

#ifndef ASSIGNMENT2_FILE_H
#define ASSIGNMENT2_FILE_H


#include <string>
#include <sstream>
#include <fstream>
#include "Common/ServiceAddress.h"
#include "Common/EnumsAndConstants.h"

class File {
    std::fstream fStream;
    std::stringstream stringStream;
    std::string filePath, host;
public:
    File operator=(const File &file);

    File(const File &file);

    File(const std::string &filePath, const std::string &host);

    void open(std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);

    const std::fstream &getFStream() const;

    const std::string &getFilePath() const;

    void setFilePath(const std::string &filePath);

    const std::string &getHost() const;

    void setHost(const std::string &host);

    ll populateHDFS(const ServiceAddress &keyValueStoreAddr, ll noOfLines, char sep = '\n');

    long getNoOfLines();
};


#endif //ASSIGNMENT2_FILE_H
