// Fill out your copyright notice in the Description page of Project Settings.


#include "RollingRock.h"
#include "Math/Vector.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BaseItem.h"

ARollingRock::ARollingRock()
{
	PrimaryActorTick.bCanEverTick = true;

	MoveSpeed = 120.f;
	RotateSpeed = 120.f;
	MoveDirectionVector = GetActorRotation().Vector();
	RockDamage = 15.f;
}

void ARollingRock::BeginPlay()
{
	Super::BeginPlay();
	MoveDirectionVector = GetActorRotation().Vector();
}

void ARollingRock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult HitResult;

	AddActorWorldOffset(MoveDirectionVector * MoveSpeed * DeltaTime);
	AddActorLocalRotation(FRotator(-RotateSpeed * DeltaTime, 0, 0));
}

void ARollingRock::OnTrapOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnTrapOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	// 벽과 충돌 시 반대 방향으로 이동
	if (OtherActor)
	{
		if (OtherActor->GetName().Contains("Wall"))
		{
			// 벽의 Surface Normal 기준 반사벡터를 구하려고 했으나 실패... bFromSweep 이 true 가 되지를 않았습니다...
			//if (bFromSweep)
			//{
			//	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
			//		"bFromSweep");
			//	FVector WallSurfaceNormal = SweepResult.ImpactNormal;
			//	MoveDirectionVector =
			//		MoveDirectionVector - 2 * (FVector::DotProduct(MoveDirectionVector, WallSurfaceNormal)) * WallSurfaceNormal;
			//}

			MoveDirectionVector *= -1;
			RotateSpeed *= -1;
		}

		// 힐링 또는 코인 아이템과 충돌 시 아이템 삭제
		if (OtherActor->GetName().Contains("Healing") || OtherActor->GetName().Contains("Coin"))
		{
			if (ABaseItem* Item = Cast<ABaseItem>(OtherActor))
			{
				Item->DestroyItem();
			}
		}
	}
}