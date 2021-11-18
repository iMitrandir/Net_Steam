// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePlatformsGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "World/PlatformTrigger.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "MenuSystem/MainMenu.h"
#include "MenuSystem/ExitMenu.h"
#include "OnlineSubsystem.h"


#include "Kismet/GameplayStatics.h"


static const FName SESSION_KEY = FName("TestSession");

UPuzzlePlatformsGameInstance::UPuzzlePlatformsGameInstance()
{
	// using BP in c++ - set menu class class to our Blueprinted character. There is na example in Default Game Mode cal
	ConstructorHelpers::FClassFinder<UMainMenu> MenuBPClass(TEXT("/Game/MenuSystem/WB_MainMenu"));
	if (MenuBPClass.Class != NULL)
	{
		MenuClass = MenuBPClass.Class;
	}

	ConstructorHelpers::FClassFinder<UExitMenu> QuitMenuBPClass(TEXT("/Game/MenuSystem/WB_QuitMenu"));
	if (QuitMenuBPClass.Class != NULL)
	{
		QuitMenuClass = QuitMenuBPClass.Class;
	}

	SESSION_NAME = TEXT("GameSession");
}

void UPuzzlePlatformsGameInstance::Init()
{
	UE_LOG(LogTemp, Warning, TEXT("On init %s"), *MenuClass->GetName());

	OnlineSubsystemPtr = IOnlineSubsystem::Get(); //singleton

	//получаем указатель на текущию онлайн сабситему проекта
	if(OnlineSubsystemPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("On init found subsystem: %s"), *OnlineSubsystemPtr->GetSubsystemName().ToString());

		//получаем указатель на интерфейс текущей сессии чтобы испольховать ее функции
		 SessionInterface = OnlineSubsystemPtr->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			//bind UObject to initiate call of OnCreateSessionComplete  when session is ceated successfully.Так как создание сессии может занимать какое-от время, в процессе которого игра дожна продолжать работать а не фризит, мы подписываемся на делегат который запустит ф-ю тогда когда мы получим коллбек с сервера о том, что сессия создана
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnCreateSessionComplete);

			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnDestroySessionComplete);

			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnFindSessionComplete);
		};
		
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnJoinSessionComplete);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("On init no subsystem found"));  
	}
}                                                          

void UPuzzlePlatformsGameInstance::LoadMenu()
{
	//double check that the BP exists
	if(!ensure(MenuClass!=nullptr)){return;}
	
	Menu = CreateWidget<UMainMenu>(this, MenuClass);

	if(!ensure(Menu!=nullptr)){return;}
	Menu->AddToViewport();

	Menu->Setup();

	//сохраняет в меню указатель на этот конкрентынй гейм инстанс, потому что ГМ знает о существовании Меню, а мены не занет о существовании ПазлПлатформГМ но знает про совой интерфейс
	Menu->SetMenuInterface(this);

}

void UPuzzlePlatformsGameInstance::QuitMenuCall()
{
	//double check that the BP exists
	if(!ensure(QuitMenuClass!=nullptr)){return;}

	
	QuitMenu = CreateWidget<UExitMenu>(this, QuitMenuClass);

	if(!ensure(QuitMenu!=nullptr)){return;}
	QuitMenu->AddToViewport();

	QuitMenu->Setup();

	//сохраняет в меню указатель на этот конкрентынй гейм инстанс, потому что ГМ знает о существовании Меню, а мены не занет о существовании ПазлПлатформГМ но знает про совой интерфейс
	QuitMenu->SetMenuInterface(this);           
}

void UPuzzlePlatformsGameInstance::Host(FString Name)
{
	SESSION_NAME = FName(Name);
	if(SessionInterface.IsValid())
	{                           
		auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if(ExistingSession!=nullptr)
		{
			//вызывает каллбек и выполняет SessionDestroy() после удаления текущей сессии.
			SessionInterface->DestroySession(SESSION_NAME);
		}
		else                             
		{
			//создаем сессию при нажантии на кнопку хост
			CreateSession();
		}
	}	
}

void UPuzzlePlatformsGameInstance::JoinIP(const FString& IPAddr)
{
	UE_LOG(LogTemp, Warning, TEXT("NOT IMPLEMENTED"));
}

void UPuzzlePlatformsGameInstance::Join(int32 Index)
{

	if(!SessionInterface.IsValid()) return;
	if(!SessionSearch.IsValid()) return;
	
	/*if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Join to %s address"), *Address));
	}*/

	//так как это консольная команда и может быть вызвана из командной стороки то Menu не обязательно должен присутствоватб
	if(Menu != nullptr)
	{
		
		Menu->TierDown();
		
	}
	// в конце запустит OnJoinSessionCompleteDelegates
	SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
	 
	// APlayerController* PlayerController = GetFirstLocalPlayerController(GetWorld());
	// if(!ensure(PlayerController!=nullptr)){return;}
	//
	// PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);

	//UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UPuzzlePlatformsGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if(!SessionInterface.IsValid()) return;
	
	FString ConnectInfo;
	if(SessionInterface->GetResolvedConnectString(SessionName, ConnectInfo))
	{
		//Join session
		APlayerController* PlayerController = GetFirstLocalPlayerController(GetWorld());
		if(!ensure(PlayerController!=nullptr)){return;}
	
		PlayerController->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldnot connect"))  ;
	}
}

