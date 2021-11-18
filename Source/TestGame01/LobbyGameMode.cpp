// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameMapsSettings.h"   
#include <string>
#include "PuzzlePlatformsGameInstance.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	
	
	Super::PostLogin(NewPlayer);

	if(NewPlayer != nullptr)
	{
		count++;
		
	}

	if(count >=2)
	{
		//таймер срабатывает и закидывает игроков в лобби в саму игру
		    GetWorldTimerManager().SetTimer(GameStartTimer, this, &ALobbyGameMode::StartGame, 5.0f);

	}
	
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	count--;
}

void ALobbyGameMode::StartGame()
{
	UWorld* World = GetWorld();

	//после старта сесси(игры) этим кодом, игроки не смогут подключиться к карте. Если его закоментировать, то будет просто телепорт в левел, но без старта сессии и игроки смогут заходить в уровень, и игра начнется еогда будет набрано заданное количество.
	auto GameInstanse = Cast<UPuzzlePlatformsGameInstance>(GetGameInstance());
	if(GameInstanse==nullptr) return;
	GameInstanse->StartSession();

	bUseSeamlessTravel = true; // чтобы не было фриза при переносе плееров в новы лвл используется концепция seamless trevel
		
	World->ServerTravel("/Game/Static/Maps/ThirdPersonExampleMap?listen");
       
}
