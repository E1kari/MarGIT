#include "ONNXInferenceActor.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"  // Für GEngine->AddOnScreenDebugMessage
#include <cfloat>            // Für FLT_MAX

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

    // CPU-Runtime abrufen (hier: "NNERuntimeORTCpu")
    TWeakInterfacePtr<INNERuntimeCPU> Runtime = UE::NNE::GetRuntime<INNERuntimeCPU>(FString("NNERuntimeORTCpu"));
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
    TSharedPtr<UE::NNE::IModelInstanceCPU> Instance = Model->CreateModelInstanceCPU();
    if (!Instance.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create the model instance."));
        return;
    }

    ModelInstance = Instance;
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

    // Überprüfen: Anzahl der Eingabedaten muss der erwarteten Anzahl entsprechen (z. B. 4096 Werte)
    if (InputData.Num() != 4096)
    {
        UE_LOG(LogTemp, Error, TEXT("Input data size is incorrect. Expected: 4096, Received: %d"), InputData.Num());
        return Result;
    }

    // (Optional) Logge die erwarteten Input-Tensorbeschreibungen
    {
        TConstArrayView<UE::NNE::FTensorDesc> InputDescs = ModelInstance->GetInputTensorDescs();
        for (int32 i = 0; i < InputDescs.Num(); ++i)
        {
            UE_LOG(LogTemp, Log, TEXT("Model Input %d: Name = %s"), i, *InputDescs[i].GetName());
            const UE::NNE::FSymbolicTensorShape& SymbolicShape = InputDescs[i].GetShape();
            TConstArrayView<int32> ShapeData = SymbolicShape.GetData();
            FString DimensionString;
            for (int32 j = 0; j < ShapeData.Num(); ++j)
            {
                DimensionString.Append(FString::Printf(TEXT("%d "), ShapeData[j]));
            }
            UE_LOG(LogTemp, Log, TEXT("Input Tensor %d Shape: %s"), i, *DimensionString);
        }
    }

    // Erstelle das konkrete Eingabe-Shape. Da das Modell "-1 64 64 1" erwartet (wobei -1 für variable Batch-Größe steht),
    // ersetzen wir es durch 1: also {1, 64, 64, 1}.
    TArray<uint32> ConcreteDims = { 1, 64, 64, 1 };
    UE::NNE::FTensorShape InputShape = UE::NNE::FTensorShape::Make(ConcreteDims);

    // Setze die Input-Tensor-Shape. Wenn dies fehlschlägt, gib das Ergebnis zurück.
    auto SetStatus = ModelInstance->SetInputTensorShapes({ InputShape });
    if (SetStatus != UE::NNE::EResultStatus::Ok)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to set input tensor shapes."));
        return Result;
    }

    // Erstelle das Input-Binding.
    UE::NNE::FTensorBindingCPU InputTensor;
    InputTensor.Data = const_cast<float*>(InputData.GetData());
    InputTensor.SizeInBytes = InputData.Num() * sizeof(float);
    TArray<UE::NNE::FTensorBindingCPU> Inputs;
    Inputs.Add(InputTensor);

    // Bestimme die Anzahl der Klassen anhand der Länge des Mappings (Fallback: 2)
    int32 NumClasses = (RuneMappings.Num() > 0) ? RuneMappings.Num() : 2;
    UE::NNE::FTensorBindingCPU OutputTensor;
    OutputTensor.SizeInBytes = NumClasses * sizeof(float);
    OutputTensor.Data = FMemory::Malloc(OutputTensor.SizeInBytes);
    TArray<UE::NNE::FTensorBindingCPU> Outputs;
    Outputs.Add(OutputTensor);

    auto RunStatus = ModelInstance->RunSync(Inputs, Outputs);
    if (RunStatus != UE::NNE::EResultStatus::Ok)
    {
        UE_LOG(LogTemp, Error, TEXT("Model inference failed."));
        FMemory::Free(OutputTensor.Data);
        return Result;
    }

    // Verarbeite den Output und erhalte ein FPredictionResult
    Result = ProcessOutput(Outputs);

    FMemory::Free(OutputTensor.Data);
    return Result;
}

FPredictionResult AONNXInferenceActor::ProcessOutput(const TArray<UE::NNE::FTensorBindingCPU>& Outputs)
{
    FPredictionResult Result;
    Result.bSuccess = false;
    Result.PredictedIndex = -1;
    Result.Confidence = 0.f;
    Result.PredictedLabel = TEXT("Unknown");

    if (Outputs.Num() > 0 && Outputs[0].Data != nullptr)
    {
        float* Predictions = static_cast<float*>(Outputs[0].Data);
        int32 NumClasses = (RuneMappings.Num() > 0) ? RuneMappings.Num() : 2;

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

        // Falls die beste Konfidenz unter dem Threshold liegt, suche den Mapping-Eintrag, dessen Name "Unknown" (unabhängig von der Groß-/Kleinschreibung) lautet.
        if (BestScore < ConfidenceThreshold)
        {
            for (const FRuneMapping& Mapping : RuneMappings)
            {
                if (Mapping.RuneName.Equals(TEXT("Unknown"), ESearchCase::IgnoreCase))
                {
                    PredictedClass = Mapping.Index;
                    break;
                }
            }
        }

        Result.PredictedIndex = PredictedClass;
        Result.Confidence = BestScore;
        Result.bSuccess = true;

        // Suche im Mapping nach dem Eintrag, der dem berechneten Index entspricht.
        bool bFoundMapping = false;
        for (const FRuneMapping& Mapping : RuneMappings)
        {
            if (Mapping.Index == PredictedClass)
            {
                Result.PredictedLabel = Mapping.RuneName;
                bFoundMapping = true;
                break;
            }
        }
        if (!bFoundMapping)
        {
            // Fallback: Setze "Unknown", falls kein Mapping für den Index gefunden wurde.
            Result.PredictedLabel = TEXT("Unknown");
        }

        // Anzeige als Log und On-Screen-Message
        FString ResultString = FString::Printf(TEXT("Predicted Rune: %s (Index: %d, Confidence: %.2f)"),
            *Result.PredictedLabel, Result.PredictedIndex, Result.Confidence);
        UE_LOG(LogTemp, Log, TEXT("%s"), *ResultString);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, ResultString);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid output data received from the model."));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("No valid output data received from the model."));
        }
    }

    return Result;
}
