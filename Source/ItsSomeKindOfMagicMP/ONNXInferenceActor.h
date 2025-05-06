#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NNEModelData.h"
#include "NNE.h"
#include "NNERuntimeCPU.h"       // CPU-Modell Schnittstellen
#include "NNERuntimeRunSync.h"    // Für RunSync und Tensorbindings
#include "ONNXInferenceActor.generated.h"

/**
 * FRuneMapping
 *
 * Dieses Struct definiert eine Zuordnung zwischen einem vom Modell zurückgegebenen Index
 * und dem entsprechenden Namen (z. B. "fire", "water", etc.). Die Liste kann im Blueprint
 * beliebig erweitert werden.
 */
USTRUCT(BlueprintType)
struct FRuneMapping
{
    GENERATED_BODY()

    /** Der vom Modell zurückgegebene Index */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rune Mapping")
    int32 Index;

    /** Der zugehörige Rune-Name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rune Mapping")
    FString RuneName;
};

/**
 * FPredictionResult
 *
 * Dieses Struct fasst das Ergebnis der Vorhersage zusammen.
 */
USTRUCT(BlueprintType)
struct FPredictionResult
{
    GENERATED_BODY()

    /** Der ermittelte Label-Name der Rune */
    UPROPERTY(BlueprintReadOnly, Category = "Prediction")
    FString PredictedLabel;

    /** Der ermittelte Index */
    UPROPERTY(BlueprintReadOnly, Category = "Prediction")
    int32 PredictedIndex;

    /** Die Confidence bzw. Wahrscheinlichkeit */
    UPROPERTY(BlueprintReadOnly, Category = "Prediction")
    float Confidence;

    /** War die Inferenz erfolgreich? */
    UPROPERTY(BlueprintReadOnly, Category = "Prediction")
    bool bSuccess;
};

/**
 * AONNXInferenceActor
 *
 * Dieser Actor lädt ein ONNX-Modell (über ein UNNEModelData-Asset) und führt mithilfe
 * des integrierten NNE-Moduls eine Inferenz durch. Das Ergebnis (als FPredictionResult) wird
 * zurückgegeben – so dass du es im Blueprint weiterverwenden kannst.
 */
UCLASS(BlueprintType, Blueprintable)
class ITSSOMEKINDOFMAGICMP_API AONNXInferenceActor : public AActor
{
    GENERATED_BODY()

public:
    AONNXInferenceActor();

protected:
    virtual void BeginPlay() override;

public:
    /**
     * Führt eine Inferenz durch und gibt das Vorhersageergebnis als FPredictionResult zurück.
     * @param InputData Ein Array von Float-Werten, das die Eingabedaten (z. B. aus einer Canvas) enthält.
     */
    UFUNCTION(BlueprintCallable, Category = "Inference")
    FPredictionResult RunInferenceBP(const TArray<float>& InputData);

    /** Vom Editor zuweisbares Modell-Asset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inference")
    TObjectPtr<UNNEModelData> ModelData;

    /** Dynamisch im Blueprint konfigurierbares Mapping von Indizes zu Rune-Namen */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inference")
    TArray<FRuneMapping> RuneMappings;

    /** Confidence-Threshold – wenn der höchste Wahrscheinlichkeitswert unter diesem Wert liegt, wird "Unknown" gewählt. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inference")
    float ConfidenceThreshold = 0.5f; // z. B. Standardwert 0.5

private:
    /** Die für die Inferenz verwendete Modellinstanz */
    TSharedPtr<UE::NNE::IModelInstanceCPU> ModelInstance;

    /** Wandelt den Output des Modells in ein FPredictionResult um und berücksichtigt den Threshold. */
    FPredictionResult ProcessOutput(const TArray<UE::NNE::FTensorBindingCPU>& Outputs, int32 NumClasses);
};