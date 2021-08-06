#include "ItemBase.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Shooter/Character/PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Curves/CurveVector.h"

// Sets default values
AItemBase::AItemBase() : ItemInterpStartLocation(FVector(0.F)), CameraTargetLocation(FVector(0.F)), ZCurveTime(0.7F), InterpSpeedX(30.F), InterpSpeedY(30.F), PulseCurveTime(5.F), GlowAmount(150.F), 
	FresnelExponent(3.F), FresnelReflectFraction(4.F), ItemName(FString("Default")), ItemCount(0),  MaterialIndex(0), InterpLocationIndex(0), SlotIndex(0), ItemRarity(EItemRarity::EIR_Common), 
	ItemState(EItemState::EIS_Pickup), ItemType(EItemType::EIT_MAX), bIsInterping(false), bIsPlayerInventoryFull(false), ItemInterpX(0.F), ItemInterpY(0.F), InterpInitialYawOffset(0.F), bCanSetCustomDepth(true)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Item Mesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup Widget"));
	PickupWidget->SetupAttachment(ItemMesh);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Area Sphere"));
	AreaSphere->SetupAttachment(ItemMesh);
	AreaSphere->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	SetActiveStars();

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemBase::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItemBase::OnSphereEndOverlap);

	SetItemState(ItemState);

	InitializeCustomDepth();

	StartPulseTimer();
}

void AItemBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (MaterialInstance)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);

		ItemMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);

		EnableGlowMaterial();
	}
}

// Called every frame
void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ItemInterp(DeltaTime);

	UpdatePulse();
}

void AItemBase::SetItemState(EItemState State)
{
	ItemState = State;

	SetItemProperties(State);
}

void AItemBase::StartItemCurve(APlayerCharacter* Player)
{
	Character = Player;

	InterpLocationIndex = Character->GetInterpLocationIndex();

	Character->IncrementInterpLocationItemCount(InterpLocationIndex, 1);

	PlayPickupSound();

	ItemInterpStartLocation = GetActorLocation();

	bIsInterping = true;

	SetItemState(EItemState::EIS_EquipInterping);

	GetWorldTimerManager().ClearTimer(PulseTimerHandle);

	GetWorldTimerManager().SetTimer(ItemInterpHandle, this, &AItemBase::FinshedInterping, ZCurveTime);

	const float CameraRotationYaw { Character->GetCamera()->GetComponentRotation().Yaw };
	const float ItemRotationYaw { GetActorRotation().Yaw };

	InterpInitialYawOffset = ItemRotationYaw - CameraRotationYaw;

	bCanSetCustomDepth = false;
}

void AItemBase::FinshedInterping()
{
	bIsInterping = false;

	if (Character)
	{
		Character->IncrementInterpLocationItemCount(InterpLocationIndex, -1);

		Character->GetPickupItem(this);

		Character->UnHighlightInventorySlot();
	}

	SetActorScale3D(FVector(1.F));

	DisableGlowMaterial();

	bCanSetCustomDepth = true;

	DisableCustomDepth();
}

void AItemBase::ItemInterp(float DeltaTime)
{
	if (!bIsInterping)
		return;

	if (Character && ItemZCurve)
	{
		const float ElaspedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpHandle);
		const float CurveValue = ItemZCurve->GetFloatValue(ElaspedTime);

		FVector ItemLocation = ItemInterpStartLocation;

		const FVector CameraInterpLocation { GetInterpLocation() };
		const FVector ItemToCamera { FVector(0.F, 0.F, (CameraInterpLocation - ItemLocation).Z) };

		const float DeltaZ = ItemToCamera.Size();

		const FVector CurrentLocation { GetActorLocation() };

		const float InterpXValue = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, InterpSpeedX);

		const float InterpYValue = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, InterpSpeedY);

		ItemLocation.X = InterpXValue;

		ItemLocation.Y = InterpYValue;

		ItemLocation.Z += CurveValue * DeltaZ;

		SetActorLocation(ItemLocation, false, nullptr, ETeleportType::TeleportPhysics);

		const FRotator CameraRotation { Character->GetCamera()->GetComponentRotation() };

		FRotator ItemRotation { 0.F, CameraRotation.Yaw + InterpInitialYawOffset, 0.F };

		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		if (ItemScaleCurve)
		{
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElaspedTime);

			SetActorScale3D(FVector(ScaleCurveValue));
		}
	}
}

