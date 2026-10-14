#include "ActorPool.h"
#include "EnemyBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AActorPool::AActorPool()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AActorPool::BeginPlay()
{
	Super::BeginPlay();

	if (!PooledActorClass) return;

	// 预创建一批
	for (int32 i = 0; i < PoolSize; i++)
	{
		AActor* NewActor = GetWorld()->SpawnActor<AActor>(PooledActorClass, GetActorLocation(), GetActorRotation());
		if (NewActor)
		{
			NewActor->SetActorHiddenInGame(true);
			NewActor->SetActorEnableCollision(false);
			Pool.Add(NewActor);
		}
	}

	// 自动生成
	if (bAutoSpawn && SpawnInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AActorPool::SpawnOne, SpawnInterval, true);
	}
}

AActor* AActorPool::GetFromPool()
{
	if (Pool.Num() == 0) return nullptr;

	AActor* Actor = Pool[CurrentIndex];
	CurrentIndex = (CurrentIndex + 1) % Pool.Num();

	if (Actor)
	{
		Actor->SetActorHiddenInGame(false);
		Actor->SetActorEnableCollision(true);
	}
	return Actor;
}

void AActorPool::ReturnToPool(AActor* Actor)
{
	if (Actor)
	{
		Actor->SetActorHiddenInGame(true);
		Actor->SetActorEnableCollision(false);
	}
}

void AActorPool::SpawnOne()
{
	AActor* Actor = GetFromPool();
	if (Actor)
	{
		// 固定 4 个出生点，随机选一个
		FVector SpawnPoints[] = {
			FVector(1000, 1000, 50),
			FVector(-1000, 1000, 50),
			FVector(1000, -1000, 50),
			FVector(-1000, -1000, 50)
		};
		int32 Index = FMath::RandRange(0, 3);
		Actor->SetActorLocation(GetActorLocation() + SpawnPoints[Index]);
		Actor->SetActorHiddenInGame(false);
		Actor->SetActorEnableCollision(true);

		if (AEnemyBase* Enemy = Cast<AEnemyBase>(Actor))
		{
			Enemy->OwnerPool = this;
			Enemy->Health = 50.0f;
			Enemy->bIsDead = false;
			Enemy->PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

			// 强制恢复移动
			if (UCharacterMovementComponent* MoveComp = Enemy->GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Walking);
				MoveComp->MaxWalkSpeed = 300.0f;
				MoveComp->Activate(true);
			}

			// 强制恢复碰撞和 Tick
			Enemy->SetActorEnableCollision(true);
			Enemy->SetActorHiddenInGame(false);
			Enemy->SetActorTickEnabled(true);

			// 生成 AI 控制器
			Enemy->SpawnDefaultController();
		}
	}
}