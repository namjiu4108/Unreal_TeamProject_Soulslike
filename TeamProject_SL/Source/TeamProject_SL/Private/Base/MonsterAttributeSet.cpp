// Fill out your copyright notice in the Description page of Project Settings.

#include "Base/MonsterAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UMonsterAttributeSet::UMonsterAttributeSet()
{
	Groggy = 0.0f;
	MaxGroggy = 100.0f;
}

void UMonsterAttributeSet::OnRep_Groggy(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMonsterAttributeSet, Groggy, OldValue);
}

void UMonsterAttributeSet::OnRep_MaxGroggy(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMonsterAttributeSet, MaxGroggy, OldValue);
}

void UMonsterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMonsterAttributeSet, Groggy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMonsterAttributeSet, MaxGroggy, COND_None, REPNOTIFY_Always);
}

void UMonsterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetGroggyAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxGroggy());
	}
}

void UMonsterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetGroggyAttribute())
	{
		SetGroggy(FMath::Clamp(GetGroggy(), 0.0f, GetMaxGroggy()));
	}
}
