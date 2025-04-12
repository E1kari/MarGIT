#include "RuneFunctionLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/CanvasRenderTarget2D.h"

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

    // Erwarte, dass der Canvas 64x64 Pixel groß ist (=> 4096 Werte, da 1x1x64x64)
    int32 ExpectedWidth = 64;
    int32 ExpectedHeight = 64;
    int32 ExpectedNumElements = 1 * 1 * ExpectedWidth * ExpectedHeight;  // 4096

    int32 CanvasWidth = Canvas->SizeX;
    int32 CanvasHeight = Canvas->SizeY;
    int32 Size = CanvasWidth * CanvasHeight;

    // Logge, ob die Canvas-Dimensionen den Erwartungen entsprechen
    if (Size != ExpectedNumElements)
    {
        UE_LOG(LogTemp, Warning, TEXT("Canvas dimensions are %dx%d (%d elements), but expected %dx%d (%d elements)."),
            CanvasWidth, CanvasHeight, Size, ExpectedWidth, ExpectedHeight, ExpectedNumElements);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Canvas dimensions are %dx%d, matching the expected tensor shape 1x1x64x64."),
            CanvasWidth, CanvasHeight);
    }

    // Fülle das OutData-Array
    OutData.Empty(Size);
    OutData.SetNumUninitialized(Size);

    for (int32 i = 0; i < Size; ++i)
    {
        const FColor& Pixel = Pixels[i];
        // Da die Rune weiß gezeichnet ist, nehmen wir den Rot-Kanal als Grauwert (normalisiert auf 0 bis 1)
        float Gray = Pixel.R / 255.0f;
        OutData[i] = Gray;
    }
}

