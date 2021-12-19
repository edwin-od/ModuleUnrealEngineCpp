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
#include "SaveGameCpp.h"
#include "GameInstanceCpp.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "ItemsListing.h"
#include "Item.h"
#include "CppAICharacter.h"
#include "Components/BoxComponent.h"

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

	PrimaryItemTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackTriggerBoxComponent"));
	PrimaryItemTriggerBox->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	PrimaryItemTriggerBox->SetCollisionProfileName("Trigger");
	PrimaryItemTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACppUnrealCharacter::WeaponBeginOverlap);
	PrimaryItemTriggerBox->OnComponentEndOverlap.AddDynamic(this, &ACppUnrealCharacter::WeaponEndOverlap);
	PrimaryItemTriggerBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "PrimaryWeapon");

	PrimaryItemStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Primary_Weapon_SM"));
	PrimaryItemStaticMeshComponent->SetSimulatePhysics(false);
	PrimaryItemStaticMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	PrimaryItemStaticMeshComponent->SetGenerateOverlapEvents(true);
	PrimaryItemStaticMeshComponent->SetupAttachment(PrimaryItemTriggerBox);

	GetMesh()->SetRenderCustomDepth(false);
	GetMesh()->SetCustomDepthStencilValue(0);
	PrimaryItemStaticMeshComponent->SetRenderCustomDepth(false);
	PrimaryItemStaticMeshComponent->SetCustomDepthStencilValue(0);

	SecondaryItemStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Secondary_Weapon_SM"));
	SecondaryItemStaticMeshComponent->SetSimulatePhysics(false);
	SecondaryItemStaticMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SecondaryItemStaticMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "SecondaryWeapon");

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	HP = MaxHP;
	Coins = 0;
	bIsDead = false;
	ItemGrabbed = nullptr;
	bIdleAnimationTimedOut = false;
	EquippedLeftHand = -1;
	EquippedRightHand = -1;
	bIsBlocking = false;
	bIsArmored = false;
	DamageBoost = 0.0f;
	bIsAttacking = false;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("Material'/Game/ThirdPersonCPP/MISC/SpawnEffect'"));
	SpawnEffectMaterial = Material.Succeeded() ? Material.Object : nullptr;

	static ConstructorHelpers::FClassFinder<UUserWidget> PauseWidget(TEXT("WidgetBlueprint'/Game/ThirdPersonCPP/UI/PauseMenu'"));
	PauseMenuWidget = PauseWidget.Succeeded() ? PauseWidget.Class : nullptr;

	static ConstructorHelpers::FClassFinder<UUserWidget> InventoryWidget(TEXT("WidgetBlueprint'/Game/ThirdPersonCPP/UI/InventoryMenu'"));
	InventoryMenuWidget = InventoryWidget.Succeeded() ? InventoryWidget.Class : nullptr;
}

// Called when the game starts or when spawned
void ACppUnrealCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Load Saved Game
	UGameInstanceCpp* GameInstance = Cast<UGameInstanceCpp>(GetGameInstance());
	if (GameInstance && GameInstance->SaveSlotIndex != -1 && GameInstance->SaveSlotName != "")
		LoadGame(GameInstance->SaveSlotIndex, GameInstance->SaveSlotName);

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

	bIdleAnimationTimedOut = false;
	GetWorldTimerManager().ClearTimer(IdleAnimationTimerHandle);
	GetWorldTimerManager().SetTimer(IdleAnimationTimerHandle, this, &ACppUnrealCharacter::IdleAnimationTimedOut, IdleAnimationTimout, false);

	Inventory.Init(FItemsTableStruct(), 4);
	InventoryInstances.Init(nullptr, 4);

	AnimEndDelegate.BindUObject(this, &ACppUnrealCharacter::OnAnimationEnded);
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
	PlayerInputComponent->BindAction("Pause", IE_Released, this, &ACppUnrealCharacter::PauseGame);
	PlayerInputComponent->BindAction("Inventory", IE_Released, this, &ACppUnrealCharacter::OpenInventory);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ACppUnrealCharacter::InputAttack);
	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &ACppUnrealCharacter::InputBlockPressed);
	PlayerInputComponent->BindAction("Block", IE_Released, this, &ACppUnrealCharacter::InputBlockReleased);
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

		PRINT("HP : " + FString::FromInt(HP));
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
	GetMesh()->SetGenerateOverlapEvents(false);

	for (int i = 0; i < 4; i++)
	{
		if (Inventory[i].ItemEnumListing == EItemsListing::None)
			break;
		RemoveItemFromInventory(i);
		Inventory[i] = FItemsTableStruct();
		InventoryInstances[i] = nullptr;
	}

	PrimaryItemStaticMeshComponent->SetStaticMesh(nullptr);
	EquippedRightHand = -1;

	SecondaryItemStaticMeshComponent->SetStaticMesh(nullptr);
	EquippedLeftHand = -1;

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
	GetWorldTimerManager().ClearTimer(IdleAnimationTimerHandle);
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
	if (!bIsDead)
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
}

