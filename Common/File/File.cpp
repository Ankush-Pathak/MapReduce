//
// Created by ankpath on 10/1/22.
//

#include <vector>
#include <sstream>
#include "File.h"
#include "MapReduce.grpc.pb.h"
#include "grpcpp/create_channel.h"
#include "spdlog/spdlog.h"
#include "Utils/Utils.h"

File::File(const File &file) {
    filePath = file.filePath;
    host = file.host;
}

File::File(const std::string &filePath, const std::string &host) : filePath(filePath), host(host) {}

const std::fstream &File::getFStream() const {
    return fStream;
}

const std::string &File::getFilePath() const {
    return filePath;
}

void File::setFilePath(const std::string &filePath) {
    File::filePath = filePath;
}

const std::string &File::getHost() const {
    return host;
}

void File::setHost(const std::string &host) {
    File::host = host;
}

File File::operator=(const File &file) {
    return File(file.filePath, file.host);
}

void File::open(std::ios_base::openmode mode) {
    fStream.open(filePath, mode);
    std::string fileContent((std::istreambuf_iterator<char>(fStream)), std::istreambuf_iterator<char>());
    stringStream = std::stringstream(fileContent);
}

ll File::populateHDFS(const ServiceAddress &keyValueStoreAddr, ll noOfLines, char sep) {
    ll linesPushed = 0;
    std::shared_ptr<HDFSService::Stub> stub = HDFSService::NewStub(
            grpc::CreateChannel(keyValueStoreAddr.getTuple(), grpc::InsecureChannelCredentials()));
    grpc::ClientContext clientContext;
    Empty empty;
    clientContext.set_deadline(getDeadline(5));
    grpc::Status status = stub->clear(&clientContext, empty, &empty);
    if (!status.ok()) {
        spdlog::error("Failed to clear data store");
        spdlog::error("{}: {}", status.error_message(), status.error_details());
    } else {
        spdlog::debug("Cleared data store");
    }

    if (!fStream.is_open() || !fStream.good()) {
        return linesPushed;
    }
    std::string buffer;
    Empty emptySet;
    grpc::ClientContext clientContextSet;
    clientContextSet.set_deadline(getDeadline(300));
    std::shared_ptr<HDFSService::Stub> localStub = HDFSService::NewStub(
            grpc::CreateChannel(keyValueStoreAddr.getTuple(), grpc::InsecureChannelCredentials()));
    std::shared_ptr<grpc::ClientWriter<SetRequest>> writer(localStub->setStream(&clientContextSet, &emptySet));
    while (std::getline(stringStream, buffer, sep) && noOfLines != 0) {
        SetRequest setRequest;
        setRequest.set_key(std::to_string(ll(stringStream.tellg()) - buffer.size() - 1));
        setRequest.set_value(buffer);
        writer->Write(setRequest);
        buffer.clear();
        if(noOfLines > 0)
            noOfLines--;
        linesPushed++;
    }
    writer->WritesDone();
    grpc::Status statusSet = writer->Finish();
    if (!statusSet.ok()) {
        spdlog::error("Failed to push offset and line to {}", keyValueStoreAddr.getTuple());
        spdlog::error("{} {}: {}", statusSet.error_code(), statusSet.error_message(), statusSet.error_details());
    } else {
        spdlog::info("Pushed: {} lines", linesPushed);
    }
    return linesPushed;
}

long File::getNoOfLines() {
    std::ifstream inputFile(filePath);
    inputFile.unsetf(std::ios_base::skipws);
    return std::count(std::istream_iterator<char>(inputFile), std::istream_iterator<char>(), '\n');
}
