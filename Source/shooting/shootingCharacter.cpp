// Copyright Epic Games, Inc. All Rights Reserved.

#include "shootingCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Aweaponbase.h"
#include "TimerManager.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AshootingCharacter::AshootingCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(30.0f, 0.0f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetActive(false);
}

void AshootingCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 在蓝图里按名字找摄像机
	FirstPersonCamera = Cast<UCameraComponent>(GetDefaultSubobjectByName(TEXT("FirstPersonCamera")));
	if (!FirstPersonCamera)
	{
		// 如果默认子对象里没有，就遍历组件找
		TArray<UCameraComponent*> Cameras;
		GetComponents<UCameraComponent>(Cameras);
		for (UCameraComponent* Cam : Cameras)
		{
			if (Cam && Cam->GetFName() == TEXT("FirstPersonCamera"))
			{
				FirstPersonCamera = Cam;
				break;
			}
		}
	}

	if (FirstPersonCamera)
	{
		FirstPersonCamera->SetActive(false); // 默认关闭第一人称
		UE_LOG(LogTemp, Warning, TEXT("找到第一人称相机！"));
	}
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
}

// ========== 生成武器 ==========
void AshootingCharacter::SpawnWeapon()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	// 动态加载 BP_weapon 蓝图
	UClass* WeaponBPClass = LoadClass<AAweaponbase>(nullptr, *WeaponBlueprintPath);

	if (WeaponBPClass)
	{
		AAweaponbase* NewWeapon = GetWorld()->SpawnActor<AAweaponbase>(
			WeaponBPClass,
			GetActorLocation(),
			GetActorRotation(),
			SpawnParams
		);

		if (NewWeapon)
		{
			NewWeapon->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				WeaponSocketName
			);
			CurrentWeapon = NewWeapon;
			NewWeapon->ResetAmmo();
			UE_LOG(LogTemp, Warning, TEXT("武器生成成功！"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("武器蓝图加载失败：%s"), *WeaponBlueprintPath);
	}
}

void AshootingCharacter::PickupWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("=== E pressed ==="));

	// 遍历场景里所有武器
	TArray<AActor*> FoundWeapons;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAweaponbase::StaticClass(), FoundWeapons);

	AAweaponbase* ClosestWeapon = nullptr;
	float ClosestDist = 500.0f; // 最大捡取距离（厘米）

	for (AActor* Actor : FoundWeapons)
	{
		AAweaponbase* Weapon = Cast<AAweaponbase>(Actor);
		if (Weapon && Weapon != CurrentWeapon)
		{
			float Dist = FVector::Dist(GetActorLocation(), Weapon->GetActorLocation());
			if (Dist < ClosestDist)
			{
				ClosestDist = Dist;
				ClosestWeapon = Weapon;
			}
		}
	}

	if (ClosestWeapon)
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->Destroy();
		}

		CurrentWeapon = ClosestWeapon;
		CurrentWeapon->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			WeaponSocketName
		);
		CurrentWeapon->ResetAmmo();
		UE_LOG(LogTemp, Warning, TEXT("=== Weapon picked up! ==="));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("=== No weapon in range ==="));
	}
}

// ========== 输入绑定 ==========
void AshootingCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AshootingCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AshootingCharacter::Look);
		EnhancedInputComponent->BindAction(ToggleViewAction, ETriggerEvent::Started, this, &AshootingCharacter::ToggleView);

		// 开火
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AshootingCharacter::StartFire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AshootingCharacter::StopFire);

		// 换弹
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AshootingCharacter::ReloadWeapon);

		// 拾取
		EnhancedInputComponent->BindAction(PickupAction, ETriggerEvent::Started, this, &AshootingCharacter::PickupWeapon);

		//技能
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AshootingCharacter::Dash);
	}
}

void AshootingCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AshootingCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// ========== 开火逻辑 ==========
void AshootingCharacter::StartFire()
{
	bIsFiring = true;
	FireWeapon();
}

void AshootingCharacter::StopFire()
{
	bIsFiring = false;
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
}

void AshootingCharacter::FireWeapon()
{
	if (bIsFiring && CurrentWeapon)
	{
		CurrentWeapon->Fire();
		AddControllerPitchInput(-CurrentWeapon->RecoilPitch);

		// 用定时器连发，间隔 = 1/射速
		float Interval = 1.0f / CurrentWeapon->FireRate;
		GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AshootingCharacter::FireWeapon, Interval, false);
	}
}	

void AshootingCharacter::ReloadWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
	}
}

void AshootingCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 每帧同步给动画蓝图
	bHasWeapon = (CurrentWeapon != nullptr);
	UpdateHUD();
}
void AshootingCharacter::ToggleView()
{
	if (!FirstPersonCamera) return;

	bIsFirstPerson = !bIsFirstPerson;

	if (bIsFirstPerson)
	{
		FirstPersonCamera->SetActive(true);
		FollowCamera->SetActive(false);
		CameraBoom->SetActive(false);
		UE_LOG(LogTemp, Warning, TEXT("Switched to FirstPerson"));
	}
	else
	{
		FirstPersonCamera->SetActive(false);
		FollowCamera->SetActive(true);
		CameraBoom->SetActive(true);
		UE_LOG(LogTemp, Warning, TEXT("Switched to ThirdPerson"));
	}
}
float AshootingCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Health -= DamageAmount;
	UE_LOG(LogTemp, Warning, TEXT("Player HP: %f"), Health);

	if (Health <= 0.0f)
	{
		Die();
	}

	return DamageAmount;
}

void AshootingCharacter::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("Player died!"));

	// 禁用输入
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}
}
void AshootingCharacter::UpdateHUD()
{
	if (!HUDWidget) return;

	// 更新血条
	UProgressBar* HealthBar = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(TEXT("HealthBar")));
	if (HealthBar)
	{
		HealthBar->SetPercent(Health / MaxHealth);
	}

	// 更新弹药文本
	UTextBlock* AmmoText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("AmmoText")));
	if (AmmoText && CurrentWeapon)
	{
		FString AmmoStr = FString::Printf(TEXT("Ammo: %d / %d"), CurrentWeapon->CurrentAmmo, CurrentWeapon->MaxAmmo);
		AmmoText->SetText(FText::FromString(AmmoStr));
	}
	// 更新 Dash 冷却
	UTextBlock* DashText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("DashText")));
	if (DashText)
	{
		if (bCanDash)
		{
			DashText->SetText(FText::FromString(TEXT("DASH: Ready")));
			DashText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
		}
		else
		{
			DashText->SetText(FText::FromString(TEXT("DASH: Cooldown")));
			DashText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		}
	}
}
void AshootingCharacter::Dash()
{
	if (!bCanDash) return;

	bCanDash = false;

	FVector DashOffset = GetActorForwardVector() * DashDistance;
	SetActorLocation(GetActorLocation() + DashOffset);
	UE_LOG(LogTemp, Warning, TEXT("Dash!"));

	// 冷却
	GetWorldTimerManager().SetTimer(DashTimerHandle, [this]()
		{
			bCanDash = true;
			UE_LOG(LogTemp, Warning, TEXT("Dash ready!"));
		}, DashCooldown, false);
}