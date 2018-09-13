// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

// Contains information of a single hitscan weapon linetrace
USTRUCT()
struct FHitScanTrace
{
    GENERATED_BODY()

public:

    UPROPERTY()
    FVector_NetQuantize TraceFrom;
    
    UPROPERTY()
    FVector_NetQuantize TraceTo;
};

UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();
    
protected:
    
    virtual void BeginPlay() override;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* MeshComp;
    
    void PlayFireEffects(FVector TraceEnd);
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    TSubclassOf<UDamageType> DamageType;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FName MuzzleSocketName;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* MuzzleEffect;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* DefaultImpactEffect;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* FleshImpactEffect;
    
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FName TracerBeamEndName;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    UParticleSystem* TracerEffect;
    
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<UCameraShake> FireCamShake;
    
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float BaseDamage;
    
    void Fire();

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerFire();
    
    FTimerHandle TimerHandle_TimeBetweenShots;
    
    float LastFireTime;
    
    /* RPM - Bullets per minute fired by weapon */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float RateOfFire;
    
    // Dervied from RateOfFire
    float TimeBetweenShots;

    UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
    FHitScanTrace HitScanTrace;

    UFUNCTION()
    void OnRep_HitScanTrace();
    
public:
    
    void StartFire();
    
    void StopFire();
};