void UPuzzlePlatformsGameInstance::StartSession()
{
	if(SessionInterface.IsValid())
	{
		SessionInterface->StartSession(SESSION_NAME);
	}
}

void UPuzzlePlatformsGameInstance::QuitGame()
{
	 //UGameplayStatics::OpenLevel(GetWorld(), "/Game/Static/Maps/MenuLevel");
	
	APlayerController* PlayerController = GetFirstLocalPlayerController(GetWorld());
	if(!ensure(PlayerController!=nullptr)){return;}

	PlayerController->ClientTravel("/Game/Static/Maps/MenuLevel", ETravelType::TRAVEL_Absolute);
}

void UPuzzlePlatformsGameInstance::ExitGame()
{
	APlayerController* PlayerController = GetFirstLocalPlayerController(GetWorld());
	if(!ensure(PlayerController!=nullptr)){return;}

	PlayerController->ConsoleCommand("Exit");
}

void UPuzzlePlatformsGameInstance::CreateSession()
{
	//так как процессы создания сессии ассинхронный, нельзя быть уверенным,что интерфейс еще существует
	if(SessionInterface.IsValid())
	{		    
		FOnlineSessionSettings 	SessionSettings;

		//если стим не используется на пример при запуске с -nostea то будет использовать NULL subsystem
		if(IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
		{
			SessionSettings.bIsLANMatch = true;
		}
		else
		{
			SessionSettings.bIsLANMatch = false;
         
		}
		SessionSettings.NumPublicConnections = 5;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true; // включение Presence для ссервера
		//SessionSettings.BuildUniqueId = 0x00c89141;
		SessionSettings.bAllowJoinViaPresence = true;

		//кастомные настройки
		SessionSettings.Set(SESSION_KEY, SESSION_NAME.ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		
		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);    
	}
}

void UPuzzlePlatformsGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccess) 
{
	if(!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session already creaed, cant create another one"));	
		return;
	}
	
	UWorld* World = GetWorld();
	
	if(!ensure(World!=nullptr)){return;}

	//так как это консольная команда и может быть вызвана из командной стороки то Menu не обязательно должен присутствоватб
	if(Menu != nullptr)
	{
		Menu->TierDown();
	}

	//on call of thos function - specify the server as a listener server, makes the client which is calling this function as a server 
	World->ServerTravel("/Game/Static/Maps/Lobby?listen");      
}

void UPuzzlePlatformsGameInstance::OnDestroySessionComplete(FName SessionName, bool bSuccess)
{
	if(bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Create new session ufter destroy"));

		  CreateSession();
	}   
}

void UPuzzlePlatformsGameInstance::RefreshSession()
{
	RefreshServerList();
}

void UPuzzlePlatformsGameInstance::RefreshServerList()
{
	///ищем созданные сессии
	//1й вариант конструирования шаред рефа
	SessionSearch = MakeShareable(new  FOnlineSessionSearch);

	//2й вариант конструирования шаред рефа
	//TSharedRef<FOnlineSessionSearch> OnlineSearchSettings = MakeShared<FOnlineSessionSearch>();

			
	if(SessionSearch.IsValid())
	{


		//что мы ищем - устанавливаем настройки поиска Presence
		//SessionSearch->bIsLanQuery = true; // LAN query

		SessionSearch->MaxSearchResults = 500; // в туториале ставят 100 и выдает всех у кого есть лодди, но это не решает проблему. Решает проблему установка флага SessionSettings.bAllowJoinViaPresence = true 

		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE,true, EOnlineComparisonOp::Equals); //Presence query

		TSharedRef<FOnlineSessionSearch> OnlineSearchSettings = SessionSearch.ToSharedRef();
		
		UE_LOG(LogTemp, Warning, TEXT("Find Sessions Process ......"));				
				
		SessionInterface->FindSessions(0,OnlineSearchSettings);
	}	
}

void UPuzzlePlatformsGameInstance::OnFindSessionComplete(bool bSuccess)
{
	
	if(bSuccess && SessionSearch.IsValid() && Menu != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Session found"));

		auto SessionsArray =  SessionSearch->SearchResults;		   

		TArray<FServerData> SessionDatas;		
		/*SessionNames.Add("Test Session 1");     
		SessionNames.Add("Test Session 2");
		SessionNames.Add("Test Session 3");  */ 
		for(const auto& SessionInstance : SessionsArray)
		{

			UE_LOG(LogTemp, Warning, TEXT("Session name: %s"), *SessionInstance.GetSessionIdStr());
													
			FServerData SessionInstanceData;
			FString OutSessionCustomName;
			
			SessionInstance.Session.SessionSettings.Get(SESSION_KEY,OutSessionCustomName);      
			SessionInstanceData.Name = OutSessionCustomName; // SessionInstance.GetSessionIdStr();
			SessionInstanceData.CurrentPlayer = SessionInstance.Session.NumOpenPublicConnections;
			SessionInstanceData.MaxPlayers = SessionInstance.Session.SessionSettings.NumPublicConnections;
			SessionInstanceData.HostUsername = SessionInstance.Session.OwningUserName;

			//передаю структуру с установленными параметрами
			SessionDatas.Add(SessionInstanceData);  
		}
		Menu->SetServerList(SessionDatas);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Session NOT found"));
  
	}
}

