#include "Weapon.h"

AWeapon::AWeapon() : ImpulseStrength(20'000), Ammo(30), MagCapcity(30), WeaponType(EWeaponType::EWT_SMG), AmmoType(EAmmoType::EAT_9mm), ReloadMontageSectionName(FName(TEXT("Reload_SMG"))),
	MagBoneName(FName(TEXT("smg_clip"))), bIsMovingMag(false), ThrowWeaponTime(0.7F), bIsFalling(false)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Keep Weapon Upright
	if (GetItemState() == EItemState::EIS_Falling && bIsFalling)
	{
		FRotator MeshRotation { 0.F, GetItemMesh()->GetComponentRotation().Yaw, 0.F };

		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation { 0.F, GetItemMesh()->GetComponentRotation().Yaw, 0.F };

	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward { GetItemMesh()->GetForwardVector() };
	const FVector MeshRight { GetItemMesh()->GetRightVector() };

	//Direction in which the weapon is thrown
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.F, MeshForward);

	float RandomRotation { 30.F };

	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.F, 0.F, 1.F));
	ImpulseDirection *= ImpulseStrength;

	GetItemMesh()->AddImpulse(ImpulseDirection);

	bIsFalling = true;

	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);

	EnableGlowMaterial();
}

bool AWeapon::IsMagFull()
{
	return Ammo >= MagCapcity;
}

void AWeapon::DecreaseAmmo()
{
	if (Ammo - 1 <= 0)
	{
		Ammo = 0;
	}

	else
	{
		--Ammo;
	}
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	checkf(Ammo + Amount <= MagCapcity, TEXT("Ammo exceeds Mag Capcity"));

	Ammo += Amount;
}

void AWeapon::StopFalling() 
{ 
	bIsFalling = false;  

	SetItemState(EItemState::EIS_Pickup); 

	StartPulseTimer();
}
