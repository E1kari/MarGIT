// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugRuneCount.h"

UDebugRuneCount::UDebugRuneCount()
{

}

void UDebugRuneCount::IncrementCount(float Confidence)
{
	AverageConfidence = (AverageConfidence * RuneCounter + Confidence) / (RuneCounter + 1);
	if (Confidence < LowestConfidence) LowestConfidence = Confidence;
	else if (Confidence > HighestConfidence) HighestConfidence = Confidence;
	RuneCounter++;
}

void UDebugRuneCount::Initialize(FPredictionResult PredictionResult)
{
	RuneName = PredictionResult.PredictedLabel;
	AverageConfidence = LowestConfidence = HighestConfidence = PredictionResult.Confidence;
	RuneCounter = 1;
}

UDebugRuneCount* UDebugRuneCount::CreateDebugRuneCountObject(UObject* WorldContextObject, FPredictionResult PredictionResult)
{
	UDebugRuneCount* NewObj = NewObject<UDebugRuneCount>(WorldContextObject);
	NewObj->Initialize(PredictionResult);
	return NewObj;
}