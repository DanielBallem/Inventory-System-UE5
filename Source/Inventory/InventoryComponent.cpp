// Copyright of Daniel Ballem 2023
#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UInventoryComponent::DropItemIntoInventory(FName name, int32 amount)
{
	AddToStacksByName(name, name, amount);
	
	if (amount == 0)
		return 0;

	AddToStacksByName(FName(ItemConstants::NO_ITEM_STRING), name, amount);

	return amount;
}

void UInventoryComponent::SwapInventorySlotsWithOtherInventory(UInventoryComponent* secondInventory, int32 myIndex, int32 otherIndex)
{
	CheckIndexValidity(&secondInventory->InventorySlots, otherIndex);
	CheckIndexValidity(&InventorySlots, myIndex);

	//should swap if both items are swapping into compatable slots. 
	if (AreSlotsCompatibleForSwapping(myIndex, secondInventory, otherIndex))
		SwapCompatibleInventorySlots(secondInventory, otherIndex, myIndex);
	else
		UE_LOG(LogTemp, Warning, TEXT("Cannot swap slots with incompatible item types."));
	
}

bool UInventoryComponent::AreSlotsCompatibleForSwapping(const int32& myIndex, UInventoryComponent* secondInventory, const int32& otherIndex)
{
	return InventorySlots[myIndex].DoesItemSlotTypeMatchRestriction(secondInventory->InventorySlots[otherIndex].ItemMetadata.type) && secondInventory->InventorySlots[otherIndex].DoesItemSlotTypeMatchRestriction(InventorySlots[myIndex].ItemMetadata.type);
}

void UInventoryComponent::SwapCompatibleInventorySlots(UInventoryComponent* secondInventory, const int32& otherIndex, const int32& myIndex)
{
	//preserving restrictions of slots while swapping everything else.
	ItemType otherRestriction = secondInventory->InventorySlots[otherIndex].ItemSlotTypeRestriction;
	ItemType mySlotRestriction = InventorySlots[myIndex].ItemSlotTypeRestriction;

	FInventorySlot slot1 = secondInventory->InventorySlots[otherIndex];
	secondInventory->InventorySlots[otherIndex] = InventorySlots[myIndex];
	InventorySlots[myIndex] = slot1;

	//preserve slot restrictions
	secondInventory->InventorySlots[otherIndex].ItemSlotTypeRestriction = otherRestriction;
	InventorySlots[myIndex].ItemSlotTypeRestriction = mySlotRestriction;
}

//returns amount left over
int32 UInventoryComponent::DropItemIntoInventoryByIndex(int32 index, int32 amount)
{
	FInventorySlot& slot = InventorySlots[index];
	return slot.AddToStack(amount);
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
		return 0;
	}

	int32 amount = transferInventorySlot.CurrentAmount;
	int32 leftover = DropItemIntoInventoryByIndex(index, amount);
	int32 difference = transferInventorySlot.CurrentAmount - leftover;
	int32 leftoverOther = sourceInventory->DropItemIntoInventoryByIndex(sourceIndex, -difference);
	return leftoverOther;
}

int32 UInventoryComponent::TransferAndMergeToEmptyStack(UInventoryComponent* sourceInventory, FInventorySlot transferInventorySlot, int32 toIndex, int32 fromIndex)
{
	if (!InventorySlots[toIndex].ItemName.ToString().Equals(ItemConstants::NO_ITEM_STRING))
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Transfer to non-empty stack"));
		return 0;
	}

	int32 amount = transferInventorySlot.CurrentAmount;

	//cancels transfer if item slot restriction won't allow it.
	if (!InventorySlots[toIndex].DoesItemSlotTypeMatchRestriction(transferInventorySlot.ItemMetadata.type))
		return amount;

	//insert inventory slot data.
	InsertItemIntoInventoryByIndex(transferInventorySlot, toIndex);

	//officially remove transfered amount from source.
	int32 leftover = sourceInventory->DropItemIntoInventoryByIndex(fromIndex, -transferInventorySlot.CurrentAmount);
	return leftover;
}

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

			outAmount = slot.AddToStack(outAmount);

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
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInventoryComponent::CheckIndexValidity(TArray<FInventorySlot>* arr, int32 index) {
	if (!arr)
		UE_LOG(LogTemp, Fatal, TEXT("Inventory Array is null"));
	if (index < 0 || index >= arr->Num())
		UE_LOG(LogTemp, Fatal, TEXT("Index %d can't go in array of size %d"), index, arr->Num());
}