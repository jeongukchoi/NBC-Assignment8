
#pragma once

#include "CoreMinimal.h"
#include "ItemSpawnRow.h"
#include "BaseTrap.h"
#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

UCLASS()
class PROJECT1_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpawnVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
	USceneComponent* Scene;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	class UBoxComponent* SpawningBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	UDataTable* ItemDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	float TrapSpawnLocationZ;
	
	UFUNCTION(BlueprintCallable, Category="Spawning")
	AActor* SpawnRandomItem();
	FItemSpawnRow* GetRandomItem() const;
	AActor* SpawnItem(TSubclassOf<AActor> ItemClass);
	FVector GetRandomPointInVolume() const;

	//AActor* SpawnTrap(TSubclassOf<ABaseTrap> TrapClass);
};
