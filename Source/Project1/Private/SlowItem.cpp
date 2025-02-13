
#include "SlowItem.h"
#include "SpartaCharacter.h"


ASlowItem::ASlowItem()
{
	SlowMultiplier = 0.5f;
	SlowDuration = 7.f;
	ItemType = "Slow";
}

void ASlowItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);

	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(Activator))
		{
			PlayerCharacter->ApplySlow(SlowMultiplier, SlowDuration);
		}

		DestroyItem();
	}
}
