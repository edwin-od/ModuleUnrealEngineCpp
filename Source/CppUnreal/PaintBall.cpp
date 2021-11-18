// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintBall.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

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

	StaticMesh->SetWorldScale3D(FVector(0.25f, 0.25f, 0.25f));
	StaticMesh->SetSimulatePhysics(true);
	StaticMesh->OnComponentHit.AddDynamic(this, &APaintBall::OnHit);
	StaticMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	StaticMesh->SetNotifyRigidBodyCollision(true);

	RootComponent = StaticMesh;

	Projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("PaintBall_Projectile"));

	Projectile->MaxSpeed = 3000.0f;
	Projectile->InitialSpeed = 3000.0f;
}

// Called when the game starts or when spawned
void APaintBall::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APaintBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APaintBall::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);

	if (Character)
	{
		Destroy();
		return;
	}

	Destroy();
}
