// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "TimerManager.h"

static int32 DebugWeaponsDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing (
                        TEXT("COOP.DebugWeapons"),
                        DebugWeaponsDrawing,
                        TEXT("Draw Debug Lines for Weapons"),
                        ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;
    
    MuzzleSocketName = "MuzzleSocket";
    TracerBeamEndName = "BeamEnd";
    
    BaseDamage = 20.0f;
    
    RateOfFire = 600;

}

void ASWeapon::BeginPlay()
{
    Super::BeginPlay();
    
    TimeBetweenShots =  60.0f / RateOfFire;
}

void ASWeapon::Fire()
{
    //Trace the world, form pawn eyes to crosshair location
    AActor* MyOwner = GetOwner();
    if (MyOwner)
    {
        FVector EyeLocation;
        FRotator EyeRotation;
        MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
        
        FVector ShotDirection = EyeRotation.Vector();
        
        FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * 10000);
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(MyOwner);
        QueryParams.AddIgnoredActor(this);
        QueryParams.bTraceComplex = true;
        QueryParams.bReturnPhysicalMaterial = true;
        
        // Partcile "Target" parameter
        FVector TracerEndPoint = TraceEnd;
        
        FHitResult Hit;
        if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
        {
            // Get surface type
            EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
            
            // Apply damage
            float CurrentDamage = BaseDamage;
            if (SurfaceType == SURFACE_FLESH_VULNERABLE)
            {
                CurrentDamage *= 2.0f;
            }
            AActor* HitActor = Hit.GetActor();
            UGameplayStatics::ApplyPointDamage(HitActor, CurrentDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
            
            // Show effects
            UParticleSystem* SelectedEffect = nullptr;
            switch (SurfaceType)
            {
                case SURFACE_FLESH_DEFAULT:
                case SURFACE_FLESH_VULNERABLE:
                    SelectedEffect = FleshImpactEffect;
                    break;
                default:
                    SelectedEffect = DefaultImpactEffect;
                    break;
            }
            
            if (SelectedEffect)
            {
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
            }
            
            // Save impact point as end point
            TracerEndPoint = Hit.ImpactPoint;
        }
        
        if (DebugWeaponsDrawing > 0)
        {
            DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
        }
        
        PlayFireEffects(TracerEndPoint);
        
        LastFireTime = GetWorld()->TimeSeconds;
    }
    
    
}

void ASWeapon::PlayFireEffects(FVector TraceEnd)
{
    if (MuzzleEffect)
    {
        UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
    }
    
    if (TracerEffect)
    {
        FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
        UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
        if (TracerComp)
        {
            TracerComp->SetVectorParameter(TracerBeamEndName, TraceEnd);
        }
    }
    
    APawn* MyOwner = Cast<APawn>(GetOwner());
    if (MyOwner)
    {
        APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
        if (PC)
        {
            PC->ClientPlayCameraShake(FireCamShake);
        }
    }
}

void ASWeapon::StartFire()
{
    float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
    
    GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}
