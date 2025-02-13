
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SpartaGameState.generated.h"


UCLASS()
class PROJECT1_API ASpartaGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	ASpartaGameState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 Score;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Status")
	bool bIsGameOver;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 SpawnedCoinCount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 CollectedCoinCount;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 MinCoinCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Level")
	int32 CurrentLevelIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 MaxLevels;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TArray<FName> LevelMapNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mine")
	int32 MineOverlapCount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mine")
	int32 MaxMineOverlaps;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mine")
	int32 MineDisadvantage;

	/***************** 웨이브 관련 변수 *****************/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 CurrentWaveIndex;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 MaxWaves;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	uint8 AdditionalItemsToSpawn;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float WaveDuration;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 ItemToSpawn;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 TrapToSpawn;

	FTimerHandle WaveTimerHandle;
	/*************************************************/

	/***************** HUD 관련 변수 *****************/
	TMap<FName, class UTextBlock*> StatusTextMap;
	FTimerHandle HUDUpdateTimerHandle;

	UFUNCTION(BlueprintPure, Category = "score")
	int32 GetScore() const;

	UFUNCTION(BlueprintCallable, Category = "score")
	void AddScore(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "score")
	void OnGameOver();
	
	void StartLevel();
	void EndLevel();

	/***************** 웨이브 관련 함수 *****************/
	void StartWave();
	void OnWaveTimeUp();
	void OnCoinCollected();
	void OnMineOverlap();
	void EndWave();
	/*************************************************/

	void UpdateHUD();
	void SetAllTimersPaused(bool bShouldPause);

	//void OnScoreTimerEnd();

protected:
	virtual void BeginPlay() override;

};
