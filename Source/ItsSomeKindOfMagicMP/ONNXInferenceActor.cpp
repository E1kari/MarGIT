// ONNXInferenceActor.cpp

#include "ONNXInferenceActor.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"      // Für GEngine->AddOnScreenDebugMessage
#include <cfloat>              // Für FLT_MAX

AONNXInferenceActor::AONNXInferenceActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AONNXInferenceActor::BeginPlay()
{
    Super::BeginPlay();

    if (!ModelData)
    {
        UE_LOG(LogTemp, Error, TEXT("ModelData is not set! Please assign a valid model asset in the Editor."));
        return;
    }

    // CPU‐Runtime beschaffen
    TWeakInterfacePtr<INNERuntimeCPU> Runtime = UE::NNE::GetRuntime<INNERuntimeCPU>(TEXT("NNERuntimeORTCpu"));
    if (!Runtime.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot find runtime 'NNERuntimeORTCpu'. Please enable the corresponding plugin."));
        return;
    }

    // Modell erstellen
    TSharedPtr<UE::NNE::IModelCPU> Model = Runtime->CreateModelCPU(ModelData);
    if (!Model.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create the model."));
        return;
    }

    // Modellinstanz erstellen
    ModelInstance = Model->CreateModelInstanceCPU();
    if (!ModelInstance.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create the model instance."));
    }
}

FPredictionResult AONNXInferenceActor::RunInferenceBP(const TArray<float>& InputData)
{
    FPredictionResult Result;
    Result.bSuccess = false;

    if (!ModelInstance.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Model instance is invalid."));
        return Result;
    }

    if (InputData.Num() != 4096)
    {
        UE_LOG(LogTemp, Error, TEXT("Input data size is incorrect. Expected: 4096, Received: %d"), InputData.Num());
        return Result;
    }

    // Konkrete Input-Shape setzen (Batch=1, 64×64×1)
    TArray<uint32> ConcreteDims = { 1, 64, 64, 1 };
    UE::NNE::FTensorShape InputShape = UE::NNE::FTensorShape::Make(ConcreteDims);
    if (ModelInstance->SetInputTensorShapes({ InputShape }) != UE::NNE::EResultStatus::Ok)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to set input tensor shapes."));
        return Result;
    }

    // ** Neu: dynamisch die Anzahl der Output-Klassen ermitteln **
    auto OutputDescs = ModelInstance->GetOutputTensorDescs();
    check(OutputDescs.Num() == 1);
    const auto ShapeData = OutputDescs[0].GetShape().GetData();
    int32 NumModelClasses = 1;
    if (ShapeData.Num() >= 2 && ShapeData[1] > 0)
    {
        NumModelClasses = ShapeData[1];
    }
    else if (RuneMappings.Num() > 0)
    {
        NumModelClasses = RuneMappings.Num();
    }
    else
    {
        NumModelClasses = 2;
    }

    // Input-Binding erstellen
    UE::NNE::FTensorBindingCPU InputTensor;
    InputTensor.Data = const_cast<float*>(InputData.GetData());
    InputTensor.SizeInBytes = InputData.Num() * sizeof(float);
    TArray<UE::NNE::FTensorBindingCPU> Inputs = { InputTensor };

    // Output-Binding mit exakt so vielen Floats wie das Modell liefert
    UE::NNE::FTensorBindingCPU OutputTensor;
    OutputTensor.SizeInBytes = NumModelClasses * sizeof(float);
    OutputTensor.Data = FMemory::Malloc(OutputTensor.SizeInBytes);
    TArray<UE::NNE::FTensorBindingCPU> Outputs = { OutputTensor };

    // Inferenz synchron ausführen
    if (ModelInstance->RunSync(Inputs, Outputs) != UE::NNE::EResultStatus::Ok)
    {
        UE_LOG(LogTemp, Error, TEXT("Model inference failed."));
        FMemory::Free(OutputTensor.Data);
        return Result;
    }

    // Output auswerten
    Result = ProcessOutput(Outputs, NumModelClasses);

    // Puffer wieder freigeben
    FMemory::Free(OutputTensor.Data);
    return Result;
}

FPredictionResult AONNXInferenceActor::ProcessOutput(const TArray<UE::NNE::FTensorBindingCPU>& Outputs, int32 NumClasses)
{
    FPredictionResult Result;
    Result.bSuccess = false;
    Result.PredictedIndex = -1;
    Result.Confidence = 0.f;
    Result.PredictedLabel = TEXT("Unknown");

    if (Outputs.Num() > 0 && Outputs[0].Data)
    {
        float* Predictions = static_cast<float*>(Outputs[0].Data);

        // Spitzenklasse finden
        int32 PredictedClass = -1;
        float BestScore = -FLT_MAX;
        for (int32 i = 0; i < NumClasses; ++i)
        {
            if (Predictions[i] > BestScore)
            {
                BestScore = Predictions[i];
                PredictedClass = i;
            }
        }

        // Falls unter Threshold, auf Unknown zurücksetzen
        if (BestScore < ConfidenceThreshold)
        {
            int32 UnknownIndex = -1;
            for (const FRuneMapping& M : RuneMappings)
            {
                if (M.RuneName.Equals(TEXT("Unknown"), ESearchCase::IgnoreCase))
                {
                    UnknownIndex = M.Index;
                    break;
                }
            }
            PredictedClass = UnknownIndex;
            BestScore = 0.f;  // auf definierten Default zurücksetzen
        }

        // Ergebnis füllen
        Result.PredictedIndex = PredictedClass;
        Result.Confidence = BestScore;
        Result.bSuccess = true;

        // Label aus Mapping suchen
        bool bFound = false;
        for (const FRuneMapping& M : RuneMappings)
        {
            if (M.Index == PredictedClass)
            {
                Result.PredictedLabel = M.RuneName;
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            Result.PredictedLabel = TEXT("Unknown");
        }

        // Loggen / On-Screen-Debug
        FString LogStr = FString::Printf(
            TEXT("Predicted Rune: %s (Index: %d, Confidence: %.2f)"),
            *Result.PredictedLabel, Result.PredictedIndex, Result.Confidence);
        UE_LOG(LogTemp, Log, TEXT("%s"), *LogStr);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, LogStr);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid output data received from the model."));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
                TEXT("No valid output data received from the model."));
        }
    }

    return Result;
}
