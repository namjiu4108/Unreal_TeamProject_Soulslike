// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS_BASIC(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class TEAMPROJECT_SL_API UMonsterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMonsterAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_Groggy)
	FGameplayAttributeData Groggy;
	ATTRIBUTE_ACCESSORS_BASIC(UMonsterAttributeSet, Groggy);

	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_MaxGroggy)
	FGameplayAttributeData MaxGroggy;
	ATTRIBUTE_ACCESSORS_BASIC(UMonsterAttributeSet, MaxGroggy);

	// 사망 이후 Groggy를 Max로 고정하고, 이후 어떤 GameplayEffect가 들어와도 값이 안 바뀌게 잠금
	void LockGroggyAtMax();

private:
	UFUNCTION()
	void OnRep_Groggy(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_MaxGroggy(const FGameplayAttributeData& OldValue) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	// true가 되면 Groggy는 더 이상 변하지 않고 Max로 고정됨 (사망 이후 사용)
	bool bGroggyLocked = false;
};
