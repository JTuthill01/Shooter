#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Shooter/Actors/Items/ItemBase.h"
#include "Shooter/Actors/Items/Weapons/Weapon.h"
#include "Shooter/Actors/Items/Ammo/Ammo.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
APlayerCharacter::APlayerCharacter() : CombatState(ECombatState::ECS_Unoccupied), Starting9mmAmmo(85), StartingARAmmo(120), HighlightedSlot(-1), bIsAiming(false), bIsCrouching(false), ZoomInterpSpeed(20.F), 
	HipTurnRate(90.F), HipLookUpRate(90.F), AimingTurnRate(20.F), AimingLookUpRate(20.F), CrosshairSpreadMultiplier(0.F), CrosshairVelocityFactor(0.F), CrosshairInAirFactor(0.F), CrosshairAimFactor(0.F), 
	CrosshairShootingFactor(0.F), CameraInterpDistance(250.F), CameraInterpElevation(65.F), BaseMovementSpeed(650.F), CrouchMovementSpeed(300.F), CurrentCapsuleHalfHeight(0.F), StandingCapsuleHalfHeight(88.F), 
	CrouchingCapsuleHalfHeight(44.F), BaseGroundFriction(2.F), CrouchGroundFriction(100.F), CameraDefaultFOV(0.F), CameraZoomedFOV(25.F), PickupSoundResetTime(0.2F), EquipSoundResetTime(0.2F), CameraCurrentFOV(0.F), 
	ShootTimeDuration(0.05F), AutomaticFireRate(0.1F), bIsFiringBullet(false), bIsFireButtonPressed(false), bShouldFire(true), bShouldTraceForItems(false), bIsAimingPressed(false), bShouldPlayPickupSound(true), bShouldPlayEquipSound(true)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.F;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.F, 50.F, 70.F);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	WeaponInterpComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interp Component"));
	InterpComponent1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 1"));
	InterpComponent2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 2"));
	InterpComponent3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 3"));
	InterpComponent4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 4"));
	InterpComponent5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 5"));
	InterpComponent6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 6"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	WeaponInterpComponent->SetupAttachment(Camera);
	InterpComponent1->SetupAttachment(Camera);
	InterpComponent2->SetupAttachment(Camera);
	InterpComponent3->SetupAttachment(Camera);
	InterpComponent4->SetupAttachment(Camera);
	InterpComponent5->SetupAttachment(Camera);
	InterpComponent6->SetupAttachment(Camera);

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.F, 540.F, 0.F);
	GetCharacterMovement()->JumpZVelocity = 600.F;
	GetCharacterMovement()->AirControl = 0.2F;

	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComponent"));
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (Camera)
	{
		CameraDefaultFOV = GetCamera()->FieldOfView;

		CameraCurrentFOV = CameraDefaultFOV;
	}

	EquipWeapon(SpawnDefaultWeapon());

	Inventory.Add(CurrentWeapon);

	CurrentWeapon->DisableCustomDepth();
	CurrentWeapon->DisableGlowMaterial();
	CurrentWeapon->SetSlotIndex(0);

	InitializeAmmoMap();

	InitializeInterpLocations();

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime);

	CalculateCrosshairSpread(DeltaTime);

	TraceForItems();

	InterpCapsuleHalfHeight(DeltaTime);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Make sure that we are using a UEnhancedInputComponent; if not, the project is not configured correctly.
	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// This calls the handler function on the tick when MyInputAction starts, such as when pressing an action button.
		if (JumpAction)
		{
			PlayerEnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::Jump);

			PlayerEnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		//// This calls the handler function on the tick when MyInputAction starts, such as when pressing an action button.
		if (FireActionPressed)
		{
			PlayerEnhancedInputComponent->BindAction(FireActionPressed, ETriggerEvent::Started, this, &APlayerCharacter::FireButtonPressed);

			PlayerEnhancedInputComponent->BindAction(FireActionPressed, ETriggerEvent::Completed, this, &APlayerCharacter::FireButtonReleased);
		}

		if (AimingActionPressed)
		{
			PlayerEnhancedInputComponent->BindAction(AimingActionPressed, ETriggerEvent::Started, this, &APlayerCharacter::AimingButtonPressed);

			PlayerEnhancedInputComponent->BindAction(AimingActionPressed, ETriggerEvent::Completed, this, &APlayerCharacter::AimingButtonReleased);
		}

		if (SelectAction)
		{
			PlayerEnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Started, this, &APlayerCharacter::SelectButtonPressed);

			PlayerEnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Completed, this, &APlayerCharacter::SelectButtonReleased);
		}

		if (ReloadActionPressed)
		{
			PlayerEnhancedInputComponent->BindAction(ReloadActionPressed, ETriggerEvent::Triggered, this, &APlayerCharacter::ReloadButtonPressed);
		}

		// This calls the handler function on the tick when MyInputAction starts, such as when pressing an action button.
		if (MovementAction)
		{
			PlayerEnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &APlayerCharacter::EnhancedMove);
		}

		// This calls the handler function on the tick when MyInputAction starts, such as when pressing an action button.
		if (LookAction)
		{
			PlayerEnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::EnhancedLook);
		}

		if (CrouchAction)
		{
			PlayerEnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &APlayerCharacter::CrouchedButtonPressed);
		}

		if (FKeyAction)
		{
			PlayerEnhancedInputComponent->BindAction(FKeyAction, ETriggerEvent::Triggered, this, &APlayerCharacter::FKeyPressed);
		}

		if (OneKeyAction)
		{
			PlayerEnhancedInputComponent->BindAction(OneKeyAction, ETriggerEvent::Triggered, this, &APlayerCharacter::OneKeyPressed);
		}

		if (TwoKeyAction)
		{
			PlayerEnhancedInputComponent->BindAction(TwoKeyAction, ETriggerEvent::Triggered, this, &APlayerCharacter::TwoKeyPressed);
		}

		if (ThreeKeyAction)
		{
			PlayerEnhancedInputComponent->BindAction(ThreeKeyAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ThreeKeyPressed);
		}

		if (FourKeyAction)
		{
			PlayerEnhancedInputComponent->BindAction(FourKeyAction, ETriggerEvent::Triggered, this, &APlayerCharacter::FourKeyPressed);
		}

		if (FiveKeyAction)
		{
			PlayerEnhancedInputComponent->BindAction(FiveKeyAction, ETriggerEvent::Triggered, this, &APlayerCharacter::FiveKeyPressed);
		}
	}
}

void APlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	// Make sure that we have a valid PlayerController.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// Get the Enhanced Input Local Player Subsystem from the Local Player related to our Player Controller.
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// PawnClientRestart can run more than once in an Actor's lifetime, so start by clearing out any leftover mappings.
			Subsystem->ClearAllMappings();

			// Add each mapping context, along with their priority values. Higher values out prioritize lower values.
			Subsystem->AddMappingContext(BaseMappingContext, BaseMappingPriority);
		}
	}
}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void APlayerCharacter::Jump()
{
	if (bIsCrouching)
	{
		bIsCrouching = false;

		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}

	else
		ACharacter::Jump();
}

void APlayerCharacter::EnhancedMove(const FInputActionValue& Value)
{
	if (Value.GetMagnitude() != 0.F)
	{
		AddMovementInput(GetActorForwardVector(), Value[1]);

		AddMovementInput(GetActorRightVector(), Value[0]);
	}
}

void APlayerCharacter::EnhancedLook(const FInputActionValue& Value)
{
	AddControllerPitchInput(Value[1] * -1);

	AddControllerYawInput(Value[0]);
}

void APlayerCharacter::EnhancedFireWeapon()
{
	if (CurrentWeapon == nullptr)
	{
		return;
	}

	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		return;
	}

	if (DoesWeaponHaveAmmo())
	{
		PlayFireSound();

		SendBullet();

		PlayGunFireMontage();

		CurrentWeapon->DecreaseAmmo();

		StartFireTimer();
	}
	
	//StartCrosshairBulletFire();
}

