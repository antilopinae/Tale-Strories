# DedicatedServer/Dockerfile
FROM ubuntu:22.04 AS build-env

# Ставим зависимости (добавил ninja-build для ускорения и g++ для компиляции)
RUN apt-get update && apt-get install -y \
    build-essential cmake git curl zip unzip tar pkg-config ca-certificates \
    ninja-build g++-11

# Устанавливаем переменные окружения для компилятора
ENV CC=gcc
ENV CXX=g++

WORKDIR /src

# 1. Копируем vcpkg
COPY ThirdParty/vcpkg ThirdParty/vcpkg

# --- КРИТИЧЕСКИЙ ШАГ: Пересобираем vcpkg под Linux ---
RUN chmod +x ThirdParty/vcpkg/bootstrap-vcpkg.sh && \
    ./ThirdParty/vcpkg/bootstrap-vcpkg.sh

# 2. Копируем исходники и прото-файлы
COPY DedicatedServer DedicatedServer
COPY Api Api
COPY ThirdParty/cmake ThirdParty/cmake

# 3. Сборка (используем Ninja для надежности)
WORKDIR /src/DedicatedServer
RUN cmake -G Ninja -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DVCPKG_TARGET_TRIPLET=arm64-linux # Для Mac M1/M2/M3 используем arm64-linux
RUN cmake --build build --config Release

# ФИНАЛЬНЫЙ ОБРАЗ
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y libssl3 ca-certificates && rm -rf /var/lib/apt/lists/*
WORKDIR /app

# Копируем бинарник из стадии сборки
COPY --from=build-env /src/DedicatedServer/build/game-server .

EXPOSE 9090
ENTRYPOINT ["./game-server"]