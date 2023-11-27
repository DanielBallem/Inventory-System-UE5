// Fill out your copyright notice in the Description page of Project Settings.

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
	RESOURCE = 0 UMETA(DisplayName = "RESOURCE")
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
		type = ItemType::RESOURCE;
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

	FInventorySlot() {
		ItemName = FName(ItemConstants::NO_ITEM_STRING);
		CurrentAmount = 0;
		ItemMetadata = FItemMetadata();
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
		void SwapInventorySlots(int32 index1, int32 index2);

	UFUNCTION(BlueprintCallable)
		void SwapInventorySlotsWithOtherInventory(UInventoryComponent* other, int32 otherIndex, int32 myIndex);

	UFUNCTION(BlueprintCallable)
		int32 DropItemIntoInventoryByIndex(int32 index, int32 amount);

	UFUNCTION(BlueprintCallable)
		void InsertItemIntoInventoryByIndex(FInventorySlot slotDetails, int32 index);

	UFUNCTION(BlueprintCallable)
		int32 CombineStacks(UInventoryComponent* other, FInventorySlot slotDetails, int32 index, int32 otherIndex);

	UFUNCTION(BlueprintCallable)
		int32 TransferAndMergeStack(UInventoryComponent* other, FInventorySlot slotDetails, int32 index, int32 otherIndex);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 InventoryColSize = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		int32 InventoryRowSize = 5;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void AddToStacksByName(FName slotToInsert, FName itemName, int32 &outAmount);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
