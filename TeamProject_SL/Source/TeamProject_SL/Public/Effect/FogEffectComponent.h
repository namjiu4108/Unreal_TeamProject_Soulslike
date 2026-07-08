// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FogEffectComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TEAMPROJECT_SL_API UFogEffectComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UFogEffectComponent();

	/*충돌 범위*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Effector")
	float Radius = 150.0f;

	/*안개 밀쳐지는 강도*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Effector")
	float Strength = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Effector")
	bool bActive = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog Effector")
	FVector PreviousPosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fog Effector")
	FVector CurrentPosition;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


		
};
