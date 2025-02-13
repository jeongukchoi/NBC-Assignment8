#include "SpartaCharacter.h"
#include "SpartaPlayerController.h"
#include "SpartaGameState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"

ASpartaCharacter::ASpartaCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 350.f;
	SpringArmComp->SocketOffset = FVector(20.f, 100.f, 0.f);
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetMesh());
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);

	NormalSpeed = 600.f;
	SprintSpeedMultiplier = 1.7f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;

	MouseSensitivity = 1.f;

	MaxHealth = 100.f;
	Health = MaxHealth;
	MaxStamina = 100.f;
	Stamina = MaxStamina;
	SprintStaminaCost = 0.5f;
	RestoreStaminaAmount = 1.f;
	RestoreStaminaInterval = 0.2f;
	ExhaustedDuration = 10.f;

	InitialSpeed = NormalSpeed;
	InitialFriction = GetCharacterMovement() ? GetCharacterMovement()->GroundFriction : 0.8f;
	
	CharacterStatus = 0;
	ReverseMultiplier = 1.f;

	TimerHandleArray.Add(&ReverseTimerHandle);
	TimerHandleArray.Add(&SlipperyTimerHandle);
	TimerHandleArray.Add(&SlowTimerHandle);
	TimerHandleArray.Add(&ExhaustedTimerHandle);
	TimerHandleArray.Add(&RestoreStaminaTimerHandle);

	bIsSprinting = false;
	bIsStaminaHidden = true; // hidden by default in WBP Designer
}

void ASpartaCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 에디터에서 변경될 수 있는 값 다시 초기화
	Health = MaxHealth;
	Stamina = MaxStamina;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;
	InitialSpeed = NormalSpeed;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
		InitialFriction = GetCharacterMovement()->GroundFriction;
	}
	
	if (GetWorld())
	{
		GetWorldTimerManager().SetTimer(
			RestoreStaminaTimerHandle,
			this,
			&ASpartaCharacter::RestoreStamina,
			RestoreStaminaInterval,
			true
		);
	}

	UpdateOverheadHP();
	UpdateOverheadStamina();
}

void ASpartaCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void ASpartaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASpartaPlayerController* PlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::Move
				);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::StartJump
				);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Completed,
					this,
					&ASpartaCharacter::StopJump
				);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::Look
				);
			}

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::StartSprint
				);
			}

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Completed,
					this,
					&ASpartaCharacter::StopSprint
				);
			}

			if (PlayerController->SwitchToUIAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SwitchToUIAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::SwitchToUI
				);
			}

			if (PlayerController->SwitchToCharacterAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SwitchToCharacterAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::SwitchToCharacter
				);
			}
		}
	}
}

/***************** IMC_Character *************/
void ASpartaCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller) return;

	const FVector2D MoveInput = Value.Get<FVector2D>();

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(GetActorForwardVector(), MoveInput.X * ReverseMultiplier);
	}

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(GetActorRightVector(), MoveInput.Y * ReverseMultiplier);
	}

	if (bIsSprinting)
	{
		Stamina = FMath::Clamp(Stamina - SprintStaminaCost, 0.f, MaxStamina);
		UpdateOverheadStamina();
		ShowOverheadStamina();
	}
}

void ASpartaCharacter::StartJump(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		Jump();
	}
}

void ASpartaCharacter::StopJump(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		StopJumping();
	}
}

void ASpartaCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookInput = Value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X * MouseSensitivity);
	AddControllerPitchInput(LookInput.Y * MouseSensitivity);
}

