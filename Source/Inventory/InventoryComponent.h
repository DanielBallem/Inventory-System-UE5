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

/*
	ItemType defines categories that an item can exist as. Used for restricting certain items to certain slots.
*/
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

	/*
		Adds a specified amount to the item in this slot.

		@param amount to add in the stack.
		@return amount that couldn't be placed in the stack due to max stack size.
	*/
	int32 AddToStack(int32 amount) {
		if (CurrentAmount == ItemMetadata.MaxStackAmount && amount > 0)
			return amount;

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

		return leftover;
	}

	//Item slots allow items into it if the slot restriction is none, the item matches the restriction, or if the type of object entering is empty (NONE)
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

	//reference to a table holding Item metadata. Is used to set the metadata of new items when assigning slots.
	UPROPERTY(EditDefaultsOnly)
		UDataTable* ItemTable;

	UPROPERTY(BlueprintReadWrite)
		TArray<FInventorySlot> InventorySlots;

	/**
	  Attempts to find an available slot in the inventory to place the specified item.
	 
	  It prioritizes filling slots with matching items until they are full, then proceeds
	   to fill empty slots if necessary.
	  
	  @param name The name of the item to be dropped into the inventory.
	  @param amount The amount of the item to be dropped into the inventory.
	  @return The amount that could not be deposited into the inventory, due to there being insufficient space.
	 */
	UFUNCTION(BlueprintCallable)
		FInventoryInputResponse DropItemIntoInventory(FName name, int32 amount);

	/*
		Attempts to swap inventory slot items with inventory referenced. Will cancel the swap if items attempt to enter a slot with an incompatable restriction. (Example, placing a resource in an armour slot).

		@param secondInventory A reference to the other inventory involved in swapping.
		@param myIndex This object's index to swap
		@param otherIndex secondInventory's index to swap with.
	*/
	UFUNCTION(BlueprintCallable)
		void SwapInventorySlotsWithOtherInventory(UInventoryComponent* secondInventory, int32 myIndex, int32 otherIndex);

	/**
		Attempts to add the specified amount to the inventory slot at the given index,
		disregarding the type of item or slot restrictions.
		@param index The index of the inventory slot where the item(s) will be added.
		@param amount The amount of the item to be added to the inventory slot.
		@return The amount leftover that could not be inserted due to max stack size.
 */
	UFUNCTION(BlueprintCallable)
		int32 DropItemIntoInventoryByIndex(int32 index, int32 amount);

	/**
	Attempts to insert the specified inventory slot at the index. Will only complete if the slot at the index is empty. It will cancel the insertion otherwise.

	@param slotDetails The index of the inventory slot where the item(s) will be added.
	@param amount The amount of the item to be added to the inventory slot.
	@return The amount leftover that could not be inserted due to max stack size.
*/
	UFUNCTION(BlueprintCallable)
		void InsertItemIntoInventoryByIndex(FInventorySlot slotDetails, int32 index);

	/**
		Facilitates a transaction between two stacks of the same type.

		The source inventory slot attempts to add items into the destination inventory slot
		until the destination reaches its maximum stack amount or until the source runs out.
		The source will become an empty stack if it transfers all its items.

		@param sourceInventory Pointer to the source inventory component.
		@param transferInventorySlot The inventory slot containing the items to transfer.
		@param index The index of the destination inventory slot for the transaction.
		@param sourceIndex The index of the source inventory slot for the transaction.
		@return The amount that couldn't be transferred into the destination inventory slot.
*/
	UFUNCTION(BlueprintCallable)
		int32 CombineStacks(UInventoryComponent* sourceInventory, FInventorySlot transferInventorySlot, int32 index, int32 sourceIndex);

	/**
		Transfers the inventory slot from the source inventory to an empty slot in this inventory.
		
		Does not overwrite non-empty slots in the destination inventory. The function attempts to merge
		items from the transfer inventory slot into an available empty slot in this inventory.
	
		@param sourceInventory Pointer to the source inventory component.
		@param transferInventorySlot The inventory slot containing the items to transfer.
		@param Toindex The index of the destination inventory slot in this inventory.
		@param FromIndex The index of the source inventory slot in the source inventory.
		@return The amount leftover that could not be inserted into the destination index due to max stack size.
 */
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

	/*
		@return the FItemMetadata row in metadata table if it exists. Otherwise, returns a default FItemMetaData
	*/
	FItemMetadata GetItemMetadata(FName);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	static void CheckIndexValidity(TArray<FInventorySlot>* arr, int32 index);

		
};
