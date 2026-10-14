#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class AActorPool;

UCLASS()
class SHOOTING_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 血量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float Health = 100.0f;

	// 移动速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MoveSpeed = 300.0f;

	// 攻击距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float AttackRange = 300.0f;

	// 攻击伤害
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float AttackDamage = 10.0f;

	// 攻击间隔
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float AttackInterval = 1.0f;

	// 追踪玩家
	void MoveToPlayer();

	// 攻击玩家
	void AttackPlayer();

	// 受伤
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// 死亡时通知池子
	class AActorPool* OwnerPool;

	// 死了没
	bool bIsDead = false;

	// 玩家引用
	class ACharacter* PlayerCharacter;

private:
	// 攻击计时器
	FTimerHandle AttackTimerHandle;


};