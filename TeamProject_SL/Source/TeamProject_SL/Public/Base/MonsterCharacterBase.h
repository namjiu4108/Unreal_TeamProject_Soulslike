// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/CharacterBase.h"
#include "MonsterCharacterBase.generated.h"

class UMonsterAttributeSet;
struct FOnAttributeChangeData;

UCLASS()
class TEAMPROJECT_SL_API AMonsterCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

public:
	AMonsterCharacterBase();

	UPROPERTY(EditAnywhere, Category = "Attributes")
	TObjectPtr<UMonsterAttributeSet> MonsterAttribute;

	// Groggy 또는 MaxGroggy가 갱신됐을 때 Blueprint로 알려주는 이벤트 (그로기 UI용)
	UFUNCTION(BlueprintImplementableEvent, Category = "Attributes")
	void OnGroggyUpdated(float CurrentGroggy, float MaxGroggy);

	// Groggy가 MaxGroggy의 절반을 처음 넘는 순간 한 번만 호출 (소경직)
	UFUNCTION(BlueprintImplementableEvent, Category = "Attributes")
	void OnGroggyFlinch();

	// Groggy가 MaxGroggy에 도달하는 순간 한 번만 호출 (쓰러짐)
	UFUNCTION(BlueprintImplementableEvent, Category = "Attributes")
	void OnGroggyDown();

	// Health가 0 이하로 떨어지는 순간 한 번만 호출 (사망)
	UFUNCTION(BlueprintImplementableEvent, Category = "Attributes")
	void OnMonsterDeath();

	// 대경직 모션이 끝난 뒤 Blueprint에서 호출해서 Groggy를 0으로 되돌리는 용도
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ResetGroggy();

	// 사망 처리(OnMonsterDeath) 시 호출 — Groggy를 Max로 고정하고 이후 변경되지 않게 잠금
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void LockGroggyAtMax();



protected:
	// 부모 CharacterBase에서 Health / MaxHealth 관련 바인딩(UI 갱신용)은 이미 처리됨
	// 몬스터 전용 Attribute인 Groggy / MaxGroggy 바인딩과, 사망 감지용 Health 바인딩을 추가로 등록
	virtual void BindAttributeChangeDelegates() override;

	void HandleGroggyChanged(const FOnAttributeChangeData& Data);
	void HandleMonsterHealthChanged(const FOnAttributeChangeData& Data);

	// Groggy/사망 델리게이트가 중복 등록되지 않도록 막는 플래그
	bool MonsterAttributeDelegatesBound = false;
};
