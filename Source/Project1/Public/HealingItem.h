
#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "HealingItem.generated.h"


UCLASS()
class PROJECT1_API AHealingItem : public ABaseItem
{
	GENERATED_BODY()
	
public:
	AHealingItem();

	virtual void ActivateItem(AActor* Activator) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	int HealAmount;
};
