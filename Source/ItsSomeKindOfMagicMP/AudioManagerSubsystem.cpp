// Fill out your copyright notice in the Description page of Project Settings.


#include "AudioManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UAudioManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UAudioManagerSubsystem::Deinitialize()
{
	if (CurrentMusicComponent)
	{
		CurrentMusicComponent->Stop();
		CurrentMusicComponent->DestroyComponent();
		CurrentMusicComponent = nullptr;
	}
	Super::Deinitialize();
}

void UAudioManagerSubsystem::SetBaseTrack(USoundBase* BaseTrack)
{
	if (BaseTrack)
	{
		UWorld* World = GetWorld();
		if (!World) return;

		CurrentMusicComponent = UGameplayStatics::SpawnSound2D(World, BaseTrack, 1.0f, 1.0f, 0.0f, nullptr, true, true);
	}
}

void UAudioManagerSubsystem::PlayMusic(USoundBase* Track, float FadeInTime, float Volume)
{
	if (!Track) return;

	if (CurrentMusicComponent)
	{
		CurrentMusicComponent->Stop();
		CurrentMusicComponent->DestroyComponent();
		CurrentMusicComponent = nullptr;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	CurrentMusicComponent = UGameplayStatics::SpawnSound2D(World, Track, 1.0f, 1.0f, 0.0f, nullptr, true, true);
	if (CurrentMusicComponent)
	{
		CurrentMusicComponent->Play();
		CurrentMusicComponent->FadeIn(FadeInTime, Volume);
	}
}

void UAudioManagerSubsystem::StopMusic(float FadeOutTime)
{
	if (!CurrentMusicComponent) return;

	CurrentMusicComponent->FadeOut(FadeOutTime, 0.0f);
	HandleOldMusicFadeOut(CurrentMusicComponent, FadeOutTime);
	CurrentMusicComponent = nullptr;
}

void UAudioManagerSubsystem::CrossfadeMusic(USoundBase* NewTrack, float FadeTime, float Volume)
{
	if (!NewTrack) return;
	UWorld* World = GetWorld();
	if (!World) return;

	UAudioComponent* OldComp = CurrentMusicComponent;

	CurrentMusicComponent = UGameplayStatics::SpawnSound2D(World, NewTrack, 1.0f, 1.0f, 0.0f, nullptr, true, true);
	if (CurrentMusicComponent)
	{
		CurrentMusicComponent->Play();
		CurrentMusicComponent->FadeIn(FadeTime, Volume);
	}

	if (OldComp)
	{
		OldComp->FadeOut(FadeTime, 0.0f);
		HandleOldMusicFadeOut(OldComp, FadeTime);
	}
}

void UAudioManagerSubsystem::FadeMusicLayer(FName LayerName, float Volume)
{
	if (!CurrentMusicComponent) return;
	CurrentMusicComponent->SetFloatParameter(LayerName, Volume);
}

void UAudioManagerSubsystem::PlaySFX2D(USoundBase* Sfx, float Volume)
{
	if (!Sfx) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::SpawnSound2D(World, Sfx, Volume, 0.0f, 0.0f, nullptr, false, true);
}

void UAudioManagerSubsystem::PlaySFXAtLocation(USoundBase* Sfx, FVector Location, USoundAttenuation* Attenuation, float Volume)
{
	if (!Sfx) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::SpawnSoundAtLocation(World, Sfx, Location, FRotator::ZeroRotator, Volume, 0.0f, 0.0f, Attenuation, nullptr, true);
}

void UAudioManagerSubsystem::HandleOldMusicFadeOut(UAudioComponent* OldComp, float Delay)
{
	if (!OldComp) return;
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateLambda([OldComp]()
			{
				OldComp->Stop();
				OldComp->DestroyComponent();
			}),
		Delay,
		false
	);
}
