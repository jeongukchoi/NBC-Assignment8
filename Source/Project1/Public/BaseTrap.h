// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrapInterface.h"
#include "BaseTrap.generated.h"

UCLASS()
class PROJECT1_API ABaseTrap : public AActor, public ITrapInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseTrap();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	FName TrapType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap|Component")
	USceneComponent* Scene;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap|Component")
	class USphereComponent* Collision;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap|Component")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Effects")
	USoundBase* TrapSound;

	float OverlapDelay;
	FTimerHandle OverlapTimerHandle;

	virtual void BeginPlay() override;

	virtual void OnTrapOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

	virtual void OnTrapEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex) override;

	virtual void ActivateTrap(AActor* Activator);
	virtual FName GetTrapType() const;

};
