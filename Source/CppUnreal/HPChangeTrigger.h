// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "HPChangeTrigger.generated.h"

UCLASS()
class CPPUNREAL_API AHPChangeTrigger : public ATriggerBox
{
	GENERATED_BODY()
	
protected:

	virtual void BeginPlay() override;

public:

	AHPChangeTrigger();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChangeHPTimerValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ChangeValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class APointLight* PointLight;

	UPROPERTY()
	FTimerHandle ChangeHPTimerHandle;

	UFUNCTION()
	void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

	UFUNCTION()
	void OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor);

	UFUNCTION()
	void ChangeCharacterHP(class ACppUnrealCharacter* Character);

};
