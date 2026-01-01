#pragma once
#include <string>
#include <memory>
#include <vector>

class GrpcReproxer {
public:
    GrpcReproxer(const std::string &lobby_url);

    ~GrpcReproxer();

    // 1. Авторизация: передаем code и redirect_uri
    bool Authenticate(const std::string &auth_code, const std::string &redirect_uri);

    // 2. Лобби: запрос на вход в комнату. Возвращает адрес выделенного сервера.
    bool JoinRoom(const std::string &room_name, std::string &out_server_addr);

    // 3. Переключение на выделенный сервер (Dedicated)
    void ConnectToDedicated(const std::string &dedicated_addr);

    // Тестовый метод для выделенного сервера
    bool PingDedicated(int64_t &out_server_time);

private:
    std::string jwt_token_;
    struct Impl;
    Impl *impl_;
};
