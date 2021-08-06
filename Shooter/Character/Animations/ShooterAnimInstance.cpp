#include "ShooterAnimInstance.h"
#include "Shooter/Character/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() : Speed(0.F), MovementOffsetYaw(0.F), LastMovementOffsetYaw(0.F), TIPCharacterYaw(0.F), TIPCharacterYawLastFrame(0.F), RootYawOffset(0.F), Pitch(0.F),
	RecoilWeight(1.F), YawDeltaLean(0.F), OffsetState(EOffsetState::EOS_Hip), bIsInAir(false), bIsAccelerating(false),  bIsAiming(false), bIsReloading(false), bIsCrouching(false), bIsTurningInPlace(false),
	bIsEquipping(false), CharactorRotation(FRotator(0.F)), CharactorRotationLastFrame(FRotator(0.F))
{
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	Player = Cast<APlayerCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (!Player)
	{
		Player = Cast<APlayerCharacter>(TryGetPawnOwner());
	}

	if (Player)
	{
		bIsReloading = Player->GetCombatState() == ECombatState::ECS_Reloading;

		bIsCrouching = Player->GetIsCrouching();

		bIsEquipping = Player->GetCombatState() == ECombatState::ECS_Equipping;

		FVector Velocity = Player->GetVelocity();

		Velocity.Z = 0.F;

		Speed = Velocity.Size();

		bIsInAir = Player->GetCharacterMovement()->IsFalling();

		if (Player->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.F)
		{
			bIsAccelerating = true;
		}

		else
		{
			bIsAccelerating = false;
		}

		FRotator AimRotation = Player->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Player->GetVelocity());

		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

		if (Player->GetVelocity().Size() > 0.F)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}

		bIsAiming = Player->GetAiming();

		if (bIsReloading)
			OffsetState = EOffsetState::EOS_Reloading;

		else if (bIsInAir)
			OffsetState = EOffsetState::EOS_InAir;

		else if (Player->GetAiming())
			OffsetState = EOffsetState::EOS_Aiming;

		else
			OffsetState = EOffsetState::EOS_Hip;
	}

	TurnInPlace();

	Lean(DeltaTime);
}

void UShooterAnimInstance::TurnInPlace()
{
	if (!Player)
		return;

	Pitch = Player->GetBaseAimRotation().Pitch;

	if (Speed > 0 || bIsInAir)
	{
		RootYawOffset = 0.F;

		TIPCharacterYaw = Player->GetActorRotation().Yaw;

		TIPCharacterYawLastFrame = TIPCharacterYaw;

		RotationCurveLastFrame = 0.F;

		RotationCurve = 0.F;
	}

	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;

		TIPCharacterYaw = Player->GetActorRotation().Yaw;

		const float YawDelta { TIPCharacterYaw - TIPCharacterYawLastFrame };

		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);

		const float Turning { GetCurveValue(TEXT("Turning")) };

		if (Turning > 0.F)
		{
			bIsTurningInPlace = true;

			RotationCurveLastFrame = RotationCurve;

			RotationCurve = GetCurveValue(TEXT("Rotation"));

			const float DeltaRotation { RotationCurve - RotationCurveLastFrame };

			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float ABSRootYawOffset { FMath::Abs(RootYawOffset) };

			if (ABSRootYawOffset > 90.F)
			{
				const float YawExcess { ABSRootYawOffset - 90.F };

				RootYawOffset > 0.F ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}

		else
			bIsTurningInPlace = false;
	}

	if (bIsTurningInPlace)
	{
		if (bIsReloading || bIsEquipping)
			RecoilWeight = 1.F;

		else
			RecoilWeight = 0.F;
	}

	else
	{
		if (bIsCrouching)
		{
			if (bIsReloading || bIsEquipping)
				RecoilWeight = 1.F;

			else
				RecoilWeight = 0.1F;
		}

		else
		{
			if (bIsAiming || bIsReloading || bIsEquipping)
				RecoilWeight = 1.F;

			else
				RecoilWeight = 0.5F;
		}
	}
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if (!Player)
		return;
	
	CharactorRotationLastFrame = CharactorRotation;
	CharactorRotation = Player->GetActorRotation();

	const FRotator Delta { UKismetMathLibrary::NormalizedDeltaRotator(CharactorRotation, CharactorRotationLastFrame) };

	const float Target { Delta.Yaw / DeltaTime };
	const float Interp { FMath::FInterpTo(YawDeltaLean, Target, DeltaTime, 6.F) };

	YawDeltaLean = FMath::Clamp(Interp, -90.F, 90.F);
}
