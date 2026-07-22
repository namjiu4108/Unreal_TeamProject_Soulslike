// Fill out your copyright notice in the Description page of Project Settings.

#include "Base/MonsterCharacterBase.h"
#include "Base/MonsterAttributeSet.h"
#include "Base/BaseAttributeSet.h"

AMonsterCharacterBase::AMonsterCharacterBase()
{
	MonsterAttribute = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
	BaseAttributeSet.Add(MonsterAttribute);

	// BaseAttribute(Health/MaxHealth)는 100/100 기본값으로 이미 생성되어 있음, 몬스터 체력으로 덮어씀
	// 생성자 시점에는 ASC가 아직 초기화되지 않아서 Set 함수(ASC 경유)를 쓰면 안 되고, 구조체에 직접 대입해야 함
	if (BaseAttribute)
	{
		BaseAttribute->Health = 500.0f;
		BaseAttribute->MaxHealth = 500.0f;
	}
}

void AMonsterCharacterBase::BindAttributeChangeDelegates()
{
	Super::BindAttributeChangeDelegates();

	if (MonsterAttributeDelegatesBound || !AbilitySystemComponent || !MonsterAttribute)
	{
		return;
	}

	MonsterAttributeDelegatesBound = true;

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UMonsterAttributeSet::GetGroggyAttribute())
		.AddUObject(this, &AMonsterCharacterBase::HandleGroggyChanged);

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetHealthAttribute())
		.AddUObject(this, &AMonsterCharacterBase::HandleMonsterHealthChanged);
}

void AMonsterCharacterBase::HandleGroggyChanged(const FOnAttributeChangeData& Data)
{
	if (!MonsterAttribute)
	{
		return;
	}

	OnGroggyUpdated(MonsterAttribute->GetGroggy(), MonsterAttribute->GetMaxGroggy());

	const float MaxGroggyValue = MonsterAttribute->GetMaxGroggy();
	const float HalfThreshold = MaxGroggyValue * 0.5f;

	if (Data.OldValue < HalfThreshold && Data.NewValue >= HalfThreshold)
	{
		OnGroggyFlinch();
	}

	if (Data.OldValue < MaxGroggyValue && Data.NewValue >= MaxGroggyValue)
	{
		OnGroggyDown();
	}
}

void AMonsterCharacterBase::HandleMonsterHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.OldValue > 0.0f && Data.NewValue <= 0.0f)
	{
		OnMonsterDeath();
	}
}

void AMonsterCharacterBase::ResetGroggy()
{
	if (MonsterAttribute)
	{
		MonsterAttribute->SetGroggy(0.0f);
	}
}
