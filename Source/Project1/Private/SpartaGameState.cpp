

#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "SpartaPlayerController.h"
#include "SpartaCharacter.h"
#include "Kismet/GamePlayStatics.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Blueprint/UserWidget.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "SmallCoinItem.h"
#include "MineItem.h"
#include "RollingRock.h"

/***********************************************************/
/**** 과제 내용 구현을 위해 추가한 함수는 맨 마지막에 있습니다 ****/
/***********************************************************/
/* "지뢰 n회 발동 시 게임오버" - 강의자료 숙제 내용이나, UI와 연계해보고자 유지하였습니다. */

ASpartaGameState::ASpartaGameState()
{
	// 점수 및 게임오버 플래그 초기화
	Score = 0;
	bIsGameOver = false;

	// 스폰 관련 변수 초기화
	ItemToSpawn = 40;
	AdditionalItemsToSpawn = 15;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	MinCoinCount = 10;
	TrapToSpawn = 5;

	// 레벨 관련 변수 초기화
	CurrentLevelIndex = 0;
	MaxLevels = 3;

	// 웨이브 관련 변수 초기화
	CurrentWaveIndex = 0;
	MaxWaves = 3;
	WaveDuration = 20.f;

	// 지뢰 발동 횟수 관련 변수 초기화
	MineOverlapCount = 0;
	MaxMineOverlaps = 10;
	MineDisadvantage = -100;
}

void ASpartaGameState::BeginPlay()
{
	Super::BeginPlay();

	StartLevel();

	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ASpartaGameState::UpdateHUD,
		0.1f,
		true
	);
}

// 현재 레벨 점수 반환 (누적 점수 아님)
int32 ASpartaGameState::GetScore() const
{
	return Score;
}

// 현재 점수 추가 (누적 점수에도 추가)
void ASpartaGameState::AddScore(int32 Amount)
{
	Score += Amount;
	if (Score < 0) Score = 0;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			SpartaGameInstance->AddToScore(Amount);
		}
	}
}

// 새로운 레벨 시작 및 초기화
void ASpartaGameState::StartLevel()
{
	// HUD 표시
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			SpartaPlayerController->ShowGameHUD();
		}
	}

	// 현재 레벨 인덱스 업데이트
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			CurrentLevelIndex = SpartaGameInstance->CurrentLevelIndex;
		}
	}

	// 웨이브 시작
	StartWave();
}

// 현재 스폰된 코인을 모두 콜렉트 한 경우 웨이브 종료
void ASpartaGameState::OnCoinCollected()
{
	CollectedCoinCount++;
	UE_LOG(LogTemp, Warning, TEXT("Coin Collected: %d / %d"), CollectedCoinCount, SpawnedCoinCount)

	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
	{
		EndWave();
	}
}

// 밟은 지뢰 개수가 특정 숫자에 도달하면 점수 감소
void ASpartaGameState::OnMineOverlap()
{
	MineOverlapCount++;

	if (MineOverlapCount >= MaxMineOverlaps)
	{
		AddScore(MineDisadvantage);
		MineOverlapCount = 0;
	}
}

void ASpartaGameState::EndLevel()
{	
	// 현재 레벨 인덱스 증가 및 게임 인스턴스와 동기화
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
		if (SpartaGameInstance)
		{
			CurrentLevelIndex++;
			SpartaGameInstance->CurrentLevelIndex = CurrentLevelIndex;

			// 최고 기록 업데이트
			SpartaGameInstance->UpdateAndGetRecord(SpartaGameInstance->TotalScore);
		}
	}

	// 마지막 레벨이 종료된 경우 게임오버 호출
	if (CurrentLevelIndex >= MaxLevels)
	{
		OnGameOver();
		return;
	}

	// 새로운 레벨 시작
	if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
	{
		UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
	}
	else
	{
		OnGameOver();
	}
}

void ASpartaGameState::OnGameOver()
{
	bIsGameOver = true;

	// 플레이어 컨트롤러 pause 및 메인 메뉴 표시
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			if (ACharacter* Character = SpartaPlayerController->GetCharacter())
			{
				SpartaPlayerController->SetPause(true);
			}
			SpartaPlayerController->ShowMainMenu("The End", "Restart", bIsGameOver, !bIsGameOver);
		}
	}
}

