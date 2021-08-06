#include "FinishEquipping.h"
#include "Shooter/Character/PlayerCharacter.h"

void UFinishEquipping::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		APlayerCharacter* Character = Cast<APlayerCharacter>(MeshComp->GetOwner());

		if (Character != NULL)
			Character->FinishedEquipping();
	}
}
