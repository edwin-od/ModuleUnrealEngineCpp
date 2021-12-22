// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameInstanceCpp.generated.h"

/**
 * 
 */
UCLASS()
class CPPUNREAL_API UGameInstanceCpp : public UGameInstance
{
	GENERATED_BODY()
	
public:

	UGameInstanceCpp();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bDontPlayIntro = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 SaveSlotIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString SaveSlotName;

	UFUNCTION(BlueprintCallable)
	void SetSaveData(int32 SlotIndex, FString SlotName);

	UFUNCTION(BlueprintCallable)
	void ResetSaveData();
};
