// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintDecal.h"
#include "Components/DecalComponent.h"

APaintDecal::APaintDecal()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	UDecalComponent* DecalComponent = FindComponentByClass<UDecalComponent>();

	DecalComponent->DecalSize = FVector(15.0f, 15.0f, 15.0f);
	DecalComponent->FadeStartDelay = 3.0f;
	DecalComponent->FadeDuration = 3.0f;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("Material'/Game/ThirdPersonCPP/MISC/PaintDecalMaterial'"));
	if (Material.Succeeded())
	{
		DecalInstanceMaterial = UMaterialInstanceDynamic::Create(Material.Object, DecalComponent, "PaintDecal_Instance_MAT");
		if(DecalInstanceMaterial)
			DecalComponent->SetDecalMaterial(DecalInstanceMaterial);
	}
}

// Called when the game starts or when spawned
void APaintDecal::BeginPlay()
{
	Super::BeginPlay();
}

void APaintDecal::SetDecalColor(FLinearColor PaintColor)
{
	if(DecalInstanceMaterial)
		DecalInstanceMaterial->SetVectorParameterValue("PaintColor", PaintColor);
}