bool ACppUnrealCharacter::SaveGame(int32 SlotIndex, FString SlotName)
{
	USaveGameCpp* SaveGameInstance = Cast<USaveGameCpp>(UGameplayStatics::CreateSaveGameObject(USaveGameCpp::StaticClass()));
	if (SaveGameInstance)
	{
		SaveGameInstance->UserIndex = SlotIndex;
		SaveGameInstance->SaveSlotName = SlotName;
		SaveGameInstance->PlayerLocation = GetActorLocation() + FVector(0, 0, 5);
		SaveGameInstance->PlayerHP = HP;

		if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->UserIndex))
			return true;
	}
	return false;
}

bool ACppUnrealCharacter::LoadGame(int32 SlotIndex, FString SlotName)
{
	USaveGameCpp* SaveGameInstance = Cast<USaveGameCpp>(UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex));
	if (SaveGameInstance)
	{
		SetActorLocation(SaveGameInstance->PlayerLocation);
		HP = SaveGameInstance->PlayerHP;

		return true;
	}
	return false;
}

void ACppUnrealCharacter::OpenInventory()
{
	if (!bIsDead)
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

				if (InventoryMenuWidget)
				{
					UUserWidget* InventoryMenuWidgetInstance = CreateWidget<UUserWidget>(GetGameInstance(), InventoryMenuWidget);
					InventoryMenuWidgetInstance->AddToViewport();
				}
			}
		}
	}
}

bool ACppUnrealCharacter::PickupItem(FItemsTableStruct Item, AItem* ItemInstance)
{
	if (Item.ItemEnumListing == EItemsListing::None)
		return false;

	if (Item.CanBeStored)
	{
		if (Inventory[3].ItemEnumListing == EItemsListing::None)	// Not full inventory
		{
			for (int i = 3; i > 0; i--)
			{
				if (Inventory[i - 1].ItemEnumListing == EItemsListing::None)
					continue;
				AddItemToInventory(i, Item, ItemInstance);
				if (!Item.IsVolatile)
					EquipItem(i);
				return true;
			}
			AddItemToInventory(0, Item, ItemInstance);
			if (!Item.IsVolatile)
				EquipItem(0);
			return true;
		}
		else
			PRINT("Inventory Full !");
	}
	else
	{
		switch (Item.ItemEnumListing)
		{
		case EItemsListing::Coin:
			AddCoins(Item.ItemValue);
			return true;
		default:
			break;
		}
	}

	return false;
}

void ACppUnrealCharacter::AddItemToInventory(int32 Index, FItemsTableStruct Item, class AItem* ItemInstance)
{
	Inventory[Index] = Item;
	InventoryInstances[Index] = ItemInstance;

	ItemInstance->SetActorHiddenInGame(true);
	ItemInstance->SetActorTickEnabled(false);
	ItemInstance->SetActorEnableCollision(false);
}

bool ACppUnrealCharacter::DropItem(int32 Index)
{
	if (Index > 3 || Index < 0)
		return false;

	if (Inventory[Index].ItemEnumListing != EItemsListing::None)
	{
		RemoveItemFromInventory(Index);
		if (EquippedLeftHand == Index || EquippedRightHand == Index)
			UnequipItem(Index);
		for (int i = Index; i < 3; i++)
		{
			if (Inventory[i + 1].ItemEnumListing != EItemsListing::None)
			{
				if (EquippedLeftHand == i + 1)
					EquippedLeftHand = i;
				else if (EquippedRightHand == i + 1)
					EquippedRightHand = i;
			}

			Inventory[i] = Inventory[i + 1];
			InventoryInstances[i] = InventoryInstances[i + 1];

			if (Inventory[i + 1].ItemEnumListing == EItemsListing::None)
				break;
		}
		if (Inventory[3].ItemEnumListing != EItemsListing::None)
		{
			Inventory[3] = FItemsTableStruct();
			InventoryInstances[3] = nullptr;
		}
		return true;
	}

	return false;
}

