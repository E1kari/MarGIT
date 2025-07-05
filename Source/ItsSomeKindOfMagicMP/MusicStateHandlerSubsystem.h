// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "AudioManagerSubsystem.h"
#include "MusicStateHandlerSubsystem.generated.h"


USTRUCT(BlueprintType)
struct FMusicStateMapping : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag StateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LayerName;
};

UCLASS()
class ITSSOMEKINDOFMAGICMP_API UMusicStateHandlerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "MusicStateHandler")
	void SetMusicStateActive(const FGameplayTag& StateTag,bool bActive);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	bool GetLayerNameForTag(const FGameplayTag& StateTag, FName& OutLayerName) const;

	void ActivateState(const FGameplayTag& StateTag);

	void DeactivateState(const FGameplayTag& StateTag);

private:
	UPROPERTY(EditDefaultsOnly, Category = "MusicStateHandler")
	UDataTable* StateMappingTable;

	TMap<FGameplayTag, int32> StateRefCount;
};
