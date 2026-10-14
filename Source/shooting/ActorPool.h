#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActorPool.generated.h"

class AEnemyBase;

UCLASS()
class SHOOTING_API AActorPool : public AActor
{
	GENERATED_BODY()

public:
	AActorPool();

	// 预创建的数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool")
	int32 PoolSize = 5;

	// 池化的 Actor 类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool")
	TSubclassOf<AActor> PooledActorClass;

	// 自动生成间隔
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool")
	float SpawnInterval = 5.0f;

	// 是否自动生成
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool")
	bool bAutoSpawn = false;

	// 从池里取一个
	AActor* GetFromPool();

	// 归还到池里
	void ReturnToPool(AActor* Actor);

	// 生成一个敌人
	UFUNCTION(BlueprintCallable, Category = "Pool")
	void SpawnOne();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<AActor*> Pool;

	int32 CurrentIndex = 0;

	FTimerHandle SpawnTimerHandle;
};