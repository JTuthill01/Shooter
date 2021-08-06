#include "WeaponAnimInstance.h"
#include "Shooter/Character/PlayerCharacter.h"
#include "Shooter/Actors/Items/Weapons/Weapon.h"
#include "Kismet/GameplayStatics.h"

void UWeaponAnimInstance::NativeInitializeAnimation()
{
	PlayerRef = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

void UWeaponAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (!PlayerRef )
		PlayerRef = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	
	if (PlayerRef)
	{
		HandSceneComponentTransform = PlayerRef->GetHandSceneComponent()->GetComponentTransform();

		if (!BaseWeapon)
		{
			BaseWeapon = Cast<AWeapon>(GetOwningActor());

			bIsMagMoving = BaseWeapon->GetIsMagMoving();
		}

		if (BaseWeapon)
			bIsMagMoving = BaseWeapon->GetIsMagMoving();
	}
}
