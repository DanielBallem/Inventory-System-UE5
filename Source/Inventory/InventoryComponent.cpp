// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	InventorySlots.Init(FInventorySlot(), InventoryColSize * InventoryRowSize);


	// ...
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

void UInventoryComponent::SwapInventorySlots(int32 index1, int32 index2)
{
	if (index1 >= InventorySlots.Num() || index2 >= InventorySlots.Num()) return;
	if (index1 < 0 || index2 < 0) return;

	FInventorySlot slot1 = InventorySlots[index1];
	InventorySlots[index1] = InventorySlots[index2];
	InventorySlots[index2] = slot1;
}

void UInventoryComponent::SwapInventorySlotsWithOtherInventory(UInventoryComponent* other, int32 otherIndex, int32 myIndex)
{
	if (otherIndex >= other->InventorySlots.Num() || myIndex >= InventorySlots.Num()) return;
	if (otherIndex < 0 || myIndex < 0) return;

	FInventorySlot slot1 = other->InventorySlots[otherIndex];
	other->InventorySlots[otherIndex] = InventorySlots[myIndex];
	InventorySlots[myIndex] = slot1;
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
	if (InventorySlots[index].ItemName == ItemConstants::NO_ITEM_STRING)
	{
		InventorySlots[index] = slotDetails;
	}
}

/*
	TODO: Use hash map to store locations of shared items so it doesn't need to traverse the whole inventory.
*/
void UInventoryComponent::AddToStacksByName(FName slotToInsert, FName itemName, int32 &outAmount)
{
	for (int32 i = 0; i < InventoryColSize * InventoryRowSize; i++)
	{
		FInventorySlot& slot = InventorySlots[i];
		FString slotName = slot.ItemName.ToString();
		if (slotName.Equals(slotToInsert.ToString()))
		{
			slot.ItemName = itemName; //in the case where it's empty. Doesn't matter otherwise
			FInventoryInputResponse response = slot.AddToStack(outAmount);
			outAmount = response.AmountLeft;
			if (outAmount == 0)
				return;
		}
	}
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