bool ACppUnrealCharacter::DropItemVolatile(int32 Index)
{
	if (Index > 3 || Index < 0)
		return false;

	if (Inventory[Index].ItemEnumListing != EItemsListing::None)
	{
		if (InventoryInstances[Index])
		{
			InventoryInstances[Index]->Destroy();
			InventoryInstances[Index] = nullptr;
		}
		for (int i = Index; i < 3; i++)
		{
			if (Inventory[i + 1].ItemEnumListing != EItemsListing::None)
			{
				if (EquippedLeftHand == i + 1)
					EquippedLeftHand = i;
				else if (EquippedRightHand == i + 1)
					EquippedRightHand = i;
			}

			Inventory[i] = Inventory[i + 1];
			InventoryInstances[i] = InventoryInstances[i + 1];

			if (Inventory[i + 1].ItemEnumListing == EItemsListing::None)
				break;
		}
		if (Inventory[3].ItemEnumListing != EItemsListing::None)
		{
			Inventory[3] = FItemsTableStruct();
			InventoryInstances[3] = nullptr;
		}
		return true;
	}

	return false;
}

void ACppUnrealCharacter::RemoveItemFromInventory(int32 Index)
{
	InventoryInstances[Index]->SetActorHiddenInGame(false);
	InventoryInstances[Index]->SetActorTickEnabled(true);
	InventoryInstances[Index]->SetActorEnableCollision(true);
	InventoryInstances[Index]->SetActorLocation(GrabLocation->GetComponentLocation());
}

void ACppUnrealCharacter::BindEquipItem(int32 Index)
{
	EquipItem(Index);
}

bool ACppUnrealCharacter::EquipItem(int32 Index)
{
	FItemsTableStruct Item = GetItem(Index);
	if (Item.ItemEnumListing == EItemsListing::None)
		return false;

	switch (Item.ItemEnumListing)
	{
	case EItemsListing::Sword:
	{
		if (EquippedRightHand != -1)
			UnequipItem(EquippedRightHand);
		EquippedRightHand = Index;
		UStaticMeshComponent* ItemSM = InventoryInstances[Index]->FindComponentByClass<UStaticMeshComponent>();
		PrimaryItemStaticMeshComponent->SetStaticMesh(ItemSM->GetStaticMesh());
		PrimaryItemStaticMeshComponent->SetRelativeLocation(FVector(-3.0f, 5.0f, -10.0f));
		PrimaryItemStaticMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
		PrimaryItemStaticMeshComponent->SetRelativeScale3D(FVector(1.6f, 1.75f, 2.0f));
		return true;
	}
	case EItemsListing::Shield:
	{
		if (EquippedLeftHand != -1)
			UnequipItem(EquippedLeftHand);
		EquippedLeftHand = Index;
		UStaticMeshComponent* ItemSM = InventoryInstances[Index]->FindComponentByClass<UStaticMeshComponent>();
		SecondaryItemStaticMeshComponent->SetStaticMesh(ItemSM->GetStaticMesh());
		SecondaryItemStaticMeshComponent->SetRelativeLocation(FVector(8.0f, 11.0f, -33.0f));
		SecondaryItemStaticMeshComponent->SetRelativeRotation(FRotator(-23.0f, -150.2f, 12.5f));
		SecondaryItemStaticMeshComponent->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
		return true;
	}
	case EItemsListing::Map:
		// open map

		return true;
	case EItemsListing::HealthPotion:
		ChangeHP(Item.ItemValue);
		DropItemVolatile(Index);
		return true;
	case EItemsListing::ArmorPotion:
		ResetArmorPotionTimer();
		bIsArmored = true;
		GetMesh()->SetCustomDepthStencilValue(1);
		GetMesh()->SetRenderCustomDepth(true);
		DropItemVolatile(Index);
		GetWorldTimerManager().SetTimer(ArmorPotionTimerHandle, this, &ACppUnrealCharacter::ResetArmorPotionTimer, Item.ItemDuration, false);
		return true;
	case EItemsListing::DamagePotion:
		ResetDamagePotionTimer();
		DamageBoost = Item.ItemValue;
		PrimaryItemStaticMeshComponent->SetCustomDepthStencilValue(2);
		PrimaryItemStaticMeshComponent->SetRenderCustomDepth(true);
		DropItemVolatile(Index);
		GetWorldTimerManager().SetTimer(DamagePotionTimerHandle, this, &ACppUnrealCharacter::ResetDamagePotionTimer, Item.ItemDuration, false);
		return true;
	default:
		break;
	}

	return false;
}