// HUD 구성요소
// 1. 누적 점수 (텍스트)
// 2. 콜렉트한 코인 개수 / 레벨 내 코인 개수 (텍스트)
// 3. 현재 레벨 & 웨이브 (텍스트)
// 4. 현재 웨이브 남은 시간 (텍스트 / Progress Bar)
// 5. 디버프 이름 & 남은 시간 (텍스트)
void ASpartaGameState::UpdateHUD()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
		{
			if (UUserWidget* HUDWidget = SpartaPlayerController->GetHUDWidget())
			{
				// 1. 누적 점수
				if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
				{
					if (UGameInstance* GameInstance = GetGameInstance())
					{
						USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
						if (SpartaGameInstance)
						{
							ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), SpartaGameInstance->TotalScore)));
						}
					}
				}

				// 2. 콜렉트한 코인 개수 / 레벨 내 코인 개수
				if (UTextBlock* CoinsText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Coins"))))
				{
					CoinsText->SetText(FText::FromString(FString::Printf(TEXT("Coins: %d / %d"), CollectedCoinCount, SpawnedCoinCount)));
				}

				// 3. 현재 레벨 & 웨이브
				if (UTextBlock* LevelText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
				{
					LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level %d"), CurrentLevelIndex + 1)));
				}

				if (UTextBlock* WaveText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Wave"))))
				{
					WaveText->SetText(FText::FromString(FString::Printf(TEXT("Wave %d"), CurrentWaveIndex + 1)));
				}

				// 4. 현재 웨이브 남은 시간 (n초 / Progress Bar)
				float WaveRemainingTime = GetWorldTimerManager().GetTimerRemaining(WaveTimerHandle);
				
				if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
				{
					TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), WaveRemainingTime)));
				}

				if (UProgressBar* TimeBar = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(TEXT("TimeBar"))))
				{
					float Fraction = WaveDuration > 0 ? WaveRemainingTime / WaveDuration : 0.f;
					TimeBar->SetPercent(Fraction);
					TimeBar->SetFillColorAndOpacity(Fraction > 0.3 ? FLinearColor::Green : FLinearColor::Red);
				}
				
				// 5. 상태 이름 & 남은 시간
				if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(PlayerController->GetCharacter()))
				{
					uint8 CharacterStatus = PlayerCharacter->CharacterStatus;

					if (UVerticalBox* StatusBox = Cast<UVerticalBox>(HUDWidget->GetWidgetFromName(TEXT("StatusBox"))))
					{
						// 각 상태별 ECharacterStatus 에 정의된 값과 Bitwise & 연산하여 체크
						for (uint8 ECharacterStatusValue = 1; ECharacterStatusValue <= PlayerCharacter->ECharacterStatusMax; ECharacterStatusValue *= 2)
						{
							FName StatusName = PlayerCharacter->GetStatusName(ECharacterStatusValue);
							if ((CharacterStatus & ECharacterStatusValue) == ECharacterStatusValue)
							{
								// StatusTextMap 에서 현재 추가된 상태 관련 텍스트를 <FName, UTextBlock*> 타입으로 관리
								UTextBlock* StatusText;
								if (UTextBlock** StatusTextPtr = StatusTextMap.Find(StatusName))
								{
									StatusText = *StatusTextPtr;
								}
								// 맵에 해당 상태의 UTextBlock이 없는 경우 생성 및 맵과 부모 위젯인 StatusBox(VerticalBox) 에 추가
								else
								{
									StatusText = NewObject<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("StatusBox")));
									StatusBox->AddChildToVerticalBox(StatusText);
									StatusTextMap.Add(TTuple<FName, UTextBlock*>(StatusName, StatusText));
								}
								// 상태 텍스트를 디버프 이름과 남은 시간으로 설정
								if (FTimerHandle* StatusTimerHandlePtr = PlayerCharacter->GetStatusTimerHandle(ECharacterStatusValue))
								{
									FTimerHandle StatusTimerHandle = *StatusTimerHandlePtr;
									if (IsValid(StatusText))
									{
										float StatusRemainingTime = GetWorldTimerManager().GetTimerRemaining(StatusTimerHandle);
										StatusText->SetText(FText::FromString(
											StatusName.ToString() + FString::Printf(TEXT(" (%.0f s)"), StatusRemainingTime)));
									}
								}
							}
							else
							{
								// 상태 해제 시 해당 텍스트 삭제 및 StatusTextMap 에서 삭제
								if (StatusTextMap.Find(StatusName))
								{
									UTextBlock* StatusText;
									if (StatusTextMap.RemoveAndCopyValue(StatusName, StatusText))
									{
										StatusText->RemoveFromParent();
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void ASpartaGameState::StartWave()
{	
	/******** 아이템 스폰 *******/
	// 스폰할 아이템 개수 (첫 웨이브가 아닌 경우 추가 수량만큼만 스폰 (이미 레벨에 아이템이 있기 때문에))
	if (CurrentWaveIndex > 0)
	{
		ItemToSpawn = AdditionalItemsToSpawn;
	}

	// 월드 내 SpawnVolume 을 찾아 랜덤 아이템 스폰 함수 호출
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);
	ASpawnVolume* SpawnVolume = nullptr;
	if (FoundVolumes.IsValidIndex(0))
	{
		SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
	}

	if (SpawnVolume)
	{
		// 이번 웨이브에 스폰된 코인 개수
		int32 WaveSpawnedCoinCount = 0;

		for (int32 i = 0; i < ItemToSpawn; i++)
		{
			if (AActor* SpawnedItem = SpawnVolume->SpawnRandomItem())
			{
				if (SpawnedItem->IsA(ACoinItem::StaticClass()))
				{
					SpawnedCoinCount++;
					WaveSpawnedCoinCount++;
				}
			}
		}

		// 이번 웨이브에 스폰된 코인 개수가 최소 코인 개수보다 작은 경우 부족한 개수만큼 SmallCoin 스폰
		if (WaveSpawnedCoinCount < MinCoinCount)
		{
			for (int32 i = 0; i < MinCoinCount - WaveSpawnedCoinCount; i++)
			{
				if (SpawnVolume->SpawnItem(ASmallCoinItem::StaticClass()))
				{
					SpawnedCoinCount++;
				}
			}
		}

		/******** 웨이브 별 트랩 스폰 *******/
		/******** TODO: 장애물 추가되면 웨이브 인덱스에 따라 다른 트랩 클래스의 액터 스폰 ********/
	}

	// 현재 웨이브 타이머 시작
	GetWorldTimerManager().SetTimer(
		WaveTimerHandle,
		this,
		&ASpartaGameState::OnWaveTimeUp,
		WaveDuration,
		false
	);

	// 웨이브 시작 시 로그 출력
	UE_LOG(LogTemp, Warning, TEXT("Level %d - Wave %d start!"),
		CurrentLevelIndex + 1, CurrentWaveIndex + 1, SpawnedCoinCount);
}

void ASpartaGameState::OnWaveTimeUp()
{
	// 웨이브 타이머 종료 시 EndWave 호출
	EndWave();
}

void ASpartaGameState::EndWave()
{
	// 웨이브 타이머 clear
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);

	// 웨이브 인덱스 증가
	CurrentWaveIndex++;

	// 마지막 웨이브 종료된 경우 레벨 종료 호출
	if (CurrentWaveIndex >= MaxWaves)
	{
		EndLevel();
		return;
	}

	// 새로 웨이브 시작
	StartWave();
}

void ASpartaGameState::SetAllTimersPaused(bool bShouldPause)
{
	if (GetWorld())
	{
		// 캐릭터 상태 타이머 일시정지 / 재개
		if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		{
			if (ASpartaCharacter* SpartaCharacter = Cast<ASpartaCharacter>(PlayerController->GetCharacter()))
			{
				for (FTimerHandle* TimerHandle : SpartaCharacter->TimerHandleArray)
				{
					UE_LOG(LogTemp, Warning, TEXT("Pausing Status"))
					if (TimerHandle)
					{
						if (bShouldPause) GetWorldTimerManager().PauseTimer(*TimerHandle);
						else GetWorldTimerManager().UnPauseTimer(*TimerHandle);
					}
				}
			}
		}

		// 레벨 내 아이템 파티클 타이머 일시정지 / 재개
		TArray<AActor*> ItemsInWorld;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseItem::StaticClass(), ItemsInWorld);
		for (int32 i = 0; i < ItemsInWorld.Num(); i++)
		{
			if (ABaseItem* Item = Cast<ABaseItem>(ItemsInWorld[i]))
			{
				if (bShouldPause) GetWorldTimerManager().PauseTimer(Item->DestroyParticleTimerHandle);
				else GetWorldTimerManager().UnPauseTimer(Item->DestroyParticleTimerHandle);
				
				if (AMineItem* MineItem = Cast<AMineItem>(Item))
				{
					if (bShouldPause)
					{
						GetWorldTimerManager().PauseTimer(MineItem->DestroyExplosionParticleTimerHandle);
						GetWorldTimerManager().PauseTimer(MineItem->ExplosionTimerHandle);
					}
					else
					{
						GetWorldTimerManager().UnPauseTimer(MineItem->DestroyExplosionParticleTimerHandle);
						GetWorldTimerManager().UnPauseTimer(MineItem->ExplosionTimerHandle);
					}
				}
			}
		}

		// 현재 웨이브 타이머 일시정지 / 재개
		if (bShouldPause)
		{
			GetWorldTimerManager().PauseTimer(HUDUpdateTimerHandle);
			GetWorldTimerManager().PauseTimer(WaveTimerHandle);
		}
		else
		{
			GetWorldTimerManager().UnPauseTimer(HUDUpdateTimerHandle);
			GetWorldTimerManager().UnPauseTimer(WaveTimerHandle);
		}
	}
}