// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelFunctionLibrary.h"
#include "Misc/Paths.h"

bool ULevelFunctionLibrary::IsActorInSublevel(const AActor* Actor, const TSoftObjectPtr<UWorld>& WorldReference)
{
	if (!Actor || !Actor->GetWorld() || !WorldReference.ToSoftObjectPath().IsValid()) return false;

	const FString ActorLevelPackage = GetPathNameAfterDot(Actor->GetLevel()->GetOuter()->GetPathName());

    UE_LOG(LogTemp, Log, TEXT("ActorLevelPackage: %s"), *ActorLevelPackage);

	const FString RefPackage = GetPathNameAfterDot(WorldReference.ToSoftObjectPath().GetAssetPathString());;

    UE_LOG(LogTemp, Log, TEXT("RefPackage: %s"), *RefPackage);

	return ActorLevelPackage.Equals(RefPackage, ESearchCase::IgnoreCase);
}

FString ULevelFunctionLibrary::GetSublevelNameFromReference(const TSoftObjectPtr<UWorld>& WorldReference)
{
    FString FullPath = WorldReference.ToSoftObjectPath().GetAssetPathString();
    return GetPathNameAfterDot(FullPath);
    
}

FString ULevelFunctionLibrary::GetPathNameAfterDot(const FString& AssetPath)
{
    int32 DotIndex = INDEX_NONE;
    if (AssetPath.FindLastChar(TEXT('.'), DotIndex) && DotIndex + 1 < AssetPath.Len())
    {
        return AssetPath.Mid(DotIndex + 1);
    }
    return AssetPath;
}

void ULevelFunctionLibrary::GetAllLoadedSublevelNames(UObject* WorldContextObject, TArray<FString>& OutLevelNames)
{
    OutLevelNames.Empty();
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    for (ULevelStreaming* LS : World->GetStreamingLevels())
    {
        if (LS && LS->IsLevelLoaded())
        {
            FString FullPath = LS->GetWorldAssetPackageName();
            FString Name = GetPathNameAfterDot(FullPath);
            OutLevelNames.AddUnique(Name);
        }
    }
}

void ULevelFunctionLibrary::GetAllLoadedSublevel(UObject* WorldContextObject, TArray<TSoftObjectPtr<UWorld>>& OutLevel)
{
    OutLevel.Empty();
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    for (ULevelStreaming* LS : World->GetStreamingLevels())
    {
        if (LS && LS->IsLevelLoaded())
        {
            TSoftObjectPtr<UWorld> LvlRef = LS->GetWorldAsset();
            OutLevel.AddUnique(LvlRef);
        }
    }
}

bool ULevelFunctionLibrary::IsSublevelLoaded(UObject* WorldContextObject, const TSoftObjectPtr<UWorld>& LevelReference)
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    FString RefPath = LevelReference.ToSoftObjectPath().GetAssetPathString();
    FString RefName = GetPathNameAfterDot(RefPath);

    for (ULevelStreaming* LS : World->GetStreamingLevels())
    {
        FString CurName = GetPathNameAfterDot(LS->GetWorldAssetPackageName());
        if (CurName.Equals(RefName, ESearchCase::IgnoreCase))
        {
            return true;
        }
    }
    return false;
}

void ULevelFunctionLibrary::GetAllSublevel(const TSoftObjectPtr<UWorld>& WorldRef, TArray<TSoftObjectPtr<UWorld>>& OutLevel)
{
    OutLevel.Empty();
    UWorld* World = WorldRef.Get();
    if (!World)
    {
        World = WorldRef.LoadSynchronous();
    }
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetAllSublevel: WorldRef konnte nicht geladen werden"));
        return;
    }
    for (ULevelStreaming* LS : World->GetStreamingLevels())
    {
        TSoftObjectPtr<UWorld> LvlRef = LS->GetWorldAsset();
        OutLevel.AddUnique(LvlRef);
    }
}

