// Copyright Epic Games, Inc. All Rights Reserved.

#include "CppUnrealCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "PaintBall.h"
#include "CppUnrealGameMode.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Blueprint/UserWidget.h"

//////////////////////////////////////////////////////////////////////////
// ACppUnrealCharacter

ACppUnrealCharacter::ACppUnrealCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	ShootLocation = CreateDefaultSubobject<USceneComponent>(TEXT("PaintBall_Shoot_Location"));
	ShootLocation->SetRelativeLocation(FVector(100.0f, 0.0f, 100.0f));
	ShootLocation->SetRelativeRotation(FRotator(15.0f, 0.0f, 0.0f));
	ShootLocation->SetupAttachment(RootComponent);

	GrabLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Item_Grab_Location"));
	GrabLocation->SetRelativeLocation(FVector(150.0f, 0.0f, 0.0f));
	GrabLocation->SetupAttachment(RootComponent);

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	HP = MaxHP;
	bIsDead = false;
	ItemGrabbed = nullptr;
	bIdleAnimationTimedOut = false;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("Material'/Game/ThirdPersonCPP/MISC/SpawnEffect'"));
	SpawnEffectMaterial = Material.Succeeded() ? Material.Object : nullptr;

	static ConstructorHelpers::FClassFinder<UUserWidget> Widget(TEXT("WidgetBlueprint'/Game/ThirdPersonCPP/UI/PauseMenu'"));
	PauseMenuWidget = Widget.Succeeded() ? Widget.Class : nullptr;
}

// Called when the game starts or when spawned
void ACppUnrealCharacter::BeginPlay()
{
	Super::BeginPlay();

	BodyInstanceMaterial = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(0), GetMesh(), "Body_Instance_MAT");
	GetMesh()->SetMaterial(0, BodyInstanceMaterial);

	HeadInstanceMaterial = UMaterialInstanceDynamic::Create(GetMesh()->GetMaterial(1), GetMesh(), "Head_Instance_MAT");
	GetMesh()->SetMaterial(1, HeadInstanceMaterial);

	if (SpawnEffectMaterial)
	{
		SpawnEffectInstanceMaterial = UMaterialInstanceDynamic::Create(SpawnEffectMaterial, GetMesh(), "SpawnEffect_Instance_MAT");
		SpawnEffectInstanceMaterial->SetScalarParameterValue("Time", 0.0f);

		GetMesh()->SetMaterial(0, SpawnEffectInstanceMaterial);
		GetMesh()->SetMaterial(1, SpawnEffectInstanceMaterial);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACppUnrealCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACppUnrealCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACppUnrealCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACppUnrealCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACppUnrealCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACppUnrealCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ACppUnrealCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ACppUnrealCharacter::OnResetVR);

	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &ACppUnrealCharacter::ShootPaintBall);
	PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &ACppUnrealCharacter::GrabItem);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ACppUnrealCharacter::CrouchAction);
	PlayerInputComponent->BindAction("Any", IE_Pressed, this, &ACppUnrealCharacter::AnyKeyPressed);
	PlayerInputComponent->BindAction("Any", IE_Released, this, &ACppUnrealCharacter::AnyKeyReleased);
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &ACppUnrealCharacter::PauseGame);
}


void ACppUnrealCharacter::OnResetVR()
{
	// If CppUnreal is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in CppUnreal.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ACppUnrealCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ACppUnrealCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ACppUnrealCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACppUnrealCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACppUnrealCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ACppUnrealCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

// Called every frame
void ACppUnrealCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (GetMesh()->GetMaterial(0) == SpawnEffectInstanceMaterial)
	{
		float time = 0;
		SpawnEffectInstanceMaterial->GetScalarParameterValue(FName(TEXT("Time")), time);
		time += DeltaTime / 2;
		SpawnEffectInstanceMaterial->SetScalarParameterValue("Time", time);
		if (time >= SpawnEffectDuration)
		{
			GetMesh()->SetMaterial(0, BodyInstanceMaterial);
			GetMesh()->SetMaterial(1, HeadInstanceMaterial);
		}
	}

	if (ItemGrabbed)
		PhysicsHandle->SetTargetLocationAndRotation(GrabLocation->GetComponentLocation(), GrabLocation->GetComponentRotation());

	// Tourner avec la camera
	SetActorRotation(((FVector::CrossProduct(GetFollowCamera()->GetForwardVector(), FVector(0.0f, 0.0f, 1.0f))).Rotation() + FRotator(0.0f, 90.0f, 0.0f)));
}

