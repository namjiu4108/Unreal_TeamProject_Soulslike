#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FogInteractionManager.generated.h"

class AActor;
class UFogEffectComponent;
class UNiagaraComponent;

UCLASS()
class TEAMPROJECT_SL_API AFogInteractionManager : public AActor
{
	GENERATED_BODY()

public:
	AFogInteractionManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Assign the placed fog actor that owns the NiagaraComponent.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Fog")
	TObjectPtr<AActor> FogSourceActor;

	// Optional manual list of actors that own FogEffectComponent instances.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Fog")
	TArray<TObjectPtr<AActor>> EffectorActors;

	// When enabled, the manager also searches the world for active FogEffectComponent instances.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
	bool bSearchEffectorsInWorld = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog", meta = (ClampMin = "1"))
	int32 MaxEffectors = 4;

	// Resolved from FogSourceActor at runtime.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Fog")
	TObjectPtr<UNiagaraComponent> FogNiagaraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog|Debug")
	bool bEnableDebugOutput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog|Debug")
	bool bShowOnScreenDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog|Debug", meta = (ClampMin = "0.05"))
	float DebugPrintInterval = 0.25f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Fog|Debug")
	int32 LastActiveEffectorCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Fog|Debug")
	FString LastDebugMessage;

private:
	void UpdateDebugOutput(const TArray<UFogEffectComponent*>& ActiveEffectors, int32 ActiveCount);
	void ResolveNiagaraComponent();
	void GatherActiveEffectors(TArray<UFogEffectComponent*>& OutEffectors) const;
	void ClearInactiveNiagaraSlots(int32 ActiveCount) const;
	void UpdateNiagaraEffectors();
	static FName BuildIndexedParameterName(const TCHAR* BaseName, int32 Index);

	float TimeSinceLastDebugPrint = 0.0f;
};
