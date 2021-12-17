// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ItemsListing.h"
#include "ItemsTableStruct.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS()
class CPPUNREAL_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AItem();

public:	

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere)
	class UDataTable* ItemDataTable;

	UPROPERTY(EditAnywhere)
	EItemsListing ItemType;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UBoxComponent* TriggerBox;

	UPROPERTY(EditAnywhere)
	float SpinSpeed = 50.0f;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedCompo, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	FItemsTableStruct GetFromItemsDataTable(EItemsListing Type);

	UFUNCTION()
	void PickupItem(EItemsListing Type, class ACppUnrealCharacter* Character);

};
