#pragma once
#include <string>
#include <memory>

class GrpcReproxer {
public:
    GrpcReproxer(const std::string &target_url);

    // Главный метод подключения:
    // 1. Стучится в AuthService.AuthenticateWithGoogle(google_token)
    // 2. Получает JWT и сохраняет его внутри класса
    bool ConnectToServer(const std::string &google_id_token);

    // Пример игрового метода, который уже использует сохраненный JWT
    bool JoinGameSession(std::string &out_session_id);

    ~GrpcReproxer();

private:
    std::string target_url_;
    std::string jwt_token_; // Тот самый Game JWT

    // Смарт-пойнтеры для стабов (генерация из proto)
    // Мы определим их в .cpp, чтобы не засорять хедер лишними инклудами
    struct Impl;
    Impl* impl_;
};
