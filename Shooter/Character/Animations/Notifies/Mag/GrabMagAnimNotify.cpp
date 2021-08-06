#include "GrabMagAnimNotify.h"
#include "Shooter/Character/PlayerCharacter.h"

void UGrabMagAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if(MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		APlayerCharacter* Character = Cast<APlayerCharacter>(MeshComp->GetOwner());

		if (Character != NULL)
			Character->GrabMag();
	}
}
