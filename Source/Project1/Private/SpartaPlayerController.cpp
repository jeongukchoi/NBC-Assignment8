#include "SpartaPlayerController.h"
#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"

ASpartaPlayerController::ASpartaPlayerController()
	: InputMappingContext(nullptr),
	IMC_UI(nullptr),
	MoveAction(nullptr),
	LookAction(nullptr),
	JumpAction(nullptr),
	SprintAction(nullptr),
	SwitchToUIAction(nullptr),
	SwitchToCharacterAction(nullptr),
	HUDWidgetClass(nullptr),
	HUDWidgetInstance(nullptr),
	MainMenuWidgetClass(nullptr),
	MainMenuWidgetInstance(nullptr),
	MenuFadeOutDuration(0.5f)
{
}

void ASpartaPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains("MenuLevel"))
	{
		ShowMainMenu("Sparta Game", "Start", false, false);
	}
}

UUserWidget* ASpartaPlayerController::GetHUDWidget() const
{
	return HUDWidgetInstance;
}


void ASpartaPlayerController::ShowGameHUD()
{
	// 메인 메뉴 제거
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	// HUD 표시
	if (IsValid(HUDWidgetInstance))
	{
		HUDWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		// nullptr 는 아닌데 IsValid 가 false 인 경우 nullptr assigned
		if (HUDWidgetInstance != nullptr) HUDWidgetInstance = nullptr;

		// HUD 위젯 생성 및 뷰포트에 추가
		if (HUDWidgetClass)
		{
			HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		}

		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();
		}
	}

	// 마우스 / 입력모드 변경
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	// HUD 업데이트
	ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr;
	if (SpartaGameState)
	{
		SpartaGameState->UpdateHUD();
	}
}

/**
 * 게임 상태에 따라 다른 메인 메뉴 표시 (게임 상태 : 첫 시작 / 플레이 중 / 게임 오버)
 * 
 * @param MainString 메뉴 상단에 표시할 텍스트 ("Sparta Game" or "Game Over!" 등)
 * @param ButtonString 시작/재시작 버튼에 표시할 텍스트 ("Start" or "Restart" 등)
 * @param bIsGameOver 게임 종료 상태 (애니메이션 재생 / Resume 버튼 숨김)
 * @param bIsResume 게임 플레이 중 (Resume 버튼 표시)
 */
void ASpartaPlayerController::ShowMainMenu(FString MainString, FString ButtonString, bool bIsGameOver, bool bIsResume)
{
	// 게임 오버 상태와 게임 재개 상태는 동시에 true 일 수 없으므로 return
	if (bIsGameOver && bIsResume) return;

	// HUD 숨기기
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}

	// 메인메뉴 인스턴스 제거 후 다시 생성
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	// 뷰포트에 메인 메뉴 위젯 추가
	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();

			// 마우스 표시
			bShowMouseCursor = true;
			// 게임 플레이 중 메뉴에 진입한 상태에서 IMC_UI 에 맵핑된 입력으로 메뉴 상호작용 가능하도록 인풋 모드를 GameAndUI로 설정
			SetInputMode(FInputModeGameAndUI());

			// 메뉴 페이드 인 (딜레이 없이 바로)
			UFunction* MenuFadeInFunc = MainMenuWidgetInstance->FindFunction(FName("PlayMenuFadeIn"));
			MainMenuWidgetInstance->ProcessEvent(MenuFadeInFunc, nullptr);

			// 상단 표시되는 텍스트 (MainString)
			if (UTextBlock* MainText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(FName("MainText"))))
			{
				if (MainString.IsEmpty())
				{
					MainText->SetText(FText::FromString("Sparta Game"));
				}
				else
				{
					MainText->SetText(FText::FromString(MainString));
				}
			}

			// 시작 버튼 텍스트 (ButtonString)
			if (UTextBlock* ButtonText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButtonText"))))
			{
				if (ButtonString.IsEmpty())
				{
					ButtonText->SetText(FText::FromString(TEXT("Start")));
				}
				else
				{
					ButtonText->SetText(FText::FromString(ButtonString));
				}
			}

			// 게임 첫 시작 상태가 아닌 경우 점수 표시 (초기값 Hidden)
			if (bIsGameOver || bIsResume)
			{
				if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(UGameplayStatics::GetGameInstance(this)))
				{
					int32 CurrentTotalScore = SpartaGameInstance->TotalScore;
					int32 CurrentRecord = SpartaGameInstance->UpdateAndGetRecord();

					// 현재 총점
					if (UTextBlock* TotalScoreText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("TotalScoreText"))))
					{
						TotalScoreText->SetText(FText::FromString(
							FString::Printf(TEXT("Your Score: %d"), CurrentTotalScore)));
					}

					// 현재까지 베스트 점수 (현재 점수가 베스트인 경우 추가 표시)
					if (UTextBlock* CurrentRecordText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName("CurrentRecord")))
					{
						CurrentRecordText->SetText(FText::FromString(
							FString::Printf(TEXT("Current Record: %d"), CurrentRecord)));
						if (CurrentTotalScore > CurrentRecord)
						{
							/* TODO: 추가로 어떤 걸 표시할까?
							"게임을 끝까지 플레이 해야 점수가 기록된다"
							축하, 동기 부여 필요 */
						}
					}

					// 점수 표시
					if (UFunction* DisplayScoreInfo = MainMenuWidgetInstance->FindFunction(FName("DisplayScoreInfo")))
					{
						MainMenuWidgetInstance->ProcessEvent(DisplayScoreInfo, nullptr);
					}
				}
			}

			// 게임 플레이 중인 경우 Resume 버튼 표시 (초기값은 Hidden)
			if (bIsResume)
			{
				if (UFunction* DisplayResumeButtonFunc = MainMenuWidgetInstance->FindFunction(FName("DisplayResumeButton")))
				{
					MainMenuWidgetInstance->ProcessEvent(DisplayResumeButtonFunc, nullptr);
				}
			}
			// 게임 종료 시 텍스트 애니메이션 & Resume 버튼 숨기기
			else if (bIsGameOver)
			{
				UFunction* GameOverAnimFunc = MainMenuWidgetInstance->FindFunction(FName("PlayGameOverAnim"));
				if (GameOverAnimFunc)
				{
					MainMenuWidgetInstance->ProcessEvent(GameOverAnimFunc, nullptr);
				}
			}

			// 게임 첫 시작 시
			else
			{
				SetInputMode(FInputModeUIOnly());
				/* TODO: 게임 시작 애니메이션 */
			}
		}
	}
}

