
#include "ReverseItem.h"
#include "SpartaCharacter.h"

AReverseItem::AReverseItem()
{
	ReverseDuration = 5.f;
	ItemType = "Reverse";
}

void AReverseItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);

	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(Activator))
		{
			PlayerCharacter->ApplyReverse(ReverseDuration);
		}

		DestroyItem();
	}
}
