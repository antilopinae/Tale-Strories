#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Networking.h"
#include "lib_ue.hpp"
#include "TaleStoriesBPLibrary.generated.h"

// Делегат для уведомления Блупринта о результате входа
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGoogleLoginComplete, bool, bSuccess, FString, ErrorMessage);

UCLASS()
class TALESTORIES_API UTaleStoriesBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// Ссылка на долгоживущий объект библиотеки
	static TUniquePtr<GrpcReproxer> ReproxerInstance;

public:
	// 1. Инициализация и вход (использует Auth Code Flow через твой сервер)
	UFUNCTION(BlueprintCallable, Category = "TaleStories|Auth")
	static void LoginWithGoogle(FString ClientId, FString ServerAddress, FOnGoogleLoginComplete OnComplete);

	// 2. Вход в комнату (оркестрация Docker)
	UFUNCTION(BlueprintCallable, Category = "TaleStories|Lobby")
	static bool JoinGameRoom(FString RoomName, FString& OutDedicatedAddr, FString& OutErrorMessage);

	// 3. Пинг выделенного сервера (проверка связи с C++ контейнером)
	UFUNCTION(BlueprintCallable, Category = "TaleStories|Dedicated")
	static bool PingDedicatedServer(int64& OutServerTime);
	
#ifdef SERVER_MODE
	static TUniquePtr<DedicatedServerWrapper> ServerInstance;
#endif
	
public:
	UFUNCTION(BlueprintCallable, Category = "TaleStoriesServer|Dedicated")
	static void StartDedicatedServer(int32 Port);

	UFUNCTION(BlueprintCallable, Category = "TaleStoriesServer|Dedicated")
	static void StopDedicatedServer();
};
