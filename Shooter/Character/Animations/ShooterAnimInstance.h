#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8 { EOS_Aiming UMETA(DisplayName = "Aiming"), EOS_Hip UMETA(DisplayName = "Not Aiming"), EOS_Reloading UMETA(DisplayName = "Reloading"), EOS_InAir UMETA(DisplayName = "In Air"),
	EOS_MAX UMETA(DisplayName = "MAX") };

UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UShooterAnimInstance();
	
public:
	virtual void NativeInitializeAnimation() override;

public:
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

protected:
	UFUNCTION(BlueprintCallable)
	void TurnInPlace();

	UFUNCTION(BlueprintCallable)
	void Lean(float DeltaTime);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class APlayerCharacter* Player;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TurnInPlace, meta = (AllowPrivateAccess = "true"))
	float TIPCharacterYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TurnInPlace, meta = (AllowPrivateAccess = "true"))
	float TIPCharacterYawLastFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TurnInPlace, meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TurnInPlace, meta = (AllowPrivateAccess = "true"))
	float Pitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float RecoilWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Lean, meta = (AllowPrivateAccess = "true"))
	float YawDeltaLean;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = TurnInPlace, meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsReloading;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crouching, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouching, meta = (AllowPrivateAccess = "true"))
	bool bIsTurningInPlace;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Crouching, meta = (AllowPrivateAccess = "true"))
	bool bIsEquipping;

	FRotator CharactorRotation;
	FRotator CharactorRotationLastFrame;

	float RotationCurve;
	float RotationCurveLastFrame;
};