void ACppUnrealCharacter::PRINT(FString str)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, str);
}

void ACppUnrealCharacter::ShootPaintBall()
{
	GetWorld()->SpawnActor<APaintBall>(ShootLocation->GetComponentLocation(), ShootLocation->GetComponentRotation());
}

void ACppUnrealCharacter::SetHP(int Value)
{
	if (!bIsDead)
	{
		HP = FMath::Clamp(Value, MinHP, MaxHP);

		if (HP == MinHP)
			Die();

		PRINT("HP " + FString::FromInt(HP));
	}
}

void ACppUnrealCharacter::ChangeHP(int ChangeValue)
{
	SetHP(HP + ChangeValue);
}

void ACppUnrealCharacter::Die()
{
	GetMesh()->SetSimulatePhysics(true);
	GetCharacterMovement()->DisableMovement();

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ACppUnrealCharacter::Respawn, RespawnDelay, false);

	bIsDead = true;
}

void ACppUnrealCharacter::Respawn()
{
	AController* ControllerReference = GetController();

	Destroy();
	
	ACppUnrealGameMode* GameMode = Cast<ACppUnrealGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->RestartPlayer(ControllerReference);
		bIsDead = false;
	}

	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
}

void ACppUnrealCharacter::GrabItem()
{
	if (!ItemGrabbed)
	{
		FHitResult Hit(ForceInit);
		FVector start = GetMesh()->GetBoneLocation("Head", EBoneSpaces::WorldSpace);
		FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(GetFollowCamera()->GetComponentLocation(), start);
		FVector End = start + (Direction * GrabDistance);
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		GetWorld()->LineTraceSingleByChannel(Hit, start, End, ECC_PhysicsBody, CollisionParams);

		if (Hit.GetActor() && Hit.GetActor()->ActorHasTag("Item"))
		{
			PhysicsHandle->GrabComponentAtLocationWithRotation(Hit.GetComponent(), "", Hit.GetComponent()->GetComponentLocation(), Hit.GetComponent()->GetComponentRotation());
			ItemGrabbed = Hit.GetActor();
		}
	}
	else
	{
		PhysicsHandle->ReleaseComponent();
		ItemGrabbed = nullptr;
	}
}

void ACppUnrealCharacter::CrouchAction()
{
	if (GetCharacterMovement()->IsCrouching())
		GetCharacterMovement()->bWantsToCrouch = false;
		//GetCharacterMovement()->UnCrouch();
	else
		GetCharacterMovement()->bWantsToCrouch = true;
		//GetCharacterMovement()->Crouch();
}

void ACppUnrealCharacter::AnyKeyPressed()
{
	bIdleAnimationTimedOut = false;
}

void ACppUnrealCharacter::AnyKeyReleased()
{
	GetWorldTimerManager().SetTimer(IdleAnimationTimerHandle, this, &ACppUnrealCharacter::IdleAnimationTimedOut, IdleAnimationTimout, false);
}

void ACppUnrealCharacter::IdleAnimationTimedOut()
{
	bIdleAnimationTimedOut = true;
	GetWorldTimerManager().ClearTimer(IdleAnimationTimerHandle);
}

void ACppUnrealCharacter::PauseGame()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		bool bIsPaused = PlayerController->SetPause(true);

		if (bIsPaused)
		{
			PlayerController->bShowMouseCursor = true;
			PlayerController->bEnableClickEvents = true;
			PlayerController->bEnableMouseOverEvents = true;

			if (PauseMenuWidget)
			{
				UUserWidget* PauseMenuWidgetInstance = CreateWidget<UUserWidget>(GetGameInstance(), PauseMenuWidget);
				PauseMenuWidgetInstance->AddToViewport();
			}
		}
	}
}