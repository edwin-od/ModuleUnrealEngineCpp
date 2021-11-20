// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DecalActor.h"
#include "PaintDecal.generated.h"

/**
 * 
 */
UCLASS()
class CPPUNREAL_API APaintDecal : public ADecalActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	APaintDecal();

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DecalInstanceMaterial;

	UFUNCTION()
	void SetDecalColor(FLinearColor PaintColor);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