void ASpartaCharacter::StartSprint(const FInputActionValue& Value)
{
	if (GetWorldTimerManager().IsTimerActive(ExhaustedTimerHandle)) return;

	if (GetCharacterMovement())
	{
		if (Stamina - SprintStaminaCost > 0)
		{
			SprintSpeed = NormalSpeed * SprintSpeedMultiplier;
			GetWorldTimerManager().PauseTimer(RestoreStaminaTimerHandle);
			bIsSprinting = true;
		}
		else
		{
			SprintSpeed = NormalSpeed;
			GetWorldTimerManager().UnPauseTimer(RestoreStaminaTimerHandle);
			GetWorldTimerManager().SetTimer(
				ExhaustedTimerHandle,
				this,
				&ASpartaCharacter::ResetExhausted,
				ExhaustedDuration,
				false
			);
			CharacterStatus |= static_cast<uint8>(ECharacterStatus::Exhausted);
			bIsSprinting = false;
			// TODO: UI 메세지 표시 "탈진하여 빨리 뛸 수 없다"
		}
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void ASpartaCharacter::StopSprint(const FInputActionValue& Value)
{
	bIsSprinting = false;
	if (GetCharacterMovement())
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	GetWorldTimerManager().UnPauseTimer(RestoreStaminaTimerHandle);
}


void ASpartaCharacter::SwitchToUI(const FInputActionValue& Value)
{	
	if (ASpartaPlayerController* PlayerController = Cast<ASpartaPlayerController>(GetController()))
	{
		PlayerController->GoToMainMenu();
	}
}
/***************** IMC_Character *************/

/******************** IMC_UI *****************/
void ASpartaCharacter::SwitchToCharacter(const FInputActionValue& Value)
{
	if (ASpartaPlayerController* PlayerController = Cast<ASpartaPlayerController>(GetController()))
	{
		PlayerController->ResumeGame();
	}
}
/******************** IMC_UI *****************/

/******************** Items ******************/
float ASpartaCharacter::GetHealth() const
{
	return Health;
}

void ASpartaCharacter::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
	UpdateOverheadHP();
}

float ASpartaCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);
	UpdateOverheadHP();

	if (Health <= 0.f)
	{
		OnDeath();
	}

	return ActualDamage;
}

void ASpartaCharacter::OnDeath()
{
	ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr;
	if (SpartaGameState)
	{
		SpartaGameState->OnGameOver();
	}
}

void ASpartaCharacter::UpdateOverheadHP()
{
	if (!OverheadWidget) return;

	if (UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject())
	{
		// 현재 체력 / 최대 체력 fraction 으로 Progress Bar 업데이트
		if (UProgressBar* HPBar = Cast<UProgressBar>(OverheadWidgetInstance->GetWidgetFromName("HPBar")))
		{
			float Fraction = MaxHealth > 0 ? Health / MaxHealth : 0.f;
			HPBar->SetPercent(Fraction);
		}

		// Progress Bar 위로 표시되는 체력 수치 업데이트
		if (UTextBlock* HPText = Cast<UTextBlock>(OverheadWidgetInstance->GetWidgetFromName(TEXT("HPText"))))
		{
			HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
		}
	}
}

void ASpartaCharacter::UpdateOverheadStamina()
{
	if (!OverheadWidget) return;

	if (UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject())
	{
		// 현재 스태미나 / 최대 스태미나 fraction 으로 Progress Bar 업데이트
		if (UProgressBar* StaminaBar = Cast<UProgressBar>(OverheadWidgetInstance->GetWidgetFromName("StaminaBar")))
		{
			float Fraction = MaxStamina > 0 ? Stamina / MaxStamina : 0.f;
			StaminaBar->SetPercent(Fraction);
		}
	}
}

void ASpartaCharacter::RestoreStamina()
{
	if (Stamina < MaxStamina)
	{
		Stamina = FMath::Clamp(Stamina + RestoreStaminaAmount, 0.f, MaxStamina);
		UpdateOverheadStamina();
	}
	else
	{
		HideOverheadStamina();
	}
}

void ASpartaCharacter::ResetExhausted()
{
	CharacterStatus &= ~(static_cast<uint8>(ECharacterStatus::Exhausted));
}

void ASpartaCharacter::ShowOverheadStamina()
{
	if (!bIsStaminaHidden) return;
	if (!OverheadWidget) return;

	if (UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject())
	{
		UFunction* StaminaFadeInFunc = OverheadWidgetInstance->FindFunction(FName(TEXT("PlayStaminaFadeIn")));
		if (StaminaFadeInFunc)
		{
			OverheadWidgetInstance->ProcessEvent(StaminaFadeInFunc, nullptr);
		}
	}
	bIsStaminaHidden = false;
}

