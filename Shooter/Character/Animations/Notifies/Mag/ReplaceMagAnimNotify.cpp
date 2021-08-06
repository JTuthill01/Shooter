#include "ReplaceMagAnimNotify.h"
#include "Shooter/Character/PlayerCharacter.h"

void UReplaceMagAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		APlayerCharacter* Character = Cast<APlayerCharacter>(MeshComp->GetOwner());

		if (Character != NULL)
			Character->ReleaseMag();
	}
}