void ACppUnrealCharacter::UnequipItem(int32 Index)
{
	FItemsTableStruct Item = GetItem(Index);
	if (Item.ItemEnumListing == EItemsListing::None)
		return;
	if (EquippedRightHand == Index)
	{
		PrimaryItemStaticMeshComponent->SetStaticMesh(nullptr);
		EquippedRightHand = -1;
	}
	else if (EquippedLeftHand == Index)
	{
		SecondaryItemStaticMeshComponent->SetStaticMesh(nullptr);
		EquippedLeftHand = -1;
	}
}

FItemsTableStruct ACppUnrealCharacter::GetItem(int32 Index)
{
	if (Index > 3 || Index < 0)
		return FItemsTableStruct();

	return Inventory[Index];
}

void ACppUnrealCharacter::AddCoins(int32 Amount)
{
	Coins += Amount;
	if (Coins < 0)
		Coins = 0;

	PRINT("Coins : " + FString::FromInt(Coins));
}

int32 ACppUnrealCharacter::GetCoins()
{
	return Coins;
}

void ACppUnrealCharacter::WeaponBeginOverlap(UPrimitiveComponent* OverlappedCompo, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACppAICharacter* AIEnemy = Cast<ACppAICharacter>(OtherActor);

	if (!AIEnemy)
		return;

	EnemyWeaponOverlapActor = AIEnemy;
}

void ACppUnrealCharacter::Attack(ACppAICharacter* Enemy)
{
	if(Enemy)
		Enemy->ApplyDamage(-FMath::Abs(GetItem(EquippedRightHand).ItemValue + DamageBoost), GetActorForwardVector());
}

void ACppUnrealCharacter::WeaponEndOverlap(UPrimitiveComponent* OverlappedCompo, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACppAICharacter* AIEnemy = Cast<ACppAICharacter>(OtherActor);

	if (!AIEnemy)
		return;

	EnemyWeaponOverlapActor = nullptr;
}

void ACppUnrealCharacter::ApplyDamage(int32 Amount, FVector Direction)
{
	if (bIsArmored)
		return;

	float Angle = 180.0f - FMath::Abs(FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GetActorForwardVector(), Direction) / (GetActorForwardVector().Size() * Direction.Size()))));

	if (bIsBlocking && Angle < 75.0f) // updated in BP
		return;

	if(HitMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(HitMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		GetMesh()->GetAnimInstance()->Montage_JumpToSection(Angle > 90.0f ? "Back" : "Front", HitMontage);
	}

	ChangeHP(Amount);
}

void ACppUnrealCharacter::ResetArmorPotionTimer()
{
	bIsArmored = false;
	GetMesh()->SetRenderCustomDepth(false);
	GetMesh()->SetCustomDepthStencilValue(0);
	GetWorldTimerManager().ClearTimer(ArmorPotionTimerHandle);
}

void ACppUnrealCharacter::ResetDamagePotionTimer()
{
	DamageBoost = 0.0f;
	PrimaryItemStaticMeshComponent->SetRenderCustomDepth(false);
	PrimaryItemStaticMeshComponent->SetCustomDepthStencilValue(0);
	GetWorldTimerManager().ClearTimer(DamagePotionTimerHandle);
}

void ACppUnrealCharacter::InputBlockPressed()
{
	if(EquippedLeftHand != -1 && ShieldBlockMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(ShieldBlockMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		GetMesh()->GetAnimInstance()->Montage_JumpToSection("Start", ShieldBlockMontage);
		GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(AnimEndDelegate);

		bIsBlocking = true;
	}
}

void ACppUnrealCharacter::InputBlockReleased()
{
	if (EquippedLeftHand != -1 && bIsBlocking)
	{
		if(ShieldBlockMontage)
		{
			GetMesh()->GetAnimInstance()->Montage_Play(ShieldBlockMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
			GetMesh()->GetAnimInstance()->Montage_JumpToSection("End", ShieldBlockMontage);
		}

		bIsBlocking = false;
	}
}


void ACppUnrealCharacter::InputAttack()
{
	if (!bIsAttacking && EquippedRightHand != -1 && SwordSlashMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(SwordSlashMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		GetMesh()->GetAnimInstance()->Montage_JumpToSection("Start", SwordSlashMontage);
		GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(AnimEndDelegate);

		bIsAttacking = true;
	}
}

void ACppUnrealCharacter::OnAnimationEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if(Montage == ShieldBlockMontage && bInterrupted)
		bIsBlocking = false;
	else if(Montage == SwordSlashMontage)
		bIsAttacking = false;
}