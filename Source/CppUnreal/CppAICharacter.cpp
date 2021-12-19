// Fill out your copyright notice in the Description page of Project Settings.


#include "CppAICharacter.h"

#include "CppUnrealCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ACppAICharacter::ACppAICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	EquippedItemStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item_SM"));
	EquippedItemStaticMeshComponent->SetSimulatePhysics(false);
	EquippedItemStaticMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	EquippedItemStaticMeshComponent->SetGenerateOverlapEvents(true);
	EquippedItemStaticMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "PrimaryWeapon");

	EquippedItemTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackTriggerBoxComponent"));
	EquippedItemTriggerBox->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	EquippedItemTriggerBox->SetCollisionProfileName("Trigger");
	EquippedItemTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACppAICharacter::WeaponBeginOverlap);
	EquippedItemTriggerBox->OnComponentEndOverlap.AddDynamic(this, &ACppAICharacter::WeaponEndOverlap);
	EquippedItemTriggerBox->SetupAttachment(EquippedItemStaticMeshComponent);

	HP = MaxHP;
	bIsDead = false;
	bIsHit = false;
	EnemyWeaponOverlapActor = nullptr;
}

// Called when the game starts or when spawned
void ACppAICharacter::BeginPlay()
{
	Super::BeginPlay();

	AnimEndDelegate.BindUObject(this, &ACppAICharacter::OnAnimationEnded);
	
}

// Called every frame
void ACppAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACppAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACppAICharacter::PRINT(FString str)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, str);
}

void ACppAICharacter::SetHP(int Value)
{
	if (!bIsDead)
	{
		HP = FMath::Clamp(Value, MinHP, MaxHP);

		PRINT("AI HP : " + FString::FromInt(HP));

		if (HP == MinHP)
			Die();
	}
}

void ACppAICharacter::ChangeHP(int ChangeValue)
{
	SetHP(HP + ChangeValue);
}

void ACppAICharacter::Die()
{
	GetMesh()->SetSimulatePhysics(true);
	GetCharacterMovement()->DisableMovement();
	FindComponentByClass<UCapsuleComponent>()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	bIsDead = true;
}

void ACppAICharacter::Attack(ACppUnrealCharacter* Enemy)
{
	if (Enemy)
		Enemy->ApplyDamage(Damage, GetActorForwardVector());
}

void ACppAICharacter::WeaponBeginOverlap(UPrimitiveComponent* OverlappedCompo, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACppUnrealCharacter* Enemy = Cast<ACppUnrealCharacter>(OtherActor);

	if (!Enemy)
		return;

	EnemyWeaponOverlapActor = Enemy;
}

void ACppAICharacter::WeaponEndOverlap(UPrimitiveComponent* OverlappedCompo, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACppUnrealCharacter* Enemy = Cast<ACppUnrealCharacter>(OtherActor);

	if (!Enemy)
		return;

	EnemyWeaponOverlapActor = nullptr;
}

void ACppAICharacter::ApplyDamage(int32 Amount, FVector Direction)
{
	float Angle = 180.0f - FMath::Abs(FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GetActorForwardVector(), Direction) / (GetActorForwardVector().Size() * Direction.Size()))));

	if (HitMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(HitMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		GetMesh()->GetAnimInstance()->Montage_JumpToSection(Angle > 90.0f ? "Back" : "Front", HitMontage);
		GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(AnimEndDelegate);

		bIsHit = true;
	}

	ChangeHP(Amount);
}

void ACppAICharacter::OnAnimationEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == HitMontage)
		bIsHit = false;
}