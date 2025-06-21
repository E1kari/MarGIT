#include "RuneFunctionLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

void URuneFunctionLibrary::GetCanvasGrayscaleData(UCanvasRenderTarget2D* Canvas, TArray<float>& OutData)
{
    if (!Canvas)
    {
        UE_LOG(LogTemp, Error, TEXT("Canvas is null."));
        return;
    }

    FTextureRenderTargetResource* Resource = Canvas->GameThread_GetRenderTargetResource();
    if (!Resource)
    {
        UE_LOG(LogTemp, Error, TEXT("Could not get render target resource from Canvas."));
        return;
    }

    TArray<FColor> Pixels;
    Resource->ReadPixels(Pixels);

    int32 Width = Canvas->SizeX;
    int32 Height = Canvas->SizeY;
    int32 Size = Width * Height;

    OutData.Empty(Size);
    OutData.SetNumUninitialized(Size);

    for (int32 i = 0; i < Size; ++i)
    {
        const FColor& Pixel = Pixels[i];
        OutData[i] = Pixel.R / 255.0f;
    }
}

bool URuneFunctionLibrary::SaveCanvasRenderTargetToPNG(UCanvasRenderTarget2D* Canvas, const FString& FolderPath, const FString& FileName)
{
    if (!Canvas)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveCanvasToPNG: Canvas is null."));
        return false;
    }

    FTextureRenderTargetResource* Resource = Canvas->GameThread_GetRenderTargetResource();
    if (!Resource)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveCanvasToPNG: Could not get render target resource."));
        return false;
    }

    TArray<FColor> Pixels;
    Resource->ReadPixels(Pixels);

    int32 Width = Canvas->SizeX;
    int32 Height = Canvas->SizeY;
    if (Pixels.Num() != Width * Height)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("SaveCanvasToPNG: Pixel count mismatch: got %d, expected %d"),
            Pixels.Num(), Width * Height);
    }

    IImageWrapperModule& ImgModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
    TSharedPtr<IImageWrapper> Wrapper = ImgModule.CreateImageWrapper(EImageFormat::PNG);
    Wrapper->SetRaw(
        reinterpret_cast<const uint8*>(Pixels.GetData()),
        Pixels.Num() * sizeof(FColor),
        Width, Height,
        ERGBFormat::BGRA, 8
    );

    const TArray64<uint8>& PngData = Wrapper->GetCompressed(100);

    // Pfad im Saved-Ordner
    FString AbsoluteFolder = FPaths::ProjectSavedDir();

    // Datei-Pfad
    FString FullPath = AbsoluteFolder / FileName;

    if (FFileHelper::SaveArrayToFile(PngData, *FullPath))
    {
        UE_LOG(LogTemp, Log, TEXT("Saved PNG to: %s"), *FullPath);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save PNG to: %s"), *FullPath);
        return false;
    }
}

bool URuneFunctionLibrary::SaveDebugStatsToText(
    const TArray<UDebugRuneCount*>& RuneCounts,
    const TMap<FString, int32>& SpellCounts,
    const FString& FileName
)
{
    // Projekt-Ordner Pfad und DebugFiles-Ordner
    FString DebugFolder = FPaths::ProjectDir() / TEXT("DebugFiles");
    IPlatformFile& PFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PFile.DirectoryExists(*DebugFolder))
    {
        PFile.CreateDirectoryTree(*DebugFolder);
    }

    const FString FilePath = DebugFolder / FileName;

    // Inhalt zusammensetzen
    FString OutString;
    OutString += TEXT("=== Rune Usage Stats ===\n\n");
    for (UDebugRuneCount* Entry : RuneCounts)
    {
        if (!Entry) continue;
        OutString += FString::Printf(
            TEXT("Rune: %-15s | Count: %-4d | AvgConf: %.3f | LowConf: %.3f | HighConf: %.3f\n"),
            *Entry->RuneName,
            Entry->RuneCounter,
            Entry->AverageConfidence,
            Entry->LowestConfidence,
            Entry->HighestConfidence
        );
    }

    OutString += TEXT("\n=== Spell Usage Stats ===\n\n");
    for (const auto& Elem : SpellCounts)
    {
        OutString += FString::Printf(
            TEXT("Spell: %-15s | Count: %-4d\n"),
            *Elem.Key,
            Elem.Value
        );
    }

    // Datei schreiben
    if (FFileHelper::SaveStringToFile(OutString, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("Debug stats saved to %s"), *FilePath);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save debug stats to %s"), *FilePath);
        return false;
    }
}
