// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimMontage.h"

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

private:
	
	UPROPERTY()
	bool bIsDead;


	UFUNCTION()
	void PRINT(FString str);

	UFUNCTION()
	void Die();

	UFUNCTION()
	void WeaponBeginOverlap(UPrimitiveComponent* OverlappedCompo, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void WeaponEndOverlap(UPrimitiveComponent* OverlappedCompo, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnAnimationEnded(UAnimMontage* Montage, bool bInterrupted);

	FOnMontageEnded AnimEndDelegate;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
	UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* EquippedItemStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UBoxComponent* EquippedItemTriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxHP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 HP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Damage = -10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHit;



	UFUNCTION(BlueprintCallable)
	void ChangeHP(int ChangeValue);

	UFUNCTION()
	void SetHP(int Value);

	UFUNCTION(BlueprintCallable)
	void Attack(class ACppUnrealCharacter* Enemy);

	UFUNCTION()
	void ApplyDamage(int32 Amount, FVector Direction);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class ACppUnrealCharacter* EnemyWeaponOverlapActor;

};
