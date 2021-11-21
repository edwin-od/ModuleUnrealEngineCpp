// Fill out your copyright notice in the Description page of Project Settings.


#include "PaintBall.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "PaintDecal.h"

// Sets default values
APaintBall::APaintBall()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere'"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PaintBall_SM"));

	if (!CubeMesh.Succeeded() || !StaticMesh)
		return;

	StaticMesh->SetStaticMesh(CubeMesh.Object);
	StaticMesh->SetWorldScale3D(FVector(0.25f, 0.25f, 0.25f));
	StaticMesh->SetSimulatePhysics(true);
	StaticMesh->OnComponentHit.AddDynamic(this, &APaintBall::OnHit);
	StaticMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	StaticMesh->SetNotifyRigidBodyCollision(true);

	PaintColor = FLinearColor(FVector4(FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), FMath::RandRange(0.0f, 1.0f), 1.0f));

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("Material'/Game/ThirdPersonCPP/MISC/PainBallMaterial'"));
	if (Material.Succeeded())
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(Material.Object, StaticMesh, "PaintBall_Instance_MAT");
		Mat->SetVectorParameterValue("PaintColor", PaintColor);

		StaticMesh->SetMaterial(0, Mat);
	}

	RootComponent = StaticMesh;

	Projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("PaintBall_Projectile"));

	Projectile->MaxSpeed = ProjectileSpeed;
	Projectile->InitialSpeed = ProjectileSpeed;
}

// Called when the game starts or when spawned
void APaintBall::BeginPlay()
{
	Super::BeginPlay();
}


void APaintBall::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);

	if (Character)
	{
		Destroy();
		return;
	}

	APaintDecal* DecalInstance = GetWorld()->SpawnActor<APaintDecal>(Hit.ImpactPoint, Hit.ImpactNormal.Rotation() + FRotator(90.0, 0.0, 0.0));
	DecalInstance->SetDecalColor(PaintColor);

	Destroy();
}
