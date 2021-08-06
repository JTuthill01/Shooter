#include "MagInSoundAnimNotify.h"
#include "Shooter/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void UMagInSoundAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != nullptr && MeshComp->GetOwner() != nullptr)
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(MeshComp->GetOwner());

		if (Player)
			UGameplayStatics::SpawnSoundAttached(Player->MagOutSound, MeshComp);
	}
}
