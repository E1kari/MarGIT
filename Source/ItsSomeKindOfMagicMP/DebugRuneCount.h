// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ONNXInferenceActor.h"
#include "DebugRuneCount.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class ITSSOMEKINDOFMAGICMP_API UDebugRuneCount : public UObject
{
	GENERATED_BODY()

public:
	UDebugRuneCount();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DebugValues")
	FString RuneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DebugValues")
	int32 RuneCounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DebugValues")
	float AverageConfidence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DebugValues")
	float LowestConfidence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DebugValues")
	float HighestConfidence;

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void IncrementCount(float Confidence);

	void Initialize(FPredictionResult PredictionResult);

	UFUNCTION(BlueprintCallable, Category = "Debug", meta = (WorldContext = "WorldContextObject"))
	static UDebugRuneCount* CreateDebugRuneCountObject(UObject* WorldContextObject, FPredictionResult PredictionResult);
	
};
