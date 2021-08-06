#include "PlayerCharacterController.h"
#include "Blueprint/UserWidget.h"

APlayerCharacterController::APlayerCharacterController()
{

}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayClass)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);

		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();

			HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}
}
