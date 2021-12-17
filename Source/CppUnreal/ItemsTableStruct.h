// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PaperSprite.h"
#include "ItemsListing.h"

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemsTableStruct.generated.h"

USTRUCT(BlueprintType)
struct FItemsTableStruct : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	FItemsTableStruct() : ItemName(FName(TEXT("None"))), ItemEnumListing(EItemsListing::None), ItemValue(0), ItemDuration(0.0f), ItemDescription(), CanBeStored(false), IsVolatile(false), ItemIcon() {  }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Item)
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	EItemsListing  ItemEnumListing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	int32 ItemValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	float ItemDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	FString  ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	bool  CanBeStored;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	bool  IsVolatile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Item)
	TSoftObjectPtr<class UTexture2D> ItemIcon;

};
