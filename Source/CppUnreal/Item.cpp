// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"

#include "Engine/DataTable.h"
#include "CppUnrealCharacter.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	TriggerBox->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	TriggerBox->SetCollisionProfileName("Trigger");

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);

	RootComponent = TriggerBox;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item_SM"));
	StaticMeshComponent->SetWorldScale3D(FVector(0.25f, 0.25f, 0.25f));
	StaticMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	StaticMeshComponent->SetSimulatePhysics(false);
	StaticMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	StaticMeshComponent->SetupAttachment(RootComponent);

}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetActorRotation(GetActorRotation() + FRotator(0.0f, DeltaTime * SpinSpeed, 0.0f));
}

void AItem::OnOverlapBegin(UPrimitiveComponent* OverlappedCompo, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACppUnrealCharacter* Character = Cast<ACppUnrealCharacter>(OtherActor);

	if (!Character || OtherComp->ComponentHasTag(FName("Weapon")))
		return;
	else if (!ItemDataTable)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("You did not set the Items DataTable in the Item's properties!"));
		return;
	}

	if (ItemType == EItemsListing::None)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("You did not set the Item's Type in the Item's properties!"));
		return;
	}

	PickupItem(ItemType, Character);
}

FItemsTableStruct AItem::GetFromItemsDataTable(EItemsListing Type)
{
	if (!ItemDataTable)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("You did not set the Items DataTable in the Item's properties!"));
		return FItemsTableStruct();
	}
	if (Type == EItemsListing::None)
		return FItemsTableStruct();

	TArray<FItemsTableStruct*> OutAllRows;
	FString Context;
	ItemDataTable->GetAllRows<FItemsTableStruct>(Context, OutAllRows);
	for (FItemsTableStruct* ItemInfoRow : OutAllRows)
	{
		if (ItemInfoRow->ItemEnumListing == Type)
			return *ItemInfoRow;
	}

	return FItemsTableStruct();
}

void AItem::PickupItem(EItemsListing Type, class ACppUnrealCharacter* Character)
{
	FItemsTableStruct Item = GetFromItemsDataTable(Type);
	if (Item.ItemEnumListing != EItemsListing::None)
	{
		if (Character->PickupItem(Item, this) && !Item.CanBeStored)
			Destroy();
	}
}