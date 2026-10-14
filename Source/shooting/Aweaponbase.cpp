// Fill out your copyright notice in the Description page of Project Settings.

#include "Aweaponbase.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "ActorPool.h"

AAweaponbase::AAweaponbase()
{
	PrimaryActorTick.bCanEverTick = false;
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
}

void AAweaponbase::BeginPlay()
{
	Super::BeginPlay();

	// 初始化弹药
	CurrentAmmo = MaxAmmo;
}

void AAweaponbase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAweaponbase::Fire()
{
	if (bIsReloading) return;

	if (CurrentAmmo <= 0)
	{
		Reload();
		return;
	}

	// ===== 找枪口位置（从子组件里找） =====
	FVector MuzzleLocation = FVector::ZeroVector;

	if (WeaponMesh)
	{
		// 先看 WeaponMesh 自己有没有 Muzzle 插槽
		if (WeaponMesh->DoesSocketExist("Muzzle"))
		{
			MuzzleLocation = WeaponMesh->GetSocketLocation("Muzzle");
		}
		else
		{
			// 没有插槽，就遍历子组件，找挂了模型的
			TArray<USceneComponent*> ChildComponents;
			WeaponMesh->GetChildrenComponents(true, ChildComponents);
			for (USceneComponent* Child : ChildComponents)
			{
				if (Child && Child->IsA(UStaticMeshComponent::StaticClass()))
				{
					MuzzleLocation = Child->GetComponentLocation();
					break;
				}
			}
		}
	}

	// 如果还是零，就用武器自身位置
	if (MuzzleLocation.IsZero())
	{
		MuzzleLocation = GetActorLocation();
	}

	// 枪口光效
	if (MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, MuzzleLocation);
	}

	// ===== 从玩家摄像机发射射线 =====
	APawn* OwnerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!OwnerPawn || !OwnerPawn->GetController())
	{
		return;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	OwnerPawn->GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector CameraEnd = CameraLocation + CameraRotation.Vector() * Range;

	FCollisionQueryParams CameraParams;
	CameraParams.AddIgnoredActor(this);
	CameraParams.AddIgnoredActor(OwnerPawn);

	FHitResult CameraHit;
	bool bCameraHit = GetWorld()->LineTraceSingleByChannel(
		CameraHit, CameraLocation, CameraEnd, ECC_Visibility, CameraParams);

	FVector AimPoint = bCameraHit ? CameraHit.Location : CameraEnd;

	// ===== 从枪口向准星方向发射射线 =====
	FVector MuzzleDirection = (AimPoint - MuzzleLocation).GetSafeNormal();
	FVector MuzzleEnd = MuzzleLocation + MuzzleDirection * Range;

	FCollisionQueryParams MuzzleParams;
	MuzzleParams.AddIgnoredActor(this);
	MuzzleParams.AddIgnoredActor(OwnerPawn);

	FHitResult MuzzleHit;
	bool bMuzzleHit = GetWorld()->LineTraceSingleByChannel(
		MuzzleHit, MuzzleLocation, MuzzleEnd, ECC_Visibility, MuzzleParams);


	// ===== 伤害 =====
	UE_LOG(LogTemp, Warning, TEXT("MuzzleHit: %d, HitActor: %s"), bMuzzleHit, MuzzleHit.GetActor() ? *MuzzleHit.GetActor()->GetName() : TEXT("None"));
	if (bMuzzleHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *MuzzleHit.GetActor()->GetName());

		if (HitEffectPool)
		{
			AActor* PooledEffect = HitEffectPool->GetFromPool();
			if (PooledEffect)
			{
				PooledEffect->SetActorLocation(MuzzleHit.Location);
			}
		}

		UGameplayStatics::ApplyDamage(
			MuzzleHit.GetActor(), Damage, GetInstigatorController(), this, nullptr);
	}

	CurrentAmmo--;
	UE_LOG(LogTemp, Warning, TEXT("Ammo: %d"), CurrentAmmo);
}

void AAweaponbase::Reload()
{
	if (bIsReloading || CurrentAmmo == MaxAmmo) return;

	bIsReloading = true;

	// 直接用一个定时器搞定换弹
	GetWorldTimerManager().SetTimer(ReloadTimerHandle, [this]()
		{
			CurrentAmmo = MaxAmmo;
			bIsReloading = false;
			UE_LOG(LogTemp, Warning, TEXT("换弹完毕！"));
		}, ReloadTime, false);
}

void AAweaponbase::ResetAmmo()
{
	CurrentAmmo = MaxAmmo;
}