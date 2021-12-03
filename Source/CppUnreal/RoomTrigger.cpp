// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomTrigger.h"
#include "CppUnrealCharacter.h"
#include "Kismet/GameplayStatics.h"

ARoomTrigger::ARoomTrigger()
{

}

void ARoomTrigger::BeginPlay()
{
    Super::BeginPlay();

    OnActorBeginOverlap.AddDynamic(this, &ARoomTrigger::OnOverlapBegin);
    OnActorEndOverlap.AddDynamic(this, &ARoomTrigger::OnOverlapEnd);
}

void ARoomTrigger::OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor)
{
    if (OtherActor && (OtherActor != this)) {
        ACppUnrealCharacter* Character = Cast<ACppUnrealCharacter>(OtherActor);

        if (!Character || LevelToLoad == "")
            return;

        FLatentActionInfo LatentInfo;
        UGameplayStatics::LoadStreamLevel(this, LevelToLoad, true, false, LatentInfo);
    }
}

void ARoomTrigger::OnOverlapEnd(class AActor* OverlappedActor, class AActor* OtherActor)
{
    if (OtherActor && (OtherActor != this)) {
        ACppUnrealCharacter* Character = Cast<ACppUnrealCharacter>(OtherActor);

        if (!Character || LevelToLoad == "")
            return;

        FLatentActionInfo LatentInfo;
        UGameplayStatics::UnloadStreamLevel(this, LevelToLoad, LatentInfo, false);
    }
}