#pragma once

#include "CoreMinimal.h"
#include "Shooter/Actors/Items/ItemBase.h"
#include "Shooter/AmmoType.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8 { EWT_SMG UMETA(DisplayName = "SMG"), EWT_Rifle UMETA(DisplayName = "Rifle"), EWT_MAX UMETA(DisplayName = "Default Max") };

UCLASS()
class SHOOTER_API AWeapon : public AItemBase
{
	GENERATED_BODY()
	
public:
	AWeapon();

	bool IsMagFull();

public:
	//Getters
	FORCEINLINE int32 GetAmmo() { return Ammo; }
	FORCEINLINE int32 GetMagCapcity() { return MagCapcity; }

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }

	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSectionName; }
	FORCEINLINE FName GetMagBoneName() const { return MagBoneName; }

	FORCEINLINE bool GetIsMagMoving() const { return bIsMovingMag; }

public:
	//Setters
	FORCEINLINE void SetIsMagMoving(bool IsMagMoving) { bIsMovingMag = IsMagMoving; }

public:
	virtual void Tick(float DeltaTime) override;

	void ThrowWeapon();

	void DecreaseAmmo();

	void ReloadAmmo(int32 Amount);

protected:
	void StopFalling();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Impulse, meta = (AllowPrivateAccess = "true"))
	float ImpulseStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponProperties, meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponProperties, meta = (AllowPrivateAccess = "true"))
	int32 MagCapcity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponProperties, meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponProperties, meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponProperties, meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSectionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponProperties, meta = (AllowPrivateAccess = "true"))
	FName MagBoneName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = WeaponProperties, meta = (AllowPrivateAccess = "true"))
	bool bIsMovingMag;

private:
	FTimerHandle ThrowWeaponTimer;

	float ThrowWeaponTime;

	bool bIsFalling;
};
