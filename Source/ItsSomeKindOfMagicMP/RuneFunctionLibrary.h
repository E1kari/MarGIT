#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "DebugRuneCount.h"
#include "RuneFunctionLibrary.generated.h"

UCLASS()
class ITSSOMEKINDOFMAGICMP_API URuneFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /** Liest ein CanvasRenderTarget aus und gibt ein Graustufen-Floatarray zurück (0.0–1.0) */
    UFUNCTION(BlueprintCallable, Category = "Rune")
    static void GetCanvasGrayscaleData(UCanvasRenderTarget2D* Canvas, TArray<float>& OutData);

    /** Speichert ein CanvasRenderTarget als PNG-Datei */
    UFUNCTION(BlueprintCallable, Category = "Rune")
    static bool SaveCanvasRenderTargetToPNG(UCanvasRenderTarget2D* Canvas, const FString& FolderPath, const FString& FileName);

    /** Speichert Rune- und Spell-Statistiken in einer TXT-Datei im Saved-Ordner */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    static bool SaveDebugStatsToText(
        const TArray<UDebugRuneCount*>& RuneCounts,
        const TMap<FString, int32>& SpellCounts,
        const FString& FileName = TEXT("Stats.txt")
    );
};