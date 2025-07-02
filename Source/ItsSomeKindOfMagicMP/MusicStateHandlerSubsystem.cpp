// Fill out your copyright notice in the Description page of Project Settings.


#include "MusicStateHandlerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "AudioManagerSubsystem.h"

void UMusicStateHandlerSubsystem::SetMusicStateActive(const FGameplayTag& StateTag, bool bActive)
{
	int32& Count = StateRefCount.FindOrAdd(StateTag);
	if (bActive)
	{
		if (++Count == 1)
		{
			ActivateState(StateTag);
		}
	}
	else
	{
		if (--Count <= 0)
		{
			StateRefCount.Remove(StateTag);
			DeactivateState(StateTag);
		}
	}
}

void UMusicStateHandlerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UAudioManagerSubsystem::StaticClass());

	Super::Initialize(Collection);

	if (!StateMappingTable)
	{
		static const FString Path = TEXT("/Game/Audio/DataTable/DT_MusicStateMapping.DT_MusicStateMapping");
		StateMappingTable = LoadObject<UDataTable>(nullptr, *Path);
		if (!StateMappingTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("MusicStateHandler: Konnte DT_MusicStateMapping nicht laden (%s)"), *Path);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("MusicStateHandler: DataTable geladen: %s"), *Path);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("MusicStateHandler: DataTable bereits im Editor gesetzt."));
	}
}

bool UMusicStateHandlerSubsystem::GetLayerNameForTag(const FGameplayTag& StateTag, FName& OutLayerName) const
{
	if (!StateMappingTable) return false;

	static const FString ContextString(TEXT("MusicStateMappingLookup"));

	for (auto& Row : StateMappingTable->GetRowMap())
	{
		const FMusicStateMapping* Mapping = reinterpret_cast<FMusicStateMapping*>(Row.Value);
		if (Mapping && Mapping->StateTag == StateTag)
		{
			OutLayerName = Mapping->LayerName;
			return true;
		}
	}
	return false;
}

void UMusicStateHandlerSubsystem::ActivateState(const FGameplayTag& StateTag)
{
	FName LayerName;

	if (!GetLayerNameForTag(StateTag, LayerName))
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateState: No mapping for tag %s"), *StateTag.ToString());
		return;
	}

	UAudioManagerSubsystem* AudioMgr = GetGameInstance()->GetSubsystem<UAudioManagerSubsystem>();
	if (AudioMgr)
	{
		AudioMgr->FadeMusicLayer(LayerName, 1.0f);
	}
}

void UMusicStateHandlerSubsystem::DeactivateState(const FGameplayTag& StateTag)
{
	FName LayerName;

	if (!GetLayerNameForTag(StateTag, LayerName))
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateState: No mapping for tag %s"), *StateTag.ToString());
		return;
	}

	UAudioManagerSubsystem* AudioMgr = GetGameInstance()->GetSubsystem<UAudioManagerSubsystem>();
	if (AudioMgr)
	{
		AudioMgr->FadeMusicLayer(LayerName, 0.0f);
	}
}
