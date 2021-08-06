#include "FinishedReloadAnimNotify.h"
#include "Shooter/Character/PlayerCharacter.h"

void UFinishedReloadAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		APlayerCharacter* Character = Cast<APlayerCharacter>(MeshComp->GetOwner());

		if (Character != nullptr)
			Character->FinishedReloading();
	}
}
