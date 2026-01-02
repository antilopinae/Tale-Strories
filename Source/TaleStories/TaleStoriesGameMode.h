#pragma once

#include "GameFramework/GameModeBase.h"
#include "TaleStoriesGameMode.generated.h"

UCLASS()
class TALESTORIES_API ATaleStoriesGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	// Эта функция вызывается, когда уровень запускается
	virtual void BeginPlay() override;
};
