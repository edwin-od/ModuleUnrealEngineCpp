// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ItemsTableStruct.h"
#include "Animation/AnimMontage.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CppUnrealCharacter.generated.h"

UCLASS(config = Game)
class ACppUnrealCharacter : public ACharacter
{
	GENERATED_BODY()

		/** Camera boom positioning the camera behind the character */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;
public:
	ACppUnrealCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	// End of APawn interface

	// Called when the game starts or when spawned
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

private:
	
	UPROPERTY()
	bool bIsDead;

	UPROPERTY()
	UMaterialInstanceDynamic* SpawnEffectInstanceMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* HeadInstanceMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* BodyInstanceMaterial;

	UPROPERTY()
	TArray<UMaterialInterface*> MeshBaseMaterials;

	UPROPERTY()
	USceneComponent* ShootLocation;

	UPROPERTY()
	USceneComponent* GrabLocation;

	UPROPERTY()
	AActor* ItemGrabbed;

	UPROPERTY()
	class UPhysicsHandleComponent* PhysicsHandle;

	UPROPERTY()
	FTimerHandle RespawnTimerHandle;

	UPROPERTY(EditAnywhere)
	class UDataTable* ItemDataTable;

	UPROPERTY()
	FTimerHandle ArmorPotionTimerHandle;

	UPROPERTY()
	bool bIsArmored;

	UPROPERTY()
	FTimerHandle DamagePotionTimerHandle;

	UPROPERTY()
	float DamageBoost;

	UPROPERTY()
	TSubclassOf<UUserWidget> PauseMenuWidget;

	UPROPERTY()
	TSubclassOf<UUserWidget> InventoryMenuWidget;

	UPROPERTY()
	FTimerHandle IdleAnimationTimerHandle;

	UPROPERTY()
	UMaterialInterface* SpawnEffectMaterial;

	UPROPERTY()
	bool bIsAttacking;

	UPROPERTY()
	int32 EquippedLeftHand;

	UPROPERTY()
	int32 EquippedRightHand;
	
	UPROPERTY()
	bool bIsBlocking;

	UPROPERTY()
	bool bPressedBlocking;


	UFUNCTION()
	void PRINT(FString str);

	UFUNCTION()
	void GrabItem();

	UFUNCTION()
	void ShootPaintBall();

	UFUNCTION()
	void CrouchAction();

	UFUNCTION()
	void Die();

	UFUNCTION()
	void Respawn();

	UFUNCTION(BlueprintCallable)
	void AnyKeyPressed();

	UFUNCTION(BlueprintCallable)
	void AnyKeyReleased();

	UFUNCTION()
	void IdleAnimationTimedOut();

	UFUNCTION()
	void PauseGame();

	UFUNCTION()
	void OpenInventory();

	UFUNCTION()
	void AddItemToInventory(int32 Index, FItemsTableStruct Item, class AItem* ItemInstance);

	UFUNCTION()
	bool DropItemVolatile(int32 Index);

	UFUNCTION()
	void RemoveItemFromInventory(int32 Index);

	UFUNCTION()
	void WeaponBeginOverlap(UPrimitiveComponent* OverlappedCompo, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void WeaponEndOverlap(UPrimitiveComponent* OverlappedCompo, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void ResetArmorPotionTimer();

	UFUNCTION()
	void ResetDamagePotionTimer();

	UFUNCTION()
	void InputBlockPressed();

	UFUNCTION()
	void InputBlockReleased();

	UFUNCTION()
	void InputAttack();

	UFUNCTION()
	void OnAnimationEnded(UAnimMontage* Montage, bool bInterrupted);

	FOnMontageEnded AnimEndDelegate;





public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ShieldBlockMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* SwordSlashMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PrimaryItemStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UBoxComponent* PrimaryItemTriggerBox;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* SecondaryItemStaticMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MinHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MaxHP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int HP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MinStamina = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int MaxStamina = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StaminaRegainRate = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StaminaBlockingLoseRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int StaminaAttackLoseAmount = 25;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int StaminaHitLoseAmount = 15;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Stamina;

	UPROPERTY(EditAnywhere)
	float RespawnDelay = 3.0f;

	UPROPERTY(EditAnywhere)
	float SpawnEffectDuration = 1.15f;

	UPROPERTY(EditAnywhere)
	float GrabDistance = 5000.0f;

	UPROPERTY(EditAnywhere)
	float IdleAnimationTimout = 15.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FItemsTableStruct> Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<class AItem*> InventoryInstances;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Coins;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIdleAnimationTimedOut;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bInventoryFullFlag;




	UFUNCTION()
	void SetHP(int Value);

	UFUNCTION()
	void ChangeHP(int ChangeValue);

	UFUNCTION()
	bool PickupItem(FItemsTableStruct Item, class AItem* ItemInstance);

	UFUNCTION(BlueprintCallable)
	bool SaveGame(int32 SlotIndex, FString SlotName);

	UFUNCTION(BlueprintCallable)
	bool LoadGame(int32 SlotIndex, FString SlotName);

	UFUNCTION(BlueprintCallable)
	bool DropItem(int32 Index);

	UFUNCTION(BlueprintCallable)
	bool EquipItem(int32 Index);

	UFUNCTION(BlueprintCallable)
	void UnequipItem(int32 Index);

	UFUNCTION(BlueprintCallable)
	FItemsTableStruct GetItem(int32 Index);

	UFUNCTION(BlueprintCallable)
	void AddCoins(int32 Amount);

	UFUNCTION(BlueprintCallable)
	int32 GetCoins();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class ACppAICharacter* EnemyWeaponOverlapActor;

	UFUNCTION(BlueprintCallable)
	void Attack(class ACppAICharacter* Enemy);

	UFUNCTION()
	void ApplyDamage(int32 Amount, FVector Direction);

};

