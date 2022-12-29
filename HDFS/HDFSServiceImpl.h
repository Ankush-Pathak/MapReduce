//
// Created by ankpath on 10/2/22.
//

#ifndef ASSIGNMENT2_HDFSSERVICEIMPL_H
#define ASSIGNMENT2_HDFSSERVICEIMPL_H

#include <thread>
#include "Common/Services/MapReduce.grpc.pb.h"
#include "HDFS/Data/DataStore.h"
#include "HDFS/Common/Utils.h"
#include "spdlog/spdlog.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class HDFSServiceImpl final : public HDFSService::Service {
    Status set(ServerContext *serverContext, const SetRequest *setRequest, SetResponse *setResponse) override {
        std::string response = dataStore->push_data(setRequest->key(),
                                                    DataValue(setRequest->value(), 0, 0, setRequest->value().size()),
                                                    false).getRawResponse();
        setResponse->set_message(response);
        return Status::OK;
    }

    Status setStream(ServerContext *context, ::grpc::ServerReader<::SetRequest> *reader, ::Empty *response) override {
        spdlog::info("Starting set stream");
        SetRequest setRequest;
        while(reader->Read(&setRequest)) {
            dataStore->push_data(setRequest.key(), DataValue(setRequest.value(), 0, 0, setRequest.value().size()));
        }
        spdlog::info("Ending set stream, data store size: {} {}", dataStore->size(), dataStore->getMap().size());

        return Status::OK;
    }


    Status append(ServerContext *serverContext, const SetRequest *setRequest, SetResponse *setResponse) override {
        dataStore->append(setRequest->key(), setRequest->value(), false);
        setResponse->set_message("STORED");
        return Status::OK;
    }

    Status streamAppend(::grpc::ServerContext *context, ::grpc::ServerReader<::SetRequest> *reader,
                        ::Empty *response) override {
        spdlog::info("Starting append stream");
        SetRequest setRequest;
        while(reader->Read(&setRequest)) {
            dataStore->append(setRequest.key(), setRequest.value(), true);
        }
        spdlog::info("Ending append stream, data store size: {} {}", dataStore->size(), dataStore->getMap().size());

        return Status::OK;
    }

    Status get(ServerContext *serverContext, const GetRequest *getRequest, GetResponse *getResponse) override {
        std::string value;
        if (dataStore->get(getRequest->key(), value)) {
            getResponse->set_value(value);
            return Status::OK;
        } else
            return {grpc::StatusCode::NOT_FOUND, "Not found"};
    }

    Status clear(ServerContext *serverContext, const Empty *empty, Empty *empty1) override {
        dataStore->clear();
        return Status::OK;
    }

    Status streamData(::grpc::ServerContext *context, const ::Empty *request,
                      ::grpc::ServerWriter<::StreamResponse> *writer) override {
        std::lock_guard lockGuard(THREAD_LOCK);
        spdlog::info("Streaming data");
        for(const auto& pair: dataStore->getMap()) {
            StreamResponse streamResponse;
            streamResponse.set_key(pair.first);
            streamResponse.set_value(pair.second.getData());
            writer->Write(streamResponse);
        }
        spdlog::info("Data streamed");
        return Status::OK;
    }

    Status getMetaData(::grpc::ServerContext *context, const ::Empty *request, ::HDFSMetaData *response) override {
        response->set_noofrecords(dataStore->size());
        return Status::OK;
    }

    Status die(::grpc::ServerContext *context, const ::Empty *request, ::Empty *response) override {
        std::thread imploder(&HDFSServiceImpl::hdfsAutolysis, this);
        imploder.detach();
        return Status::OK;
    }

    void hdfsAutolysis() {
        dataStore->flushDataStore(DATA_STORE_FILENAME);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        spdlog::info("Bye bye");
        exit(0);
    }
};


#endif //ASSIGNMENT2_HDFSSERVICEIMPL_H