void APlayerCharacter::PlayFireSound()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void APlayerCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = CurrentWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");

	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(CurrentWeapon->GetItemMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;

		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		if (bBeamEnd)
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}

			UParticleSystemComponent* LocalBeam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);

			if (LocalBeam)
			{
				LocalBeam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

void APlayerCharacter::PlayGunFireMontage()
{
	UAnimInstance* Instance = GetMesh()->GetAnimInstance();

	if (Instance)
	{
		Instance->Montage_Play(HipFireMontage);
		Instance->Montage_JumpToSection(FName("StartFire"));
	}
}

void APlayerCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied || !CurrentWeapon)
		return;

	UAnimInstance* Instance = GetMesh()->GetAnimInstance();

	if (IsCarryingAmmo() && !CurrentWeapon->IsMagFull())
	{
		if (bIsAiming)
			StopAiming();
		
		CombatState = ECombatState::ECS_Reloading;

		if (ReloadMontage && Instance)
		{
			Instance->Montage_Play(ReloadMontage);
			Instance->Montage_JumpToSection(CurrentWeapon->GetReloadMontageSection());
		}
	}
}

void APlayerCharacter::Aim()
{
	bIsAiming = true;

	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void APlayerCharacter::StopAiming()
{
	bIsAiming = false;

	if (!bIsCrouching)
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

void APlayerCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight { };

	if (bIsCrouching)
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;

	else
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;

	const float InterpHalfHeight { FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.F) };

	const float DeltaHalfHeight { InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() };

	const FVector MeshOffset { 0.F, 0.F, -DeltaHalfHeight };

	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void APlayerCharacter::PickupAmmo(AAmmo* Ammo)
{
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		int32& AmmoCount { AmmoMap[Ammo->GetAmmoType()] };
		
		AmmoCount += Ammo->GetItemCount();
	}

	if (CurrentWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		if (CurrentWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void APlayerCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocations { WeaponInterpComponent, 0 };
	InterpLocations.Add(WeaponLocations);

	FInterpLocation InterpLocation1 { InterpComponent1, 0 };
	InterpLocations.Add(InterpLocation1);

	FInterpLocation InterpLocation2 { InterpComponent2, 0 };
	InterpLocations.Add(InterpLocation2);

	FInterpLocation InterpLocation3 { InterpComponent3, 0 };
	InterpLocations.Add(InterpLocation3);

	FInterpLocation InterpLocation4 { InterpComponent4, 0 };
	InterpLocations.Add(InterpLocation4);

	FInterpLocation InterpLocation5 { InterpComponent5, 0 };
	InterpLocations.Add(InterpLocation5);

	FInterpLocation InterpLocation6 { InterpComponent6, 0 };
	InterpLocations.Add(InterpLocation6);
}

void APlayerCharacter::CameraInterpZoom(float DeltaTime)
{
	if (bIsAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}

	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	GetCamera()->SetFieldOfView(CameraCurrentFOV);
}

void APlayerCharacter::SetLookRates(float DeltaTime)
{
}

void APlayerCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange { 0.F, 600.F };
	FVector2D VelocityMultiplierRange { 0.F, 1.F };

	FVector Velocity { GetVelocity() };
	Velocity.Z = 0.F;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	if (GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25F, DeltaTime, 2.25F);
	}

	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.F, DeltaTime, 30.F);
	}

	if (bIsAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6F, DeltaTime, 30.F);
	}

	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.F, DeltaTime, 30.F);
	}

	if (bIsFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3F, DeltaTime, 60.F);
	}

	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.F, DeltaTime, 60.F);
	}

	CrosshairSpreadMultiplier = 0.5F + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void APlayerCharacter::StartCrosshairBulletFire()
{
	bIsFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimeHandle, this, &APlayerCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void APlayerCharacter::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;
	
	GetWorldTimerManager().SetTimer(AutoFireTimerHandle, this, &APlayerCharacter::AutoFireReset, AutomaticFireRate);
}

void APlayerCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bIsSwapping)
{
	if (WeaponToEquip)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("Right_Hand_Socket"));

		if (HandSocket)
			HandSocket->AttachActor(WeaponToEquip, GetMesh());

		if (CurrentWeapon == nullptr)
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());

		else if (!bIsSwapping)
			EquipItemDelegate.Broadcast(CurrentWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());

		CurrentWeapon = WeaponToEquip;

		CurrentWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void APlayerCharacter::DropWeapon()
{
	if (CurrentWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);

		CurrentWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		CurrentWeapon->SetItemState(EItemState::EIS_Falling);

		CurrentWeapon->ThrowWeapon();
	}
}

void APlayerCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);

	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

AWeapon* APlayerCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}

	return nullptr;
}

int32 APlayerCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;

	for (int32 i = 1; i < InterpLocations.Num(); ++i)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;

			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

bool APlayerCharacter::DoesWeaponHaveAmmo()
{
	if (CurrentWeapon == nullptr)
	{
		return false;
	}

	return CurrentWeapon->GetAmmo() > 0;
}

bool APlayerCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	FVector2D ViewportSize;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.F, ViewportSize.Y / 2.F);
	CrosshairLocation.Y -= 50.F;

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		const FVector Start = CrosshairWorldPosition;
		const FVector End = Start + CrosshairWorldDirection * 50'000.F;

		OutHitLocation = End;

		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;

			return true;
		}
	}

	return false;
}

bool APlayerCharacter::IsCarryingAmmo()
{
	if (CurrentWeapon == nullptr)
	{
		return false;
	}

	auto AmmoType = CurrentWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

int32 APlayerCharacter::GetEmptyInventorySlot()
{
	for (int32 i = 0; i < Inventory.Num(); ++i)
	{
		if (Inventory[i] == nullptr)
			return i;
	}

	if (Inventory.Num() < INVENTORY_CAPCITY)
		return Inventory.Num();

	return -1; //Inventory is full at this point
}

bool APlayerCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FHitResult CrosshairHitResult;

	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	if (bCrosshairHit)
	{
		OutBeamLocation = CrosshairHitResult.Location;
	}

	FHitResult WeaponTraceHit;

	const FVector WeaponTraceStart = MuzzleSocketLocation;
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd = OutBeamLocation + StartToEnd * 1.25F;

	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if (WeaponTraceHit.bBlockingHit)
	{
		OutBeamLocation = WeaponTraceHit.Location;

		return true;
	}

	return false;
}

void APlayerCharacter::FireButtonPressed()
{
	bIsFireButtonPressed = true;

	EnhancedFireWeapon();
}

void APlayerCharacter::FireButtonReleased()
{
	bIsFireButtonPressed = false;
}

void APlayerCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void APlayerCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;

		FVector HitLocation;

		TraceUnderCrosshairs(ItemTraceResult, HitLocation);

		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItemBase>(ItemTraceResult.Actor);

			const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);

			if (TraceHitWeapon)
			{
				if (HighlightedSlot == -1)
					HighlightInventorySlot();
			}

			else
			{
				if (HighlightedSlot != -1)
					UnHighlightInventorySlot();
			}

			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
				TraceHitItem = nullptr;

			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				TraceHitItem->GetPickupWidget()->SetVisibility(true);

				TraceHitItem->EnableCustomDepth();

				if (Inventory.Num() >= INVENTORY_CAPCITY)
					TraceHitItem->SetIsPlayerInventoryFull(true);

				else
					TraceHitItem->SetIsPlayerInventoryFull(false);
			}

			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}

			TraceHitItemLastFrame = TraceHitItem;
		}
	}

	else if (TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
	}
}

void APlayerCharacter::SelectButtonPressed()
{
	if (CombatState != ECombatState::ECS_Unoccupied)
		return;

	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this);

		TraceHitItem = nullptr;
	}
}

