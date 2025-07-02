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

	CurrentMusicComponent = UGameplayStatics::SpawnSound2D(World, Track, 0.0f);
	if (CurrentMusicComponent)
	{
		CurrentMusicComponent->SetVolumeMultiplier(0.0f);
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

	CurrentMusicComponent = UGameplayStatics::SpawnSound2D(World, NewTrack, 0.0f);
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
