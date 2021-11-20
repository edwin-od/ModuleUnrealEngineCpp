// Fill out your copyright notice in the Description page of Project Settings.


#include "HPChangeTrigger.h"
#include "CppUnrealCharacter.h"
#include "Engine/PointLight.h"

AHPChangeTrigger::AHPChangeTrigger()
{
    
}

void AHPChangeTrigger::BeginPlay()
{
    Super::BeginPlay();

    OnActorBeginOverlap.AddDynamic(this, &AHPChangeTrigger::OnOverlapBegin);
    OnActorEndOverlap.AddDynamic(this, &AHPChangeTrigger::OnOverlapEnd);

    if (PointLight)
    {
        if (ChangeValue > 0)
            PointLight->SetLightColor(FLinearColor(FVector4(0.0f, 1.0f, 0.0f, 1.0f)));
        else if (ChangeValue < 0)
            PointLight->SetLightColor(FLinearColor(FVector4(1.0f, 0.0f, 0.0f, 1.0f)));
        else
            PointLight->SetLightColor(FLinearColor(FVector4(1.0f, 1.0f, 1.0f, 1.0f)));
    }
}

void AHPChangeTrigger::OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor)
{
    if (OtherActor && (OtherActor != this)) {
        ACppUnrealCharacter* Character = Cast<ACppUnrealCharacter>(OtherActor);

        if (!Character)
            return;

        ChangeCharacterHP(Character);

        FTimerDelegate ChangeHPTimerDelegate;
        ChangeHPTimerDelegate.BindUFunction(this, FName("ChangeCharacterHP"), Character);
        GetWorldTimerManager().SetTimer(ChangeHPTimerHandle, ChangeHPTimerDelegate, ChangeHPTimerValue, true);
    }
}

void AHPChangeTrigger::OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor)
{
    if (OtherActor && (OtherActor != this)) {
        ACppUnrealCharacter* Character = Cast<ACppUnrealCharacter>(OtherActor);

        if (!Character)
            return;
        
        GetWorldTimerManager().ClearTimer(ChangeHPTimerHandle);
    }
}

void AHPChangeTrigger::ChangeCharacterHP(ACppUnrealCharacter* Character)
{
    Character->ChangeHP(ChangeValue);
}