#include "Effect/FogInteractionManager.h"

#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Effect/FogEffectComponent.h"
#include "EngineUtils.h"
#include "NiagaraComponent.h"

AFogInteractionManager::AFogInteractionManager()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AFogInteractionManager::BeginPlay()
{
	Super::BeginPlay();
	ResolveNiagaraComponent();
}

void AFogInteractionManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimeSinceLastDebugPrint += DeltaTime;
	UpdateNiagaraEffectors();
}

void AFogInteractionManager::ResolveNiagaraComponent()
{
	FogNiagaraComponent = nullptr;

	if (IsValid(FogSourceActor))
	{
		FogNiagaraComponent = FogSourceActor->FindComponentByClass<UNiagaraComponent>();
	}

	if (!FogNiagaraComponent)
	{
		FogNiagaraComponent = FindComponentByClass<UNiagaraComponent>();
	}
}

void AFogInteractionManager::GatherActiveEffectors(TArray<UFogEffectComponent*>& OutEffectors) const
{
	TSet<const UFogEffectComponent*> SeenEffectors;

	auto AddEffectorsFromActor = [&OutEffectors, &SeenEffectors](AActor* Actor)
	{
		if (!IsValid(Actor))
		{
			return;
		}

		TInlineComponentArray<UFogEffectComponent*> ActorEffectors;
		Actor->GetComponents(ActorEffectors);

		for (UFogEffectComponent* Effector : ActorEffectors)
		{
			if (!IsValid(Effector) || !Effector->bActive || SeenEffectors.Contains(Effector))
			{
				continue;
			}

			SeenEffectors.Add(Effector);
			OutEffectors.Add(Effector);
		}
	};

	for (AActor* EffectorActor : EffectorActors)
	{
		AddEffectorsFromActor(EffectorActor);
	}

	if (bSearchEffectorsInWorld && GetWorld())
	{
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AddEffectorsFromActor(*It);
		}
	}
}

void AFogInteractionManager::ClearInactiveNiagaraSlots(int32 ActiveCount) const
{
	if (!FogNiagaraComponent)
	{
		return;
	}

	for (int32 Index = ActiveCount; Index < MaxEffectors; ++Index)
	{
		FogNiagaraComponent->SetVariableVec3(BuildIndexedParameterName(TEXT("EffectorPos"), Index), FVector::ZeroVector);
		FogNiagaraComponent->SetVariableVec3(BuildIndexedParameterName(TEXT("EffectorPrevPos"), Index), FVector::ZeroVector);
		FogNiagaraComponent->SetVariableFloat(BuildIndexedParameterName(TEXT("EffectorRadius"), Index), 0.0f);
		FogNiagaraComponent->SetVariableFloat(BuildIndexedParameterName(TEXT("EffectorStrength"), Index), 0.0f);
	}
}

void AFogInteractionManager::UpdateNiagaraEffectors()
{
	if (!FogNiagaraComponent)
	{
		ResolveNiagaraComponent();
	}

	if (!FogNiagaraComponent)
	{
		return;
	}

	TArray<UFogEffectComponent*> ActiveEffectors;
	GatherActiveEffectors(ActiveEffectors);

	const int32 Count = FMath::Min(ActiveEffectors.Num(), MaxEffectors);
	LastActiveEffectorCount = Count;
	FogNiagaraComponent->SetVariableInt(TEXT("EffectorCount"), Count);

	for (int32 Index = 0; Index < Count; ++Index)
	{
		const UFogEffectComponent* Effector = ActiveEffectors[Index];

		FogNiagaraComponent->SetVariableVec3(
			BuildIndexedParameterName(TEXT("EffectorPos"), Index),
			Effector->CurrentPosition
		);

		FogNiagaraComponent->SetVariableVec3(
			BuildIndexedParameterName(TEXT("EffectorPrevPos"), Index),
			Effector->PreviousPosition
		);

		FogNiagaraComponent->SetVariableFloat(
			BuildIndexedParameterName(TEXT("EffectorRadius"), Index),
			Effector->Radius
		);

		FogNiagaraComponent->SetVariableFloat(
			BuildIndexedParameterName(TEXT("EffectorStrength"), Index),
			Effector->Strength
		);
	}

	ClearInactiveNiagaraSlots(Count);
	UpdateDebugOutput(ActiveEffectors, Count);
}

FName AFogInteractionManager::BuildIndexedParameterName(const TCHAR* BaseName, int32 Index)
{
	return FName(*FString::Printf(TEXT("%s%d"), BaseName, Index));
}

void AFogInteractionManager::UpdateDebugOutput(const TArray<UFogEffectComponent*>& ActiveEffectors, int32 ActiveCount)
{
	if (!bEnableDebugOutput)
	{
		LastDebugMessage = TEXT("Debug disabled");
		return;
	}

	if (TimeSinceLastDebugPrint < DebugPrintInterval)
	{
		return;
	}

	TimeSinceLastDebugPrint = 0.0f;

	const FString NiagaraName = IsValid(FogNiagaraComponent)
		? FogNiagaraComponent->GetName()
		: TEXT("None");

	if (ActiveCount > 0 && IsValid(ActiveEffectors[0]))
	{
		const UFogEffectComponent* Effector = ActiveEffectors[0];
		LastDebugMessage = FString::Printf(
			TEXT("Fog=%s Count=%d First=%s Pos=%s Prev=%s Radius=%.1f Strength=%.1f"),
			*NiagaraName,
			ActiveCount,
			*Effector->GetOwner()->GetName(),
			*Effector->CurrentPosition.ToString(),
			*Effector->PreviousPosition.ToString(),
			Effector->Radius,
			Effector->Strength
		);
	}
	else
	{
		LastDebugMessage = FString::Printf(TEXT("Fog=%s Count=0 No active effectors"), *NiagaraName);
	}

	UE_LOG(LogTemp, Log, TEXT("%s"), *LastDebugMessage);

	if (bShowOnScreenDebug && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			reinterpret_cast<uint64>(this),
			DebugPrintInterval + 0.05f,
			FColor::Cyan,
			LastDebugMessage
		);
	}
}
