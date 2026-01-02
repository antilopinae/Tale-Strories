#include "lib_ue.hpp"
#include <fmt/core.h>

#pragma push_macro("check")
#undef check
#include <grpcpp/grpcpp.h>
#include "game.grpc.pb.h"
#pragma pop_macro("check")

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

// Подключаем все три сервиса из обновленного proto
using game::AuthService;
using game::LobbyService;
using game::DedicatedService;
using game::PingRequest;
using game::PingResponse;
// using grpc::Server;
// using grpc::ServerBuilder;
// using grpc::ServerContext;

struct GrpcReproxer::Impl {
    std::shared_ptr<Channel> lobby_channel;
    std::unique_ptr<AuthService::Stub> auth_stub;
    std::unique_ptr<LobbyService::Stub> lobby_stub;

    std::shared_ptr<Channel> dedicated_channel;
    std::unique_ptr<DedicatedService::Stub> dedicated_stub;
};

GrpcReproxer::GrpcReproxer(const std::string &lobby_url) {
    impl_ = new Impl();
    impl_->lobby_channel = grpc::CreateChannel(lobby_url, grpc::InsecureChannelCredentials());
    impl_->auth_stub = AuthService::NewStub(impl_->lobby_channel);
    impl_->lobby_stub = LobbyService::NewStub(impl_->lobby_channel);
}

GrpcReproxer::~GrpcReproxer() {
    delete impl_;
}

// ШАГ 1: Обмен Auth Code на JWT
bool GrpcReproxer::Authenticate(const std::string &auth_code, const std::string &redirect_uri) {
    game::GoogleAuthRequest request;
    request.set_auth_code(auth_code);
    request.set_redirect_uri(redirect_uri);

    game::AuthResponse response;
    ClientContext context;

    Status status = impl_->auth_stub->AuthenticateWithGoogle(&context, request, &response);

    if (status.ok()) {
        this->jwt_token_ = response.access_token();
        fmt::print("✅ Auth Success! JWT obtained.\n");
        return true;
    }
    fmt::print("❌ Auth Failed: {}\n", status.error_message());
    return false;
}

// ШАГ 2: Запрос комнаты у Лобби (Kotlin)
bool GrpcReproxer::JoinRoom(const std::string &room_name, std::string &out_server_addr) {
    if (jwt_token_.empty()) return false;

    game::JoinRoomRequest request;
    request.set_room_name(room_name);

    game::JoinRoomResponse response;
    ClientContext context;
    context.AddMetadata("authorization", "Bearer " + jwt_token_);

    Status status = impl_->lobby_stub->JoinRoom(&context, request, &response);

    if (status.ok() && response.status() == game::ResponseStatus::OK) {
        out_server_addr = response.server_info().address();
        fmt::print("🏠 Lobby: Room joined. Target server: {}\n", out_server_addr);
        return true;
    }
    fmt::print("❌ Lobby Error: {}\n", response.message());
    return false;
}

// ШАГ 3: Подключение к созданному Docker-контейнеру (C++)
void GrpcReproxer::ConnectToDedicated(const std::string &dedicated_addr) {
    impl_->dedicated_channel = grpc::CreateChannel(dedicated_addr, grpc::InsecureChannelCredentials());
    impl_->dedicated_stub = DedicatedService::NewStub(impl_->dedicated_channel);
    fmt::print("🚀 Connected to Dedicated Server: {}\n", dedicated_addr);
}

// Пример запроса к выделенному серверу
bool GrpcReproxer::PingDedicated(int64_t &out_server_time) {
    if (!impl_->dedicated_stub) return false;

    game::PingRequest request;
    request.set_client_time(12345); // пример

    game::PingResponse response;
    ClientContext context;
    context.AddMetadata("authorization", "Bearer " + jwt_token_);

    Status status = impl_->dedicated_stub->Ping(&context, request, &response);
    if (status.ok()) {
        out_server_time = response.server_time();
        return true;
    }
    return false;
}

// // 1. Реализация логики сервиса
// class DedicatedServiceImpl final : public DedicatedService::Service {
//     Status Ping(ServerContext* context, const PingRequest* request, PingResponse* response) override {
//         // Логика обработки пинга
//         response->set_server_time(std::time(nullptr));
//         return Status::OK;
//     }
//
//     // Сюда можно добавить JoinMatch, SendChat и т.д.
// };
//
// struct DedicatedServerWrapper::Impl {
//     std::unique_ptr<Server> server;
// };
//
// DedicatedServerWrapper::DedicatedServerWrapper() : impl_(std::make_unique<Impl>()) {}
//
// DedicatedServerWrapper::~DedicatedServerWrapper() {
//     Stop();
// }
//
// void DedicatedServerWrapper::Start(int32_t port) {
//     if (bIsRunning) return;
//
//     // Запускаем сервер в отдельном std::thread
//     server_thread_ = std::make_unique<std::thread>([this, port]() {
//         std::string server_address = "0.0.0.0:" + std::to_string(port);
//         DedicatedServiceImpl service;
//
//         ServerBuilder builder;
//         builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
//         builder.RegisterService(&service);
//
//         impl_->server = builder.BuildAndStart();
//         this->bIsRunning = true;
//
//         std::cout << "Dedicated Server started on " << server_address << std::endl;
//
//         impl_->server->Wait(); // Это блокирующий вызов, поэтому мы в отдельном потоке
//     });
// }
//
// void DedicatedServerWrapper::Stop() {
//     if (impl_->server) {
//         impl_->server->Shutdown();
//         if (server_thread_ && server_thread_->joinable()) {
//             server_thread_->join();
//         }
//         bIsRunning = false;
//     }
// }
