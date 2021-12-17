// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EItemsListing : uint8 {
    None = 0  UMETA(DisplayName = "None"),
    Sword = 1 UMETA(DisplayName = "Sword"),
    Shield = 2 UMETA(DisplayName = "Shield"),
    Map = 3 UMETA(DisplayName = "Map"),
    HealthPotion = 4 UMETA(DisplayName = "HealthPotion"),
    ArmorPotion = 5 UMETA(DisplayName = "ArmorPotion"),
    DamagePotion = 6 UMETA(DisplayName = "DamagePotion"),
    Coin = 7 UMETA(DisplayName = "Coin")
};