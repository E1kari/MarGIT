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
	static FString GetSublevelNameFromReference(const TSoftObjectPtr<UWorld>& WorldReference);

	static FString GetPathNameAfterDot(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "Level | Utils", meta = (WorldContext = "WorldContextObject"))
	static void GetAllLoadedSublevelNames(UObject* WorldContextObject, TArray<FString>& OutLevelNames);

	UFUNCTION(BlueprintCallable, Category = "Level | Utils", meta = (WorldContext = "WorldContextObject"))
	static void GetAllLoadedSublevel(UObject* WorldContextObject, TArray<TSoftObjectPtr<UWorld>>& OutLevel);

	UFUNCTION(BlueprintCallable, Category = "Level | Utils", meta = (WorldContext = "WorldContextObject"))
	static bool IsSublevelLoaded(UObject* WorldContextObject, const TSoftObjectPtr<UWorld>& LevelReference);

	UFUNCTION(BlueprintCallable, Category = "Level | Utils")
	static void GetAllSublevel(const TSoftObjectPtr<UWorld>& WorldRef, TArray<TSoftObjectPtr<UWorld>>& OutLevel);

	UFUNCTION(BlueprintCallable, Category = "Level | Utils", meta = (WorldContext = "WorldContextObject"))
	static void UnloadAllSublevel(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Level | Utils", meta = (WorldContext = "WorldContextObject"))
	static void LoadSublevel(UObject* WorldContextObject, const TSoftObjectPtr<UWorld>& SublevelToLoad);
};