void APlayerCharacter::SelectButtonReleased()
{
}

void APlayerCharacter::CrouchedButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
		bIsCrouching = !bIsCrouching;

	if (bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;

		GetCharacterMovement()->GroundFriction = CrouchGroundFriction;
	}

	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
}

void APlayerCharacter::SwapWeapon(AWeapon* Weapon)
{
	if (Inventory.Num() - 1 >= CurrentWeapon->GetSlotIndex())
	{
		Inventory[CurrentWeapon->GetSlotIndex()] = Weapon;

		Weapon->SetSlotIndex(CurrentWeapon->GetSlotIndex());
	}

	DropWeapon();

	EquipWeapon(Weapon, true);

	TraceHitItem = nullptr;

	TraceHitItemLastFrame = nullptr;
}

void APlayerCharacter::FKeyPressed()
{
	if (CurrentWeapon->GetSlotIndex() == 0)
		return;

	ExchangeInventoryItems(CurrentWeapon->GetSlotIndex(), 0);
}

void APlayerCharacter::OneKeyPressed()
{
	if (CurrentWeapon->GetSlotIndex() == 1)
		return;

	ExchangeInventoryItems(CurrentWeapon->GetSlotIndex(), 1);
}

void APlayerCharacter::TwoKeyPressed()
{
	if (CurrentWeapon->GetSlotIndex() == 2)
		return;

	ExchangeInventoryItems(CurrentWeapon->GetSlotIndex(), 2);
}

void APlayerCharacter::ThreeKeyPressed()
{
	if (CurrentWeapon->GetSlotIndex() == 3)
		return;

	ExchangeInventoryItems(CurrentWeapon->GetSlotIndex(), 3);
}

void APlayerCharacter::FourKeyPressed()
{
	if (CurrentWeapon->GetSlotIndex() == 4)
		return;

	ExchangeInventoryItems(CurrentWeapon->GetSlotIndex(), 4);
}

void APlayerCharacter::FiveKeyPressed()
{
	if (CurrentWeapon->GetSlotIndex() == 5)
		return;

	ExchangeInventoryItems(CurrentWeapon->GetSlotIndex(), 5);
}

void APlayerCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	const bool bCanExchangeItems = (CurrentItemIndex != NewItemIndex) && (NewItemIndex < Inventory.Num()) && (CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Equipping);

	if (bCanExchangeItems)
	{
		if (bIsAiming)
			StopAiming();

		auto OldEquippedWeapon = CurrentWeapon;

		auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);

		EquipWeapon(NewWeapon);

		OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);

		NewWeapon->SetItemState(EItemState::EIS_Equipped);

		CombatState = ECombatState::ECS_Equipping;

		UAnimInstance* Instance = GetMesh()->GetAnimInstance();

		if (Instance && EquippingMontage)
		{
			Instance->Montage_Play(EquippingMontage, 1.F);
			Instance->Montage_JumpToSection(FName("Equip"));
		}
	}
}

void APlayerCharacter::AutoFireReset() 
{ 
	CombatState = ECombatState::ECS_Unoccupied;

	if (DoesWeaponHaveAmmo())
	{
		if (bIsFireButtonPressed)
		{
			EnhancedFireWeapon();
		}
	}

	else
	{
		ReloadWeapon();
	}
}

void APlayerCharacter::IncrementItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;

		bShouldTraceForItems = false;
	}

	else
	{
		OverlappedItemCount += Amount;

		bShouldTraceForItems = true;
	}
}

void APlayerCharacter::GetPickupItem(AItemBase* Item)
{
	Item->PlayEquipSound();

	auto Weapon = Cast<AWeapon>(Item);

	if (Weapon)
	{
		if (Inventory.Num() < INVENTORY_CAPCITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());

			Inventory.Add(Weapon);

			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}

		else
			SwapWeapon(Weapon);
	}

	auto Ammo = Cast<AAmmo>(Item);

	if (Ammo)
		PickupAmmo(Ammo);
}

