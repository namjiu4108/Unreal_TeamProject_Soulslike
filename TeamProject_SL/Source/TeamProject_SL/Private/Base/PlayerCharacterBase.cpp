// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/PlayerCharacterBase.h"
#include "Base/PlayerBaseAttributeSet.h"

APlayerCharacterBase::APlayerCharacterBase()
{

	PlayerAttribute = CreateDefaultSubobject<UPlayerBaseAttributeSet>(TEXT("PlayerAttributeSet"));
	BaseAttributeSet.Add(PlayerAttribute);
}
