// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/CharacterBase.h"
#include "PlayerCharacterBase.generated.h"

/**
 *
 */
class UPlayerBaseAttributeSet;
struct FOnAttributeChangeData;

UCLASS()
class TEAMPROJECT_SL_API APlayerCharacterBase : public ACharacterBase
{
	GENERATED_BODY()



public:
	APlayerCharacterBase();

	UPROPERTY(EditAnywhere, Category = "Attributes")
	TObjectPtr<UPlayerBaseAttributeSet> PlayerAttribute;

	// MP 또는 MaxMP가 변경되었을 때 Blueprint로 알려주는 이벤트
    // 플레이어에서 이 이벤트를 받아 MP바와 MP 상태 아이콘을 갱신
	UFUNCTION(BlueprintImplementableEvent, Category = "Attributes")
	void OnMPUpdated(float CurrentMP, float MaxMP);

protected:
	// 부모 CharacterBase에서 Health / MaxHealth 변경 감지를 먼저 등록
	// 플레이어 전용 Attribute인 MP / MaxMP 변경 감지를 추가로 등록
	virtual void BindAttributeChangeDelegates() override;

	// MP가 변경되었을 때 호출되는 콜백
	// 현재 MP와 MaxMP를 Blueprint 이벤트로 넘겨 UI를 갱신
	void HandleMPChanged(const FOnAttributeChangeData& Data);

	// MaxMP가 변경되었을 때 호출되는 콜백
	// 최대 MP가 바뀌면 MP 비율도 달라질 수 있으므로 UI를 다시 갱신
	void HandleMaxMPChanged(const FOnAttributeChangeData& Data);

	// MP 델리게이트가 중복 등록되지 않도록 막는 플래그
	bool PlayerAttributeDelegatesBound = false;
};
