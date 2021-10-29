// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameMapsSettings.h"   
#include <string>

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	
	
	Super::PostLogin(NewPlayer);

	if(NewPlayer != nullptr)
	{
		count++;
		
	}

	if(count >=3)
	{
		UWorld* World = GetWorld();
		

		bUseSeamlessTravel = true; // чтобы не было фриза при переносе плееров в новы лвл используется концепция seamless trevel
		
		World->ServerTravel("/Game/Static/Maps/ThirdPersonExampleMap?listen");


	}
	
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	count--;
}
