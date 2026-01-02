#include "TaleStoriesGameMode.h"
#include "TaleStoriesBPLibrary.h" // Подключаем твою библиотеку

void ATaleStoriesGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Проверяем: если мы запускаемся как сервер (даже внутри Эдитора)
	UE_LOG(LogTemp, Log, TEXT("--- SERVER STARTING ---"));
	UTaleStoriesBPLibrary::StartDedicatedServer(9000);
}
