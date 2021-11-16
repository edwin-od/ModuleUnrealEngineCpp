// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintBall.h"
#include "GameFramework/Character.h"

// Sets default values
APaintBall::APaintBall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere'"));

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PaintBall_SM"));

	if (CubeMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(CubeMesh.Object);
	}

	StaticMesh->SetSimulatePhysics(true);
	StaticMesh->OnComponentHit.AddDynamic(this, &APaintBall::OnHit);

	RootComponent = StaticMesh;
}

// Called when the game starts or when spawned
void APaintBall::BeginPlay()
{
	Super::BeginPlay();
	GLog->Log("Hello World");
}

// Called every frame
void APaintBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APaintBall::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	GLog->Log(OtherActor->GetName());
	ACharacter* Character = Cast<ACharacter>(OtherActor);

	if (Character)
	{
		// Destroy ball
		return;
	}
}
