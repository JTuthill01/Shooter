#include "MagOutSoundAnimNotify.h"
#include "Shooter/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void UMagOutSoundAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != nullptr && MeshComp->GetOwner() != nullptr)
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(MeshComp->GetOwner());

		if (Player)
			UGameplayStatics::SpawnSoundAttached(Player->MagOutSound, MeshComp);
	}
}
