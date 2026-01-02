#!/bin/bash

# 1. Путь к бандлу приложения
SERVER_BUNDLE="./Binaries/Mac/TaleStories.app"

# 2. Путь к самому бинарнику внутри бандла
# Внимание: Имя бинарника обычно совпадает с именем проекта
BINARY_NAME="TaleStories"
SERVER_EXE="$SERVER_BUNDLE/Contents/MacOS/$BINARY_NAME"

echo "🔍 Checking server application..."

# Проверка существования бандла (.app)
if [ ! -d "$SERVER_BUNDLE" ]; then
    echo "❌ Error: Server bundle not found at $SERVER_BUNDLE"
    exit 1
fi

# Проверка существования исполняемого файла
if [ ! -x "$SERVER_EXE" ]; then
    echo "❌ Error: Executable not found or not executable at $SERVER_EXE"
    # Попробуем найти бинарник, если имя отличается (например, TaleStoriesServer)
    SERVER_EXE=$(find "$SERVER_BUNDLE/Contents/MacOS" -type f -maxdepth 1 | head -n 1)
    if [ -z "$SERVER_EXE" ]; then
        exit 1
    fi
    echo "ℹ️ Found binary: $(basename "$SERVER_EXE")"
fi

echo "🚀 Starting Unreal Dedicated Server (Headless mode)..."

# Запуск бинарника напрямую (БЕЗ команды open, чтобы видеть логи в этом терминале)
# Флаги:
# -server: запуск в режиме сервера
# -log: вывод логов прямо в текущую консоль
# -nullrhi: отключение графики (GPU)
# -nosound: отключение аудио-движка (важно для серверов без звуковых карт)
"$SERVER_EXE" -log -nullrhi -nosound

# 3. Запускаем Kotlin + Kafka
#echo "🐳 Starting Backend (Kotlin)..."
#docker-compose down || true
#docker-compose up --build