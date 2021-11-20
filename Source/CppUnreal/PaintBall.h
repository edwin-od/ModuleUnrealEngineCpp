// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaintBall.generated.h"

class UProjectileMovementComponent;

UCLASS()
class CPPUNREAL_API APaintBall : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APaintBall();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	UPROPERTY()
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float ProjectileSpeed = 1000.0f;

	UPROPERTY()
	FLinearColor PaintColor;

	UPROPERTY()
	UProjectileMovementComponent* Projectile;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
