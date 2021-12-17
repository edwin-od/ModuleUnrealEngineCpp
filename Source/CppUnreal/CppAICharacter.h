// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CppAICharacter.generated.h"

UCLASS()
class CPPUNREAL_API ACppAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACppAICharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* EquippedItemStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UBoxComponent* EquippedItemTriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 HP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxHP = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Damage = -10;

	UPROPERTY()
	bool bIsDead;

	UFUNCTION(BlueprintCallable)
	void ChangeHP(int ChangeValue);

	UFUNCTION()
	void SetHP(int Value);

	UFUNCTION()
	void Die();

	UFUNCTION(BlueprintCallable)
	void Attack(class ACppUnrealCharacter* Enemy);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class ACppUnrealCharacter* EnemyWeaponOverlapActor;

	UFUNCTION()
	void WeaponBeginOverlap(UPrimitiveComponent* OverlappedCompo, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void WeaponEndOverlap(UPrimitiveComponent* OverlappedCompo, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
