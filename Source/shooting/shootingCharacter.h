// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "shootingCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

class UInputMappingContext;
class UInputAction;
class AAweaponbase;

UCLASS(config = Game)
class AshootingCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AshootingCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ========== 增强输入 ==========
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* LookAction;

	// ========== 战斗相关 ==========
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	class UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	class UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	class UInputAction* PickupAction;

	// 当前武器
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AAweaponbase* CurrentWeapon;

	// 右手骨骼名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName WeaponSocketName = "hand_r";

	// 武器蓝图路径（在 UE 里右键 BP_weapon 复制引用后粘贴到这里）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FString WeaponBlueprintPath = "/Game/Fab/AK-105/BP_weapon.BP_weapon_C";

	// 手里有没有枪（给动画蓝图用）
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHasWeapon = false;

	// ========== 功能函数 ==========
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartFire();
	void StopFire();
	void FireWeapon();
	void ReloadWeapon();

	// 生成武器
	void SpawnWeapon();

	// 拾取武器
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PickupWeapon();

	// ========== 玩家血量 ==========
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	// 受伤
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// 死亡
	void Die();

	// HUD 控件类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	// HUD 实例
	UPROPERTY()
	class UUserWidget* HUDWidget;

	// 更新 HUD
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHUD();

	// 闪现技能
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* DashAction;

	void Dash();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float DashDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float DashCooldown = 5.0f;

	bool bCanDash = true;
	FTimerHandle DashTimerHandle;
private:
	// 摄像机相关组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// 切换视角的输入
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* ToggleViewAction;

	// 切换视角函数
	void ToggleView();


	// 是否在第一人称
	bool bIsFirstPerson = false;

	// 第一人称相机
	UPROPERTY()
	class UCameraComponent* FirstPersonCamera;

	// 开火状态
	bool bIsFiring = false;
	FTimerHandle FireTimerHandle;
};