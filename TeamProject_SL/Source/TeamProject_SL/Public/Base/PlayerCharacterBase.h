// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/CharacterBase.h"
#include "PlayerCharacterBase.generated.h"

/**
 *
 */
class UPlayerBaseAttributeSet;

UCLASS()
class TEAMPROJECT_SL_API APlayerCharacterBase : public ACharacterBase
{
	GENERATED_BODY()



public:
	APlayerCharacterBase();

	UPROPERTY(EditAnywhere, Category = "Attributes")
	TObjectPtr<UPlayerBaseAttributeSet> PlayerAttribute;
};
