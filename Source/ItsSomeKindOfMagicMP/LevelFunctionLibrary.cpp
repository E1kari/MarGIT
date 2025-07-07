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

FString ULevelFunctionLibrary::GetLevelNameFromReference(const TSoftObjectPtr<UWorld>& WorldReference)
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

