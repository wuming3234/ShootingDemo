// Copyright Epic Games, Inc. All Rights Reserved.

#include "shootingGameMode.h"
#include "shootingCharacter.h"
#include "UObject/ConstructorHelpers.h"

AshootingGameMode::AshootingGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
