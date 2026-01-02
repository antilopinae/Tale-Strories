#pragma once
#include <string>
#include <memory>
#include <vector>
#include <thread>

#if defined(_WIN32)
    #define LIBUE_API __declspec(dllexport)
#else
    #define LIBUE_API __attribute__((visibility("default")))
#endif

class LIBUE_API GrpcReproxer {
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

class LIBUE_API DedicatedServerWrapper {
public:
    DedicatedServerWrapper() {
    }

    ~DedicatedServerWrapper() {
    }

    // Запуск сервера в отдельном потоке, чтобы не вешать игру
    void Start(int32_t port) {
    }

    // Остановка сервера
    void Stop() {
    }

private:
    // struct Impl;
    // std::unique_ptr<Impl> impl_;
    std::unique_ptr<std::thread> server_thread_;
    bool bIsRunning = false;
};
