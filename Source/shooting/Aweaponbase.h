// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Aweaponbase.generated.h"

class AActorPool;												

UCLASS()
class SHOOTING_API AAweaponbase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAweaponbase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 伤害值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponStats")
	float Damage = 10.0f;

	// 射速
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponStats")
	float FireRate = 10.0f;

	// 射击距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponStats")
	float Range = 5000.0f;

		// 武器模型组件（改成静态网格体）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* WeaponMesh;

	// 弹匣容量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponStats")
	int32 MaxAmmo = 30;

	// 当前弹匣里的子弹数
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponStats")
	int32 CurrentAmmo;

	// 换弹时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponStats")
	float ReloadTime = 2.0f;

	// 枪口光效组件
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponEffects")
	class UParticleSystem* MuzzleFlash;

	// 开火时镜头上抬的角度（后坐力）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponEffects")
	float RecoilPitch = 2.0f;

	// 命中特效
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponEffects")
	class UParticleSystem* HitEffect;

	// 换弹状态
	bool bIsReloading = false;

	// 换弹定时器
	FTimerHandle ReloadTimerHandle;

	// 换弹函数
	void Reload();

	// 重置弹药（拾取后会用）
	void ResetAmmo();

	// 开火函数
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();

	// 获取射击起点和方向（由角色提供）
	FVector GetFireStartLocation() const;
	FVector GetFireDirection() const;

	// 特效对象池
	class AActorPool* HitEffectPool;
};