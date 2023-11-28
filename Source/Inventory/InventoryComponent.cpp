// Copyright of Daniel Ballem 2023


#include "InventoryComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


FInventoryInputResponse UInventoryComponent::DropItemIntoInventory(FName name, int32 amount)
{
	int originalAmount = amount;
	AddToStacksByName(name, name, amount);
	if (amount == 0)
		return FInventoryInputResponse(0, true);
	AddToStacksByName(FName(ItemConstants::NO_ITEM_STRING), name, amount);
	
	if (amount == originalAmount)
		return FInventoryInputResponse(amount, false);
	return FInventoryInputResponse(amount, true);
}

void UInventoryComponent::SwapInventorySlotsWithOtherInventory(UInventoryComponent* secondInventory, int32 myIndex, int32 otherIndex)
{
	CheckIndexValidity(&secondInventory->InventorySlots, otherIndex);
	CheckIndexValidity(&InventorySlots, myIndex);

	//should swap IF:
	//restriction of myself -> restriction of the other.

	if (InventorySlots[myIndex].DoesItemSlotTypeMatchRestriction(secondInventory->InventorySlots[otherIndex].ItemMetadata.type) && secondInventory->InventorySlots[otherIndex].DoesItemSlotTypeMatchRestriction(InventorySlots[myIndex].ItemMetadata.type))
	{
		ItemType otherRestriction = secondInventory->InventorySlots[otherIndex].ItemSlotTypeRestriction;
		ItemType mySlotRestriction = InventorySlots[myIndex].ItemSlotTypeRestriction;

		FInventorySlot slot1 = secondInventory->InventorySlots[otherIndex];
		secondInventory->InventorySlots[otherIndex] = InventorySlots[myIndex];
		InventorySlots[myIndex] = slot1;

		//preserve slot restrictions
		secondInventory->InventorySlots[otherIndex].ItemSlotTypeRestriction = otherRestriction;
		InventorySlots[myIndex].ItemSlotTypeRestriction = mySlotRestriction;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot swap slots with incompatible item types."));
		return; // Prevent swapping if item types are incompatible
	}
}

//returns amount left over
int32 UInventoryComponent::DropItemIntoInventoryByIndex(int32 index, int32 amount)
{
	FInventorySlot& slot = InventorySlots[index];
	FInventoryInputResponse response = slot.AddToStack(amount);
	return response.AmountLeft;
}

void UInventoryComponent::InsertItemIntoInventoryByIndex(FInventorySlot slotDetails, int32 index)
{
	if (InventorySlots[index].ItemName == ItemConstants::NO_ITEM_STRING && InventorySlots[index].DoesItemSlotTypeMatchRestriction(slotDetails.ItemMetadata.type))
	{
		InventorySlots[index] = slotDetails;
	}
}

int32 UInventoryComponent::CombineStacks(UInventoryComponent* sourceInventory, FInventorySlot transferInventorySlot, int32 index, int32 sourceIndex)
{
	if (!InventorySlots[index].ItemName.IsEqual(sourceInventory->InventorySlots[sourceIndex].ItemName))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot combine stacks of different types"))
		return 0; //TODO throw exception or log error.
	}

	int32 amount = transferInventorySlot.CurrentAmount;
	int32 leftover = DropItemIntoInventoryByIndex(index, amount);
	int32 difference = transferInventorySlot.CurrentAmount - leftover;
	int32 leftoverOther = sourceInventory->DropItemIntoInventoryByIndex(sourceIndex, -difference);
	return leftoverOther;
}

int32 UInventoryComponent::TransferAndMergeToEmptyStack(UInventoryComponent* sourceInventory, FInventorySlot transferInventorySlot, int32 Toindex, int32 FromIndex)
{
	if (!InventorySlots[Toindex].ItemName.ToString().Equals(ItemConstants::NO_ITEM_STRING))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Transfer to non-empty stack"));
		return 0;
	}

	int32 amount = transferInventorySlot.CurrentAmount;

	//cancels transfer if item slot restriction won't allow it.
	if (!InventorySlots[Toindex].DoesItemSlotTypeMatchRestriction(transferInventorySlot.ItemMetadata.type))
		return amount;

	//insert inventory slot data.
	InsertItemIntoInventoryByIndex(transferInventorySlot, Toindex);

	//officially remove transfered amount from source.
	int32 leftover = sourceInventory->DropItemIntoInventoryByIndex(FromIndex, -transferInventorySlot.CurrentAmount);
	return leftover;
}

/*
	Iterates through all slots with the name `slotToInsert`, and tries to insert `itemName` into it.
	Sets item metadata as inserting into inventories is the way they are instantiated.
*/
void UInventoryComponent::AddToStacksByName(FName slotToInsert, FName itemName, int32 &outAmount)
{
	for (int32 i = 0; i < InventoryColSize * InventoryRowSize; i++)
	{
		FInventorySlot& slot = InventorySlots[i];
		FString slotName = slot.ItemName.ToString();
		if (slotName.Equals(slotToInsert.ToString()))
		{

			//inserting into an empty slot, populate item data.
			if (slotToInsert.ToString().Equals(ItemConstants::NO_ITEM_STRING))
			{
				FItemMetadata itemMetadata = GetItemMetadata(itemName);

				slot.ItemMetadata = itemMetadata;

				slot.ItemName = itemName;
			}
			FInventoryInputResponse response = slot.AddToStack(outAmount);
			outAmount = response.AmountLeft;

			if (outAmount == 0)
				return;
		}
	}
}

FItemMetadata UInventoryComponent::GetItemMetadata(FName itemName)
{
	FItemMetadata* row = nullptr;

	if (ItemTable)
		row = ItemTable->FindRow<FItemMetadata>(itemName, FString(""));
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No Item Table set."));
		return FItemMetadata();
	}

	if (row)
		return *row;
	else
		UE_LOG(LogTemp, Warning, TEXT("No item row found for %s"), *itemName.ToString());

	return FItemMetadata();
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	InventorySlots.Init(FInventorySlot(), InventoryColSize * InventoryRowSize);
	Super::BeginPlay();
	// ...
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInventoryComponent::CheckIndexValidity(TArray<FInventorySlot>* arr, int32 index) {
	if (!arr)
		UE_LOG(LogTemp, Fatal, TEXT("Inventory Array is null"));
	if (index < 0 || index >= arr->Num())
		UE_LOG(LogTemp, Fatal, TEXT("Index %d can't go in array of size %d"), index, arr->Num());
}