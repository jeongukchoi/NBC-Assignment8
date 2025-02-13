
#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "SlowItem.generated.h"


UCLASS()
class PROJECT1_API ASlowItem : public ABaseItem
{
	GENERATED_BODY()
	
public:
	ASlowItem();

	virtual void ActivateItem(AActor* Activator) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float SlowMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float SlowDuration;
};
