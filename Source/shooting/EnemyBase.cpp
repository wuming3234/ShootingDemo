	#include "EnemyBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "ActorPool.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 让敌人自动朝向移动方向
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;

	// 禁用控制器旋转
	bUseControllerRotationYaw = false;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// 获取场景里的玩家
	PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// 启动攻击定时器
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AEnemyBase::AttackPlayer, AttackInterval, true);
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

	// 每帧追踪玩家
	MoveToPlayer();
}

void AEnemyBase::MoveToPlayer()
{
	if (!PlayerCharacter) return;

	// 检查距离玩家多远部分，以及距离检测，超过距离开始追玩家
	float Distance = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());

	if (Distance > AttackRange)
	{
		FVector Direction = (PlayerCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		AddMovementInput(Direction, 1.0f);
	}
}

void AEnemyBase::AttackPlayer()
{
	if (bIsDead || !PlayerCharacter) return;

	// 检测玩家在不在攻击范围
	float Distance = FVector::Dist(GetActorLocation(), PlayerCharacter->GetActorLocation());
	if (Distance <= AttackRange)
	{
		UGameplayStatics::ApplyDamage(PlayerCharacter, AttackDamage, GetController(), this, nullptr);
		UE_LOG(LogTemp, Warning, TEXT("Enemy attack!"));
	}
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.0f;

	Health -= DamageAmount;
	UE_LOG(LogTemp, Warning, TEXT("Enemy HP: %f"), Health);

	if (Health <= 0.0f)
	{
		bIsDead = true;
		UE_LOG(LogTemp, Warning, TEXT("Enemy dead!"));
		// 敌人死了归还对象池
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		bIsDead = true;

		if (OwnerPool)
		{
			OwnerPool->ReturnToPool(this);
		}
	}

	return DamageAmount;
}