// Start/Restart 버튼 클릭 시 실행
void ASpartaPlayerController::StartGame()
{
	if (USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		SpartaGameInstance->TotalScore = 0;
		SpartaGameInstance->CurrentLevelIndex = 0;
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName("BasicLevel"));
	SetPause(false);
}

// (IMC_Character) Tab 키 입력 시 호출
void ASpartaPlayerController::GoToMainMenu()
{
	if (!GetWorld()) return;

	// 게임 시작 메뉴 레벨에서는 동작 X
	FString MapName = GetWorld()->GetMapName();
	if (MapName.Contains("MenuLevel")) return;

	ASpartaGameState* SpartaGameState = GetWorld()->GetGameState<ASpartaGameState>();
	if (SpartaGameState)
	{
		// 게임 종료 상태에는 동작 X
		if (SpartaGameState->bIsGameOver) return;
	
		if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (InputMappingContext && IMC_UI)
				{
					// 1. IMC 교체
					Subsystem->RemoveMappingContext(InputMappingContext);
					Subsystem->AddMappingContext(IMC_UI, 0);

					// 2. 게임 내 타이머 일시정지
					SpartaGameState->SetAllTimersPaused(true);
					//UGameplayStatics::SetGamePaused(GetWorld(), true);
					
					// 3. 메뉴 표시
					ShowMainMenu("Sparta Game", "Restart", false, true);
				}
			}
		}
	}
}

// Resume 버튼 클릭 시 또는 (IMC_UI) Backspace 키 입력 시 호출
void ASpartaPlayerController::ResumeGame()
{
	if (!GetWorld()) return;

	ASpartaGameState* SpartaGameState = GetWorld()->GetGameState<ASpartaGameState>();
	if (SpartaGameState)
	{
		// 게임 종료 상태에는 동작 X (게임 시작 상태에서는 IMC_UI 가 적용될 수 없으므로 고려하지 않음)
		if (SpartaGameState->bIsGameOver) return;

		if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (InputMappingContext && IMC_UI)
				{
					// 1. IMC 교체
					Subsystem->RemoveMappingContext(IMC_UI);
					Subsystem->AddMappingContext(InputMappingContext, 0);

					// 2. 게임 내 타이머 재개
					SpartaGameState->SetAllTimersPaused(false);
					
					// 3. 메뉴 종료 및 HUD 표시
					ExitMenu();
				}
			}
		}
	}
}

// 메뉴 애니메이션 재생 및 HUD 표시 딜레이
void ASpartaPlayerController::ExitMenu()
{
	if (MainMenuWidgetInstance)
	{
		UFunction* MenuFadeOutFunc = MainMenuWidgetInstance->FindFunction(FName("PlayMenuFadeOut"));
		MainMenuWidgetInstance->ProcessEvent(MenuFadeOutFunc, nullptr);

		// ShowGameHUD 에서 메인 메뉴 위젯을 삭제하므로 페이드 아웃 끝날 때까지 딜레이
		GetWorldTimerManager().SetTimer(
			MenuFadeOutTimerHandle,
			[this]() {
				ShowGameHUD();
			},
			MenuFadeOutDuration,
			false
		);
	}
}

void ASpartaPlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, true);
}