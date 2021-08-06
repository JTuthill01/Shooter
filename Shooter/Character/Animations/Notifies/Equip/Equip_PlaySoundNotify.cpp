#include "Equip_PlaySoundNotify.h"
#include "Shooter/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void UEquip_PlaySoundNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		APlayerCharacter* Character = Cast<APlayerCharacter>(MeshComp->GetOwner());

		if (Character != NULL)
			UGameplayStatics::SpawnSoundAttached(Character->EquipSound, MeshComp);
	}
}
