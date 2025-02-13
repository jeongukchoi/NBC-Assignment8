// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTrap.h"
#include "RollingRock.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT1_API ARollingRock : public ABaseTrap
{
	GENERATED_BODY()
	
public:
	ARollingRock();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Speed")
	float MoveSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Speed")
	float RotateSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Damage")
	float RockDamage;

	FVector MoveDirectionVector;

	virtual void BeginPlay() override;

	void OnTrapOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;


public:
	virtual void Tick(float DeltaTime) override;
};
