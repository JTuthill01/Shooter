#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "ItemBase.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8 { EIR_Damaged UMETA(DisplayName = "Damaged"), EIR_Common UMETA(DisplayName = "Common"), EIR_Uncommon UMETA(DisplayName = "Uncommon"), EIR_Rare UMETA(DisplayName = "Rare"), 
	EIR_Legendary UMETA(DisplayName = "Legendary"), EIR_MAX UMETA(DisplayName = "DefaultMAX") };

UENUM(BlueprintType)
enum class EItemState : uint8 { EIS_Pickup UMETA(DisplayName = "Pickup"), EIS_EquipInterping UMETA(DisplayName = "EquipInterping"), EIS_PickedUp UMETA(DisplayName = "PickedUp"), 
	EIS_Equipped UMETA(DisplayName = "Equipped"), EIS_Falling UMETA(DisplayName = "Falling"), EIS_Max UMETA(DisplayName = "DefaultMax") };

UENUM(BlueprintType)
enum class EItemType : uint8 { EIT_Ammo UMETA(DisplayName = "Ammo"), EIT_Weapon UMETA(DisplayName = "Weapon"), EIT_MAX UMETA(DisplayName = "DefaultMax") };

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackground;
};

UCLASS()
class SHOOTER_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemBase();

	void SetItemState(EItemState State);
	void StartItemCurve(class APlayerCharacter* Player);
	void PlayEquipSound();
	void DisableGlowMaterial();

public:
	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();

public:
	FORCEINLINE class UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE class USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE class UBoxComponent* GetCollisionBox() const { return CollisionBox; }
	FORCEINLINE class USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE class USoundCue* GetEquipSound() const { return EquipSound; }

	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }

	FORCEINLINE EItemState GetItemState() const { return ItemState; }

	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }

	FORCEINLINE EItemType GetItemType() const { return ItemType; }

	FORCEINLINE bool GetIsPlayerInventoryFull() { return bIsPlayerInventoryFull; }

public:
	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	FORCEINLINE void SetIsPlayerInventoryFull(bool IsInventoryFull) { bIsPlayerInventoryFull = IsInventoryFull; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void InitializeCustomDepth();
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	void FinshedInterping();
	void ItemInterp(float DeltaTime);
	void PlayPickupSound();
	void EnableGlowMaterial();
	void ResetPulseTimer();
	void StartPulseTimer();
	void UpdatePulse();

	FVector GetInterpLocation();

protected:
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void SetActiveStars();

	UFUNCTION()
	virtual void SetItemProperties(EItemState State);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Curves, meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* ItemZCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Curves, meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* ItemScaleCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Curves, meta = (AllowPrivateAccess = "true"))
	class UCurveVector* PulseCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Curves, meta = (AllowPrivateAccess = "true"))
	UCurveVector* InterpPulseCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds, meta = (AllowPrivateAccess = "true"))
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds, meta = (AllowPrivateAccess = "true"))
	class USoundCue* EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials, meta = (AllowPrivateAccess = "true"))
	class UMaterialInstanceDynamic* DynamicMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials, meta = (AllowPrivateAccess = "true"))
	class UMaterialInstance* MaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	class UTexture2D* IconBackground;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	class UTexture2D* IconItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	class UTexture2D* AmmoIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	class UDataTable* ItemRarityDataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerRef, meta = (AllowPrivateAccess = "true"))
	class APlayerCharacter* Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	FVector CameraTargetLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Curves, meta = (AllowPrivateAccess = "true"))
	float ZCurveTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed, meta = (AllowPrivateAccess = "true"))
	float InterpSpeedX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed, meta = (AllowPrivateAccess = "true"))
	float InterpSpeedY;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Curves, meta = (AllowPrivateAccess = "true"))
	float PulseCurveTime;

	UPROPERTY(VisibleAnywhere, Category = Materials, meta = (AllowPrivateAccess = "true"))
	float GlowAmount;

	UPROPERTY(VisibleAnywhere, Category = Materials, meta = (AllowPrivateAccess = "true"))
	float FresnelExponent;

	UPROPERTY(VisibleAnywhere, Category = Materials, meta = (AllowPrivateAccess = "true"))
	float FresnelReflectFraction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials, meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	int32 InterpLocationIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AmmoItem, meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	EItemType ItemType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ItemProperties, meta = (AllowPrivateAccess = "true"))
	bool bIsInterping;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	bool bIsPlayerInventoryFull;

private:
	float ItemInterpX;
	float ItemInterpY;
	float InterpInitialYawOffset;

	bool bCanSetCustomDepth;

	FTimerHandle ItemInterpHandle;
	FTimerHandle PulseTimerHandle;
};
