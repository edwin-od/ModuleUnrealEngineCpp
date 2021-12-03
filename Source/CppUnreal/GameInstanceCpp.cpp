// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstanceCpp.h"

UGameInstanceCpp::UGameInstanceCpp()
{
	ResetSaveData();
}

void UGameInstanceCpp::SetSaveData(int32 SlotIndex, FString SlotName)
{
	SaveSlotIndex = SlotIndex;
	SaveSlotName = SlotName;
}

void UGameInstanceCpp::ResetSaveData()
{
	SaveSlotIndex = -1;
	SaveSlotName = "";
}