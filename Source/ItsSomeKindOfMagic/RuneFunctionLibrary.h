#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "RuneFunctionLibrary.generated.h"

UCLASS()
class ITSSOMEKINDOFMAGIC_API URuneFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /** Liest ein CanvasRenderTarget aus und gibt ein Graustufen-Floatarray zurück (0.0–1.0) */
    UFUNCTION(BlueprintCallable, Category = "Rune")
    static void GetCanvasGrayscaleData(UCanvasRenderTarget2D* Canvas, TArray<float>& OutData);
};
