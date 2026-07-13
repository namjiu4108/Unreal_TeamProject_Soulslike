// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/PlayerCharacterBase.h"
#include "Base/PlayerBaseAttributeSet.h"

APlayerCharacterBase::APlayerCharacterBase()
{

	PlayerAttribute = CreateDefaultSubobject<UPlayerBaseAttributeSet>(TEXT("PlayerAttributeSet"));
	BaseAttributeSet.Add(PlayerAttribute);
}

void APlayerCharacterBase::BindAttributeChangeDelegates()
{
	Super::BindAttributeChangeDelegates();

	if (PlayerAttributeDelegatesBound || !AbilitySystemComponent || !PlayerAttribute)
	{
		return;
	}

	PlayerAttributeDelegatesBound = true;

	// MP Attribute가 바뀔 때 HandleMPChanged가 자동으로 호출되도록 등록
	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UPlayerBaseAttributeSet::GetMPAttribute())
		.AddUObject(this, &APlayerCharacterBase::HandleMPChanged);

	AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UPlayerBaseAttributeSet::GetMaxMPAttribute())
		.AddUObject(this, &APlayerCharacterBase::HandleMaxMPChanged);
}

void APlayerCharacterBase::HandleMPChanged(const FOnAttributeChangeData& Data)
{
	if (!PlayerAttribute)
	{
		return;
	}

	OnMPUpdated(PlayerAttribute->GetMP(), PlayerAttribute->GetMaxMP());
}

void APlayerCharacterBase::HandleMaxMPChanged(const FOnAttributeChangeData& Data)
{
	if (!PlayerAttribute)
	{
		return;
	}

	OnMPUpdated(PlayerAttribute->GetMP(), PlayerAttribute->GetMaxMP());
}