void APlayerCharacter::IncrementInterpLocationItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1)
		return;

	if (InterpLocations.Num() >= Index)
		InterpLocations[Index].ItemCount += Amount;
}

void APlayerCharacter::StartPickupSoundTimer()
{
	GetWorldTimerManager().SetTimer(PlayPickupSoundHandle, this, &APlayerCharacter::ResetPickupSoundTimer, PickupSoundResetTime);

	bShouldPlayPickupSound = false;
}

void APlayerCharacter::StartEquipSoundTimer()
{
	GetWorldTimerManager().SetTimer(PlayPickupSoundHandle, this, &APlayerCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
	
	bShouldPlayEquipSound = false;
}

FInterpLocation APlayerCharacter::GetInterpLocation(int32 Index)
{
	if (Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}

	return FInterpLocation();
}

void APlayerCharacter::FinishedReloading()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (!CurrentWeapon)
		return;

	if (bIsAimingPressed)
		Aim();

	const auto LocalAmmoType { CurrentWeapon->GetAmmoType() };

	if (AmmoMap.Contains(LocalAmmoType))
	{
		int32 CarriedAmmo = AmmoMap[LocalAmmoType];

		const int32 MagEmptySpace = CurrentWeapon->GetMagCapcity() - CurrentWeapon->GetAmmo();

		if (MagEmptySpace > CarriedAmmo)
		{
			CurrentWeapon->ReloadAmmo(CarriedAmmo);

			CarriedAmmo = 0;

			AmmoMap.Add(LocalAmmoType, CarriedAmmo);
		}

		else
		{
			CurrentWeapon->ReloadAmmo(MagEmptySpace);

			CarriedAmmo -= MagEmptySpace;

			AmmoMap.Add(LocalAmmoType, CarriedAmmo);
		}
	}
}

void APlayerCharacter::FinishedEquipping()
{
	CombatState = ECombatState::ECS_Unoccupied;
}

void APlayerCharacter::GrabMag()
{
	if (!CurrentWeapon)
		return;

	if (!HandSceneComponent)
		return;

	int32 MagBoneIndex { CurrentWeapon->GetItemMesh()->GetBoneIndex(CurrentWeapon->GetMagBoneName()) };

	MagTransform = CurrentWeapon->GetItemMesh()->GetBoneTransform(MagBoneIndex);

	FAttachmentTransformRules Rules(EAttachmentRule::KeepRelative, true);

	HandSceneComponent->AttachToComponent(GetMesh(), Rules, FName(TEXT("hand_l")));
	HandSceneComponent->SetWorldTransform(MagTransform);

	CurrentWeapon->SetIsMagMoving(true);
}

void APlayerCharacter::ReleaseMag()
{
	CurrentWeapon->SetIsMagMoving(false);
}

void APlayerCharacter::AimingButtonPressed() 
{ 
	bIsAimingPressed = true;

	if (CombatState != ECombatState::ECS_Reloading)
	{
		Aim();
	}
}

void APlayerCharacter::AimingButtonReleased() 
{ 
	bIsAimingPressed = false;

	StopAiming();
}

void APlayerCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot { GetEmptyInventorySlot() };

	HighlightIconDelegate.Broadcast(EmptySlot, true);

	HighlightedSlot = EmptySlot;
}

void APlayerCharacter::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);

	HighlightedSlot = -1;
}

float APlayerCharacter::GetCrosshairSpreadMultiplier() const { return CrosshairSpreadMultiplier; }

void APlayerCharacter::FinishCrosshairBulletFire() { bIsFiringBullet = false; }

void APlayerCharacter::ResetPickupSoundTimer(){ bShouldPlayPickupSound = true; GetWorldTimerManager().ClearTimer(PlayPickupSoundHandle); }

void APlayerCharacter::ResetEquipSoundTimer(){ bShouldPlayEquipSound = true; GetWorldTimerManager().ClearTimer(PlayEquipSoundHandle); }

