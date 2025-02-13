
#include "BaseTrap.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABaseTrap::ABaseTrap()
{
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetCollisionProfileName(TEXT("BlockAll"));
	Collision->SetupAttachment(Scene);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(Collision);

	// 충돌 이벤트 바인딩
	Collision->OnComponentBeginOverlap.AddDynamic(this, &ABaseTrap::OnTrapOverlap);
	Collision->OnComponentEndOverlap.AddDynamic(this, &ABaseTrap::OnTrapEndOverlap);

	OverlapDelay = 1.f;
}

void ABaseTrap::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseTrap::OnTrapOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetWorldTimerManager().IsTimerActive(OverlapTimerHandle))
	{
		if (OtherActor && OtherActor->ActorHasTag("Player"))
		{
			ActivateTrap(OtherActor);

			GetWorldTimerManager().SetTimer(
				OverlapTimerHandle,
				OverlapDelay,
				false
			);
		}
	}
}

void ABaseTrap::OnTrapEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void ABaseTrap::ActivateTrap(AActor* Activator)
{
	if (TrapSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			TrapSound,
			GetActorLocation()
		);
	}
}

FName ABaseTrap::GetTrapType() const
{
	return TrapType;
}



