// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "AudioManagerSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FManagedSFX
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SFX")
	TWeakObjectPtr<UAudioComponent> AudioComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SFX")
	bool bIsSpell = false;

	/** Ursprünglicher, vom Aufrufer gewünschter Basis-Volume (vor Master/Group) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SFX")
	float BaseVolume = 1.f;
};

UCLASS()
class ITSSOMEKINDOFMAGICMP_API UAudioManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "AudioManager")
	void SetBaseTrack(USoundBase* BaseTrack);

	UFUNCTION(BlueprintCallable, Category = "AudioManager")
	void PlayMusic(USoundBase* Track, float FadeInTime = 1.0f, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "AudioManager")
	void StopMusic(float FadeOutTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "AudioManager")
	void CrossfadeMusic(USoundBase* NewTrack, float FadeTime = 1.0f, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "AudioManager")
	void FadeMusicLayer(FName LayerName, float Volume);

	UFUNCTION(BlueprintCallable, Category = "AudioManager", meta = (ReturnDisplayName = "Audio Component", DisplayName = "Play SFX2D"))
	UAudioComponent* PlaySFX2D(USoundBase* Sfx, bool bIsSpell = false, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "AudioManager", meta=(ReturnDisplayName = "Audio Component", DisplayName = "Play SFX At Location"))
	UAudioComponent* PlaySFXAtLocation(USoundBase* Sfx, FVector Location, USoundAttenuation* Attenuation = nullptr, bool bIsSpell = false, float Volume = 1.0f, bool bAutoDestroy = true);

	UFUNCTION(BlueprintCallable, Category = "AudioManager", meta = (ReturnDisplayName = "Audio Component", DisplayName = "Play SFX Attached"))
	UAudioComponent* PlaySFXAttached(USoundBase* Sfx, AActor* Target, USoundAttenuation* Attenuation = nullptr, bool bIsSpell = false, float Volume = 1.0f, bool bAutoDestroy = true);

	UFUNCTION(BlueprintCallable, Category = "AudioManager")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "AudioManager")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "AudioManager")
	void SetSpellSFXVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "AudioManager")
	void SetNonSpellSFXVolume(float Volume);

private:
	UPROPERTY()
	UAudioComponent* CurrentMusicComponent = nullptr;
	UPROPERTY() TArray<FManagedSFX> ManagedSFX;

	void HandleOldMusicFadeOut(UAudioComponent* OldComp, float Delay);
	void SetManagedSFXVolume();

	float MasterVolume = 1.0f;
	float MusicVolume = 1.0f;
	float SpellSFXVolume = 1.0f;
	float NonSpellSFXVolume = 1.0f;
};