void ASpartaCharacter::HideOverheadStamina()
{
	if (bIsStaminaHidden) return;
	if (!OverheadWidget) return;

	if (UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject())
	{
		UFunction* StaminaFadeOutFunc = OverheadWidgetInstance->FindFunction(FName(TEXT("PlayStaminaFadeOut")));
		if (StaminaFadeOutFunc)
		{
			OverheadWidgetInstance->ProcessEvent(StaminaFadeOutFunc, nullptr);
		}
	}

	bIsStaminaHidden = true;
}

void ASpartaCharacter::ApplySlow(float SlowMultiplier, float SlowDuration)
{
	// 상태 업데이트
	CharacterStatus |= static_cast<uint8>(ECharacterStatus::Slow);

	// 속도 변경
	NormalSpeed *= SlowMultiplier;
	if (GetCharacterMovement())
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

	GetWorldTimerManager().SetTimer(
		SlowTimerHandle,
		this,
		&ASpartaCharacter::DeactivateSlow,
		SlowDuration,
		false
	);
}

void ASpartaCharacter::DeactivateSlow()
{
	// 상태 업데이트
	CharacterStatus &= ~(static_cast<uint8>(ECharacterStatus::Slow));

	// 속도 원상복구
	NormalSpeed = InitialSpeed;
	if (GetCharacterMovement())
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void ASpartaCharacter::ApplyReverse(float ReverseDuration)
{
	CharacterStatus |= static_cast<uint8>(ECharacterStatus::Reverse);

	ReverseMultiplier = -1.f;
	GetWorldTimerManager().SetTimer(
		ReverseTimerHandle,
		this,
		&ASpartaCharacter::DeactivateReverse,
		ReverseDuration,
		false
	);
}

void ASpartaCharacter::DeactivateReverse()
{
	CharacterStatus &= ~(static_cast<uint8>(ECharacterStatus::Reverse));

	ReverseMultiplier = 1.f;
}

void ASpartaCharacter::ApplySlippery(float SlipperyFriction, float SlipperyDuration)
{
	CharacterStatus |= static_cast<uint8>(ECharacterStatus::Slippery);

	if (GetCharacterMovement())
		GetCharacterMovement()->GroundFriction = SlipperyFriction;

	GetWorldTimerManager().SetTimer(
		SlipperyTimerHandle,
		this,
		&ASpartaCharacter::DeactivateSlippery,
		SlipperyDuration,
		false
	);
}

void ASpartaCharacter::DeactivateSlippery()
{
	CharacterStatus &= ~(static_cast<uint8>(ECharacterStatus::Slippery));

	if (GetCharacterMovement())
		GetCharacterMovement()->GroundFriction = InitialFriction;
}

FName ASpartaCharacter::GetStatusName(uint8 ECharacterStatusValue)
{
	switch (ECharacterStatusValue)
	{
	case static_cast<uint8>(ECharacterStatus::Reverse):		return FName(TEXT("Reverse"));
	case static_cast<uint8>(ECharacterStatus::Slippery):	return FName(TEXT("Slippery"));
	case static_cast<uint8>(ECharacterStatus::Slow):		return FName(TEXT("Slow"));
	case static_cast<uint8>(ECharacterStatus::Exhausted):	return FName(TEXT("Exhausted"));
	default:												return FName(TEXT("None"));
	}
}

FTimerHandle* ASpartaCharacter::GetStatusTimerHandle(uint8 ECharacterStatusValue)
{
	switch (ECharacterStatusValue)
	{
	case static_cast<uint8>(ECharacterStatus::Reverse):		return &ReverseTimerHandle;
	case static_cast<uint8>(ECharacterStatus::Slippery):	return &SlipperyTimerHandle;
	case static_cast<uint8>(ECharacterStatus::Slow):		return &SlowTimerHandle;
	case static_cast<uint8>(ECharacterStatus::Exhausted):	return &ExhaustedTimerHandle;
	default:												return nullptr;
	}
}

