

#include "SlipperyItem.h"
#include "SpartaCharacter.h"

ASlipperyItem::ASlipperyItem()
{
	SlipperyFriction = 0.005f;
	SlipperyDuration = 10.f;
	ItemType = "Slippery";
}

void ASlipperyItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);

	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(Activator))
		{
			PlayerCharacter->ApplySlippery(SlipperyFriction, SlipperyDuration);
		}

		DestroyItem();
	}
}
