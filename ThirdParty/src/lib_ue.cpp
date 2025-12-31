#include <lib_ue.hpp>
#include <fmt/core.h>

#pragma push_macro("check")
#undef check

#include <grpcpp/grpcpp.h>
#include "game.grpc.pb.h"

#pragma pop_macro("check")

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using game::GameService;
using game::JoinRequest;
using game::JoinResponse;

int AddNumbers(int a, int b) {
    auto channel = grpc::CreateChannel("localhost:9090", grpc::InsecureChannelCredentials());
    auto stub = GameService::NewStub(channel);

    // 2. Готовим запрос (JoinSession)
    JoinRequest request;
    request.set_playerid("Unreal_User");

    JoinResponse response;
    ClientContext context;

    // 3. Выполняем вызов
    Status status = stub->JoinSession(&context, request, &response);

    if (status.ok()) {
        // Если успешно, возвращаем сумму + логируем ID сессии
        fmt::print("gRPC Success! SessionID: {}\n", response.sessionid());
        return a + b;
    } else {
        fmt::print("gRPC Failed: {}\n", status.error_message());
        return -1;
    }
    return a + b;
}