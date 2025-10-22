// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelFunctionLibrary.h"
#include "Misc/Paths.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "Engine/LevelStreaming.h"
#include "Engine/Engine.h"

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

void ULevelFunctionLibrary::GetAllSublevel(const UObject* WorldContextObject, TArray<TSoftObjectPtr<UWorld>>& OutLevel)
{
    OutLevel.Empty();
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetAllSublevel: WorldRef konnte nicht geladen werden"));
        return;
    }
    
    const TArray<ULevelStreaming*>& Streaming = World->GetStreamingLevels();
    for (ULevelStreaming* LS : Streaming)
    {
        if (!LS) continue;

        // In 5.x ist das die saubere Art, ans Asset zu kommen
        TSoftObjectPtr<UWorld> LevelAsset = LS->GetWorldAsset();
        if (!LevelAsset.IsNull() || LevelAsset.ToSoftObjectPath().IsValid())
        {
            OutLevel.AddUnique(LevelAsset);
        }
        else
        {
            // Fallback über Paketname (nützlich bei nicht geladenem Asset)
#if ENGINE_MAJOR_VERSION >= 5
            const FName PkgName = LS->GetWorldAssetPackageFName(); // oder GetPackageNameToLoad()
#else
            const FName PkgName = LS->GetWorldAssetPackageFName();
#endif
            if (!PkgName.IsNone())
            {
                OutLevel.AddUnique(TSoftObjectPtr<UWorld>(FSoftObjectPath(PkgName.ToString())));
            }
        }
    }
}

void ULevelFunctionLibrary::UnloadAllSublevel(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    if (!World) return;

    for (ULevelStreaming* LS : World->GetStreamingLevels())
    {
        if (LS && LS->IsLevelLoaded())
        {
            LS->SetShouldBeLoaded(false);
            LS->SetShouldBeVisible(false);
        }
    }

    World->FlushLevelStreaming(EFlushLevelStreamingType::Full);
}

void ULevelFunctionLibrary::LoadSublevel(UObject* WorldContextObject, const TSoftObjectPtr<UWorld>& SublevelToLoad)
{
    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    if (!World) return;

    FSoftObjectPath SoftPath = SublevelToLoad.ToSoftObjectPath();
    FString AssetName = SoftPath.GetAssetName();
    FName LevelName = FName(*AssetName);

    if (ULevelStreaming* LS = UGameplayStatics::GetStreamingLevel(World, LevelName))
    {
        LS->SetShouldBeLoaded(true);
        LS->SetShouldBeVisible(true);
    }
    else
    {
        FLatentActionInfo Dummy;
        UGameplayStatics::LoadStreamLevel(WorldContextObject, LevelName, true, true, Dummy);
    }

    World->FlushLevelStreaming(EFlushLevelStreamingType::Full);
}



