// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "LevelFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ITSSOMEKINDOFMAGICMP_API ULevelFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Level | Utils")
	static bool IsActorInSublevel(const AActor* Actor, const TSoftObjectPtr<UWorld>& WorldReference);

	UFUNCTION(BlueprintCallable, Category = "Level | Utils")
	static FString GetLevelNameFromReference(const TSoftObjectPtr<UWorld>& WorldReference);

	static FString GetPathNameAfterDot(const FString& AssetPath);
};
