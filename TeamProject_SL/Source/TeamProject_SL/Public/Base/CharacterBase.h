// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamProject_SLCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GameplayAbilitySpecHandle.h"
#include "CharacterBase.generated.h"

class AController;
class UAbilitySystemComponent;
class UBaseAttributeSet;
class UGameplayAbility;

UCLASS()
class TEAMPROJECT_SL_API ACharacterBase : public ATeamProject_SLCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()



public:
	ACharacterBase();

	/*어빌리티 시스템 컴포넌트*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/*능력치 어빌리티 클래스*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	TArray<TObjectPtr<UAttributeSet>>  BaseAttributeSet;

	/*공용 능력치*/
	UPROPERTY(EditAnywhere, Category = "Attributes")
	TObjectPtr<UBaseAttributeSet> BaseAttribute;

	/*캐릭터가 보유할수있는 스킬 배열*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GASGamePlayAbility")
	TArray<TSubclassOf<UGameplayAbility>> InitalAbilities;

protected:
	/*멀티플레이 모드 GAS*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
	EGameplayEffectReplicationMode AscReplicationMode = EGameplayEffectReplicationMode::Mixed;

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	FGameplayAbilitySpecHandle InitializeAbility(TSubclassOf<UGameplayAbility> AbilityToGet, int32 AbilityLevel);
	void InitializeAbilityMulti(TArray<TSubclassOf<UGameplayAbility>> AbilityToAcquire, int32 AbilityLevel);
};
