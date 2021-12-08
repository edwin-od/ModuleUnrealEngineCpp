// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemsTableStruct.generated.h"

USTRUCT(BlueprintType)
struct FItemsTableStruct : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	FItemsTableStruct() : ItemName(FName(TEXT("NONE"))), ItemIcon() {  }
	FItemsTableStruct(FName Name, TSoftObjectPtr<class UTexture> Icon) : ItemName(Name), ItemIcon(Icon) {  }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Item)
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Item)
	TSoftObjectPtr<class UTexture> ItemIcon;
};
