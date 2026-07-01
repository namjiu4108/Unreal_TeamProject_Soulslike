// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/PlayerBaseAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"


UPlayerBaseAttributeSet::UPlayerBaseAttributeSet()
{
	MP= 100.0f;
	MaxMP = 100.0f;
}

void UPlayerBaseAttributeSet::OnRep_MP(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerBaseAttributeSet, MP, OldValue);
}

void UPlayerBaseAttributeSet::OnRep_MaxMP(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerBaseAttributeSet, MaxMP, OldValue);
}

void UPlayerBaseAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerBaseAttributeSet, MP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerBaseAttributeSet, MaxMP, COND_None, REPNOTIFY_Always);
}

void UPlayerBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMPAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMP());
	}
}

void UPlayerBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetMPAttribute())
	{
		SetMP(FMath::Clamp(GetMP(), 0.0f, GetMaxMP()));
	}
}
