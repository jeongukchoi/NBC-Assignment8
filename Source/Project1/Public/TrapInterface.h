
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TrapInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTrapInterface : public UInterface
{
	GENERATED_BODY()
};


class PROJECT1_API ITrapInterface
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual void OnTrapOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) = 0;

	UFUNCTION()
	virtual void OnTrapEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex) = 0;

	virtual void ActivateTrap(AActor* Activator) = 0;
	virtual FName GetTrapType() const = 0;
};
