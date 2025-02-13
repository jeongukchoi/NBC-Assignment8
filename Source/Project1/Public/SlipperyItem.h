// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "SlipperyItem.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT1_API ASlipperyItem : public ABaseItem
{
	GENERATED_BODY()

public:
	ASlipperyItem();

	virtual void ActivateItem(AActor* Activator) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties|Debuff")
	float SlipperyDuration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties|Debuff")
	float SlipperyFriction;
	
	
};
