// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

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
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	int MinHP = 0;

	UPROPERTY(EditAnywhere)
	int MaxHP = 100;

	UPROPERTY(EditAnywhere)
	float RespawnDelay = 3.0f;

	UPROPERTY(EditAnywhere)
	float SpawnEffectDuration = 1.15f;

	UPROPERTY(EditAnywhere)
	float GrabDistance = 5000.0f;

	UPROPERTY(EditAnywhere)
	float IdleAnimationTimout = 5.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bIdleAnimationTimedOut;

	UPROPERTY()
	TSubclassOf<UUserWidget> PauseMenuWidget;

	UPROPERTY()
	FTimerHandle IdleAnimationTimerHandle;

	UPROPERTY()
	bool bIsDead;

	UPROPERTY(VisibleAnywhere)
	UMaterialInterface* SpawnEffectMaterial;

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

	UPROPERTY()
	int HP;

	UFUNCTION()
	void PRINT(FString str);

	UFUNCTION()
	void GrabItem();

	UFUNCTION()
	void ShootPaintBall();

	UFUNCTION()
	void CrouchAction();

	UFUNCTION()
	void SetHP(int Value);

	UFUNCTION()
	void ChangeHP(int ChangeValue);

	UFUNCTION()
	void Die();

	UFUNCTION()
	void Respawn();

	UFUNCTION()
	void AnyKeyPressed();

	UFUNCTION()
	void AnyKeyReleased();

	UFUNCTION()
	void IdleAnimationTimedOut();

	UFUNCTION()
	void PauseGame();

	UFUNCTION(BlueprintCallable)
	bool SaveGame(int32 SlotIndex, FString SlotName);

	UFUNCTION(BlueprintCallable)
	bool LoadGame(int32 SlotIndex, FString SlotName);



};

