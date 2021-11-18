// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TestGame01GameMode.h"


#include "LobbyGameMode.generated.h"



class APlayerController;
/**
 * 
 */
UCLASS()
class TESTGAME01_API ALobbyGameMode : public ATestGame01GameMode
{
	GENERATED_BODY()

public:
 	void PostLogin(APlayerController* NewPlayer) override;

	void Logout(AController* Exiting) override;

private:
	int32 count = 0;

	FTimerHandle GameStartTimer;

	void StartGame();
	
};
