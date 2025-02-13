
#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "ReverseItem.generated.h"


UCLASS()
class PROJECT1_API AReverseItem : public ABaseItem
{
	GENERATED_BODY()
	
public:
	AReverseItem();

	virtual void ActivateItem(AActor* Activator) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float ReverseDuration;

};
