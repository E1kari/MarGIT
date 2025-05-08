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

    int32 CanvasWidth = Canvas->SizeX;
    int32 CanvasHeight = Canvas->SizeY;
    int32 Size = CanvasWidth * CanvasHeight;

    // OutData vorbereiten
    OutData.Empty(Size);
    OutData.SetNumUninitialized(Size);

    for (int32 i = 0; i < Size; ++i)
    {
        const FColor& Pixel = Pixels[i];
        float Gray = Pixel.R / 255.0f;
        OutData[i] = Gray;
    }
}

bool URuneFunctionLibrary::SaveCanvasRenderTargetToPNG(UCanvasRenderTarget2D* Canvas, const FString& FolderPath, const FString& FileName)
{
    if (!Canvas)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveCanvasToPNG: Canvas is null."));
        return false;
    }

    // RenderTarget-Ressource holen
    FTextureRenderTargetResource* Resource = Canvas->GameThread_GetRenderTargetResource();
    if (!Resource)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveCanvasToPNG: Could not get render target resource."));
        return false;
    }

    // Pixel auslesen
    TArray<FColor> Pixels;
    Resource->ReadPixels(Pixels);

    int32 Width = Canvas->SizeX;
    int32 Height = Canvas->SizeY;
    int32 NumPixels = Width * Height;
    if (Pixels.Num() != NumPixels)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("SaveCanvasToPNG: Pixel count mismatch: got %d, expected %d"),
            Pixels.Num(), NumPixels);
    }

    // PNG-Wrapper initialisieren
    IImageWrapperModule& ImgModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
    TSharedPtr<IImageWrapper> Wrapper = ImgModule.CreateImageWrapper(EImageFormat::PNG);
    Wrapper->SetRaw(
        reinterpret_cast<const uint8*>(Pixels.GetData()),
        Pixels.Num() * sizeof(FColor),
        Width, Height,
        ERGBFormat::BGRA, 8
    );

    // komprimieren
    const TArray64<uint8>& PngData = Wrapper->GetCompressed(100);

    // absoluten Pfad zusammenbauen (<ProjectDir>/FolderPath)
    FString AbsoluteFolder = FPaths::ProjectDir() / FolderPath;
    IPlatformFile& PFile = FPlatformFileManager::Get().GetPlatformFile();

    if (!PFile.DirectoryExists(*AbsoluteFolder))
    {
        PFile.CreateDirectoryTree(*AbsoluteFolder);
    }

    FString FullPath = AbsoluteFolder / (FileName + TEXT(".png"));

    // speichern
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
