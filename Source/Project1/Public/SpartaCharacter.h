#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpartaCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
struct FInputActionValue;

enum class ECharacterStatus : uint8
{
	None = 0,
	Reverse = 1,
	Slippery = 2,
	Slow = 4,
	Exhausted = 8
};

UCLASS()
class PROJECT1_API ASpartaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpartaCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* OverheadWidget;

	UFUNCTION(BlueprintPure)
	float GetHealth() const;
	UFUNCTION(BlueprintCallable, Category = "Health")
	void AddHealth(float Amount);

	UFUNCTION()
	void SwitchToUI(const FInputActionValue& Value);
	UFUNCTION()
	void SwitchToCharacter(const FInputActionValue& Value);

	/*************** 상태 관련 함수 ***************/
	UFUNCTION(BlueprintCallable, Category = "Status")
	void ApplySlow(float SlowMultiplier, float SlowDuration);
	UFUNCTION(BlueprintCallable, Category = "Status")
	void ApplyReverse(float ReverseDuration);
	UFUNCTION(BlueprintCallable, Category = "Status")
	void ApplySlippery(float SlipperyFriction, float SlipperyDuration);

	FName GetStatusName(uint8 ECharacterStatusValue);
	FTimerHandle* GetStatusTimerHandle(uint8 ECharacterStatusValue);

	FTimerHandle RestoreStaminaTimerHandle;
	TArray<FTimerHandle*> TimerHandleArray;

	/*************** 상태 관련 변수 ***************/
	const uint8 ECharacterStatusMax = 8;
	uint8 CharacterStatus;
	FTimerHandle SlowTimerHandle;
	FTimerHandle ReverseTimerHandle;
	FTimerHandle SlipperyTimerHandle;
	FTimerHandle ExhaustedTimerHandle;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties|Speed")
	float NormalSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties|Speed")
	float SprintSpeedMultiplier;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties|Sensitivity")
	float MouseSensitivity;
	UPROPERTY()
	float SprintSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float MaxStamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float Stamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float SprintStaminaCost;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float RestoreStaminaAmount;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float RestoreStaminaInterval;

	bool bIsSprinting;
	bool bIsStaminaHidden;
	float ExhaustedDuration;

	/*************** 상태 관련 변수 ***************/
	// Slow
	float InitialSpeed;
	// Reverse
	float ReverseMultiplier;
	// Slippery
	float InitialFriction;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser ) override;

	UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void StartJump(const FInputActionValue& Value);
	UFUNCTION()
	void StopJump(const FInputActionValue& Value);
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& Value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& Value);

	void OnDeath();
	void UpdateOverheadHP();
	void UpdateOverheadStamina();
	void RestoreStamina();
	void ResetExhausted();
	void ShowOverheadStamina();
	void HideOverheadStamina();

	/*************** 상태 관련 함수 ***************/
	void DeactivateSlow();
	void DeactivateReverse();
	void DeactivateSlippery();
};

