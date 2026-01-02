#!/bin/bash
set -e

# Путь к твоему Unreal Engine
UE_PATH="/Users/Shared/Epic Games/UE_5.7"
PROJECT_PATH="$(pwd)/TaleStories.uproject"

echo "🔨 Building TaleStories Game (Standalone) for Mac..."

# Сборка только игрового таргета
# Важно: имя таргета просто "TaleStories", без приставки Game
"$UE_PATH/Engine/Build/BatchFiles/Mac/Build.sh" \
    TaleStories Mac Development \
    -Project="$PROJECT_PATH" -WaitMutex

echo "✅ Game Build Complete!"

rm -r ./DedicatedServer/TaleStories.app || true
cp -r ./Binaries/Mac/TaleStories.app ./DedicatedServer/

echo "Copy App to ./DedicatedServer folder"