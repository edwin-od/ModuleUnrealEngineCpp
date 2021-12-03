// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGameCpp.h"
#include "Kismet/GameplayStatics.h"

USaveGameCpp::USaveGameCpp()
{
	SaveSlotName = TEXT("TestSaveSlot");
	UserIndex = 0;
}