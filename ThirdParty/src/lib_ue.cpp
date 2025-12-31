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
using game::AuthService;
using game::GameService;

// Внутренняя реализация со стабами
struct GrpcReproxer::Impl {
    std::unique_ptr<AuthService::Stub> auth_stub;
    std::unique_ptr<GameService::Stub> game_stub;
};

GrpcReproxer::GrpcReproxer(const std::string& target_url) : target_url_(target_url) {
    auto channel = grpc::CreateChannel(target_url, grpc::InsecureChannelCredentials());
    impl_ = new Impl();
    impl_->auth_stub = AuthService::NewStub(channel);
    impl_->game_stub = GameService::NewStub(channel);
}

GrpcReproxer::~GrpcReproxer() {
    delete impl_;
}

bool GrpcReproxer::ConnectToServer(const std::string& google_id_token) {
    game::GoogleAuthRequest request;
    request.set_id_token(google_id_token);

    game::AuthResponse response;
    ClientContext context;

    // 1. Обмениваем Google Token на наш JWT
    Status status = impl_->auth_stub->AuthenticateWithGoogle(&context, request, &response);

    if (status.ok()) {
        this->jwt_token_ = response.access_token();
        fmt::print("Successfully authenticated! PlayerID: {}\n", response.player_id());
        return true;
    } else {
        fmt::print("Auth Failed: {}\n", status.error_message());
        return false;
    }
}

bool GrpcReproxer::JoinGameSession(std::string& out_session_id) {
    if (jwt_token_.empty()) return false;

    game::JoinRequest request; // Теперь он пустой по нашему новому proto
    game::JoinResponse response;
    ClientContext context;

    // ВАЖНО: Добавляем JWT в метаданные каждого игрового вызова
    context.AddMetadata("authorization", "Bearer " + jwt_token_);

    Status status = impl_->game_stub->JoinSession(&context, request, &response);

    if (status.ok()) {
        out_session_id = response.sessionid();
        return true;
    }
    return false;
}