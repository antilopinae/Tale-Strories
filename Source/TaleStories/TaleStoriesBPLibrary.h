#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Networking.h"
#include "lib_ue.hpp"
#include "TaleStoriesBPLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGoogleLoginComplete, FString, IdToken, FString, ServerAddress);

UCLASS()
class TALESTORIES_API UTaleStoriesBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	static TUniquePtr<GrpcReproxer> ReproxerInstance;

public:
	// 1. Твой существующий метод (немного доработанный)
	UFUNCTION(BlueprintCallable, Category = "ThirdPartyLib")
	static bool ConnectAndJoin(FString Token, FString ServerAddress);

	// 2. Метод для запуска процесса логина
	UFUNCTION(BlueprintCallable, Category = "Auth")
	static void LoginWithGoogle(FString ClientId, FString ClientSecret, FString ServerAddress,
	                            FOnGoogleLoginComplete OnComplete);
};
