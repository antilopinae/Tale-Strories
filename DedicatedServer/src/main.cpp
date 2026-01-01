#include <iostream>
#include <string>
#include <grpcpp/grpcpp.h>
#include "game.grpc.pb.h" // Твой сгенерированный заголовок

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using game::DedicatedService;
using game::PingRequest;
using game::PingResponse;

class DedicatedServiceImpl final : public DedicatedService::Service {
    Status Ping(ServerContext *context, const PingRequest *request, PingResponse *response) override {
        std::cout << "🎮 Received Ping from client!" << std::endl;
        response->set_server_time(123456789); // Просто заглушка
        return Status::OK;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:9000"); // Порт внутри контейнера всегда 9000
    DedicatedServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "🚀 Dedicated Server listening on " << server_address << std::endl;
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}
