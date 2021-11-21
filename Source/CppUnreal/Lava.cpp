// Fill out your copyright notice in the Description page of Project Settings.


#include "Lava.h"
//#include "GameFramework/Character.h"
#include "CppUnrealCharacter.h"

// Sets default values
ALava::ALava()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("StaticMesh'/Game/Geometry/Meshes/1M_Cube'"));

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lava_SM"));

	if (!CubeMesh.Succeeded() || !StaticMesh)
		return;

	StaticMesh->SetStaticMesh(CubeMesh.Object);
	StaticMesh->OnComponentHit.AddDynamic(this, &ALava::OnHit);

	RootComponent = StaticMesh;
}

// Called when the game starts or when spawned
void ALava::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALava::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACppUnrealCharacter* Character = Cast<ACppUnrealCharacter>(OtherActor);

	if (!Character)
		return;

	Character->SetHP(Character->MinHP);
}