void AItemBase::PlayPickupSound()
{
	if (Character)
	{
		if (Character->GetShouldPlayPickupSound())
		{
			Character->StartPickupSoundTimer();

			if (PickupSound)
				UGameplayStatics::PlaySound2D(this, PickupSound);
		}
	}
}

void AItemBase::EnableGlowMaterial()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 0.F);
	}
}

void AItemBase::StartPulseTimer()
{
	if (ItemState == EItemState::EIS_Pickup)
		GetWorldTimerManager().SetTimer(PulseTimerHandle, this, &AItemBase::ResetPulseTimer, PulseCurveTime);
}

void AItemBase::UpdatePulse()
{
	float ElaspedTime{};

	FVector CurveValue{};

	switch (ItemState)
	{
	case EItemState::EIS_Pickup:

		if (PulseCurve)
		{
			ElaspedTime = GetWorldTimerManager().GetTimerElapsed(PulseTimerHandle);

			CurveValue = PulseCurve->GetVectorValue(ElaspedTime);
		}

		break;

	case EItemState::EIS_EquipInterping:

		if (InterpPulseCurve)
		{
			ElaspedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpHandle);
			CurveValue = InterpPulseCurve->GetVectorValue(ElaspedTime);
		}

		break;

	default:
		break;
	}

	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("Glow Amount"), CurveValue.X * GlowAmount);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("Fresnel Exponent Value"), CurveValue.Y * FresnelExponent);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("Fresnel Reflect Fraction"), CurveValue.Z * FresnelReflectFraction);
	}
}

void AItemBase::DisableGlowMaterial()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 1.F);
	}
}

void AItemBase::PlayEquipSound()
{
	if (Character)
	{
		if (Character->GetShouldPlayEquipSound())
		{
			Character->StartEquipSoundTimer();

			if (PickupSound)
				UGameplayStatics::PlaySound2D(this, EquipSound);
		}
	}
}

FVector AItemBase::GetInterpLocation()
{
	if (!Character)
		return FVector(0.F);

	switch (ItemType)
	{
	case EItemType::EIT_Ammo:

		return Character->GetInterpLocation(InterpLocationIndex).SceneComponent->GetComponentLocation();

		break;

	case EItemType::EIT_Weapon:

		return Character->GetInterpLocation(NULL).SceneComponent->GetComponentLocation();
		
		break;

	case EItemType::EIT_MAX:
		break;

	default:
		break;
	}

	return FVector();
}

void AItemBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);

		if (Player)
		{
			Player->IncrementItemCount(1);
		}
	}
}

void AItemBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);

		if (Player)
		{
			Player->IncrementItemCount(-1);

			Player->UnHighlightInventorySlot();
		}
	}
}

void AItemBase::SetActiveStars()
{
	for (size_t i = 0; i <= 5; ++i)
	{
		ActiveStars.Add(false);
	}

	switch (ItemRarity)
	{
	case EItemRarity::EIR_Damaged:

		ActiveStars[1] = true;

		break;

	case EItemRarity::EIR_Common:

		ActiveStars[1] = true;
		ActiveStars[2] = true;

		break;

	case EItemRarity::EIR_Uncommon:

		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;

		break;

	case EItemRarity::EIR_Rare:

		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;

		break;

	case EItemRarity::EIR_Legendary:

		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		ActiveStars[5] = true;

		break;

	case EItemRarity::EIR_MAX:
		break;

	default:
		break;
	}
}

void AItemBase::SetItemProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_Pickup:

		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		break;

	case EItemState::EIS_EquipInterping:

		PickupWidget->SetVisibility(false);

		// Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;

	case EItemState::EIS_PickedUp:

		PickupWidget->SetVisibility(false);

		// Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(false);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;

	case EItemState::EIS_Equipped:

		// Set mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;

	case EItemState::EIS_Falling:

		// Set mesh properties
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;

	case EItemState::EIS_Max:
		break;

	default:
		break;
	}
}

void AItemBase::InitializeCustomDepth() { DisableCustomDepth(); }

void AItemBase::EnableCustomDepth() 
{ 
	if (bCanSetCustomDepth)
		ItemMesh->SetRenderCustomDepth(true);
}

void AItemBase::DisableCustomDepth() 
{ 
	if (bCanSetCustomDepth)
		ItemMesh->SetRenderCustomDepth(false);
}

void AItemBase::ResetPulseTimer() { StartPulseTimer(); }

