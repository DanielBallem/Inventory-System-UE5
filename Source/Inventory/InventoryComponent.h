// Copyright of Daniel Ballem 2023

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "InventoryComponent.generated.h"

namespace ItemConstants 
{
	const FString NO_ITEM_STRING = "NO_ITEM";
}

UENUM(BlueprintType)
enum class ItemType : uint8 {
	NONE = 0 UMETA(DisplayName = "NONE"),
	RESOURCE = 1 UMETA(DisplayName = "RESOURCE"),
	HEADGEAR = 2 UMETA(DisplayName = "HEADGEAR"),
	CHESTGEAR = 3 UMETA(DisplayName = "CHESTGEAR"),
	LEGGING = 4 UMETA(DisplayName = "LEGGING"),
	FOOTGEAR = 5 UMETA(DisplayName = "FOOTGEAR")
};

USTRUCT(BlueprintType)
struct FItemMetadata : public FTableRowBase {
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MaxStackAmount;
		
		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ItemType type;

	FItemMetadata() {
		MaxStackAmount = 64;
		type = ItemType::NONE;
	}
};

USTRUCT(BlueprintType)
struct FInventoryInputResponse
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadOnly)
		int32 AmountLeft;

	UPROPERTY(BlueprintReadOnly)
		bool bSuccess;

	FInventoryInputResponse() {
		AmountLeft = 0;
		bSuccess = true;
	}
	FInventoryInputResponse(int amountLeft, bool bSuccessful) {
		AmountLeft = amountLeft;
		bSuccess = bSuccessful;
	}
};

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadWrite)
		int32 CurrentAmount;

	UPROPERTY(BlueprintReadWrite)
		FName ItemName;

	UPROPERTY(BlueprintReadWrite)
		FItemMetadata ItemMetadata;

	UPROPERTY(BlueprintReadWrite)
		ItemType ItemSlotTypeRestriction;

	FInventorySlot() {
		ItemName = FName(ItemConstants::NO_ITEM_STRING);
		CurrentAmount = 0;
		ItemMetadata = FItemMetadata();
		ItemSlotTypeRestriction = ItemType::NONE;
	}

	FInventoryInputResponse AddToStack(int32 amount) {

		if (CurrentAmount == ItemMetadata.MaxStackAmount && amount > 0)
			return FInventoryInputResponse(amount, false);

		int32 newAmount = CurrentAmount + amount;
		int32 leftover = newAmount - ItemMetadata.MaxStackAmount;

		if (leftover > 0) {
			CurrentAmount = newAmount - leftover;
		}
		else {
			leftover = 0; //reset
			CurrentAmount = newAmount;
		}
		if (CurrentAmount == 0)
			ItemName = FName(ItemConstants::NO_ITEM_STRING);

		return FInventoryInputResponse(leftover, true);
	}

	//Item slots allow items into it if the slot restriction is none, the item matches the restriction, or if the type is empty (NONE)
	bool DoesItemSlotTypeMatchRestriction(ItemType type) {
		return ItemSlotTypeRestriction == type || ItemSlotTypeRestriction == ItemType::NONE || type == ItemType::NONE;
	}
};



UCLASS( ClassGroup=(Custom), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent) )
class INVENTORY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	UPROPERTY(EditDefaultsOnly)
		UDataTable* ItemTable;

	UPROPERTY(BlueprintReadWrite)
		TArray<FInventorySlot> InventorySlots;

	UFUNCTION(BlueprintCallable)
		FInventoryInputResponse DropItemIntoInventory(FName name, int32 amount);

	UFUNCTION(BlueprintCallable)
		void SwapInventorySlotsWithOtherInventory(UInventoryComponent* secondInventory, int32 myIndex, int32 otherIndex);

	UFUNCTION(BlueprintCallable)
		int32 DropItemIntoInventoryByIndex(int32 index, int32 amount);

	UFUNCTION(BlueprintCallable)
		void InsertItemIntoInventoryByIndex(FInventorySlot slotDetails, int32 index);

	UFUNCTION(BlueprintCallable)
		int32 CombineStacks(UInventoryComponent* sourceInventory, FInventorySlot transferInventorySlot, int32 index, int32 sourceIndex);

	UFUNCTION(BlueprintCallable)
		int32 TransferAndMergeToEmptyStack(UInventoryComponent* sourceInventory, FInventorySlot transferInventorySlot, int32 Toindex, int32 FromIndex);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 InventoryColSize = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 InventoryRowSize = 5;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void AddToStacksByName(FName slotToInsert, FName itemName, int32 &outAmount);
	FItemMetadata GetItemMetadata(FName);
	

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	static void CheckIndexValidity(TArray<FInventorySlot>* arr, int32 index);

		
};
