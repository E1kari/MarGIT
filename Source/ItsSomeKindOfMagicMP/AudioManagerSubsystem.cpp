// Fill out your copyright notice in the Description page of Project Settings.


#include "AudioManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

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

void UAudioManagerSubsystem::PlaySFX2D(USoundBase* Sfx, bool bIsSpell, float Volume)
{
	if (!Sfx) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::SpawnSound2D(World, Sfx, Volume * MasterVolume * (bIsSpell ? SpellSFXVolume : NonSpellSFXVolume), 1.0f, 0.0f, nullptr, false, true);
}

UAudioComponent* UAudioManagerSubsystem::PlaySFXAtLocation(USoundBase* Sfx, FVector Location, USoundAttenuation* Attenuation, bool bIsSpell, float Volume, bool bAutoDestroy)
{
	if (!Sfx) return nullptr;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	UAudioComponent* SFXAtLocation = UGameplayStatics::SpawnSoundAtLocation(World, Sfx, Location, FRotator::ZeroRotator, Volume * MasterVolume * (bIsSpell ? SpellSFXVolume : NonSpellSFXVolume), 1.0f, 0.0f, Attenuation, nullptr, true);
	SFXAtLocation->bAutoDestroy = bAutoDestroy;
	return SFXAtLocation;
}

void UAudioManagerSubsystem::SetMasterVolume(float Volume)
{
	MasterVolume = Volume;
	CurrentMusicComponent->SetVolumeMultiplier(MasterVolume * MusicVolume);
}

void UAudioManagerSubsystem::SetMusicVolume(float Volume)
{
	MusicVolume = Volume;
	CurrentMusicComponent->SetVolumeMultiplier(MasterVolume * MusicVolume);
}

void UAudioManagerSubsystem::SetSpellSFXVolume(float Volume)
{
	SpellSFXVolume = Volume;
}

void UAudioManagerSubsystem::SetNonSpellSFXVolume(float Volume)
{
	NonSpellSFXVolume = Volume;
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
