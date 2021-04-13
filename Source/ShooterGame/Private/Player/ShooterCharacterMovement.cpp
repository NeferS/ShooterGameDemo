// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterCharacterMovement.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterPlayerState.h"

//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UShooterCharacterMovement::UShooterCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/**BEGIN: CODE ADDED BY VINCENZO PARRILLA*/
	bWantsToTeleport = false;
	bWantsToJetpack = false;
	JetpackAvailableFuel = JetpackMaxFuel;
	/**END: CODE ADDED BY VINCENZO PARRILLA*/
}

float UShooterCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner)
	{
		if (ShooterCharacterOwner->IsTargeting())
		{
			MaxSpeed *= ShooterCharacterOwner->GetTargetingSpeedModifier();
		}
		if (ShooterCharacterOwner->IsRunning())
		{
			MaxSpeed *= ShooterCharacterOwner->GetRunningSpeedModifier();
		}
	}

	return MaxSpeed;
}

/**BEGIN: CODE ADDED BY VINCENZO PARRILLA*/
float UShooterCharacterMovement::GetMaxAcceleration() const
{
	float NewMaxAcceleration = Super::GetMaxAcceleration();

	if (bWantsToJetpack)
		NewMaxAcceleration *= JetpackAccelerationModifier;

	return NewMaxAcceleration;
}

FVector UShooterCharacterMovement::NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const
{
	if (bWantsToJetpack)
		return Super::NewFallVelocity(InitialVelocity, (Gravity * GravityScaleWhileJetpack), DeltaTime);

	return Super::NewFallVelocity(InitialVelocity, Gravity, DeltaTime);
}

void UShooterCharacterMovement::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	//TELEPORT
	if (bWantsToTeleport && (CharacterOwner->GetLocalRole() == ROLE_Authority || CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy))
	{
		const FVector OldLocation = PawnOwner->GetActorLocation();
		const FVector NewLocation = OldLocation + PawnOwner->GetActorForwardVector() * TeleportDistance;
		PawnOwner->SetActorLocation(NewLocation, true);
		if (FVector::Dist(OldLocation, PawnOwner->GetActorLocation()) > 10.0f)
		{
			if (GetPawnOwner()->IsLocallyControlled())
				UGameplayStatics::SpawnSoundAtLocation(GetWorld(), TeleportSound, NewLocation);
			if (GetOwner()->HasAuthority())
				MulticastPlaySound(TeleportSound, PawnOwner->GetActorLocation());
		}
		bWantsToTeleport = false;
	}

	//JETPACK
	if (bWantsToJetpack && CanUseJetpack())
	{
		JetpackAvailableFuel = FMath::Clamp(JetpackAvailableFuel - JetpackFuelConsumeRate * DeltaSeconds, 0.0f, JetpackMaxFuel);
		if (PawnOwner->IsLocallyControlled())
			MoveDirection = PawnOwner->GetLastMovementInputVector();
		Velocity.X += (MoveDirection.X * JetpackForce * DeltaSeconds) * 0.7f;
		Velocity.Y += (MoveDirection.Y * JetpackForce * DeltaSeconds) * 0.7f;
		Velocity.Z += JetpackForce * DeltaSeconds;
		SetMovementMode(MOVE_Falling);
	}
	else 
	{
		bWantsToJetpack = false;
		FillJetpack(DeltaSeconds);
	}
}

bool UShooterCharacterMovement::CanUseJetpack()
{
	return !IsFuelEmpty();
}

bool UShooterCharacterMovement::IsFuelEmpty()
{
	return JetpackAvailableFuel == 0.0f;
}

float UShooterCharacterMovement::GetJetpackMaxFuel()
{
	return JetpackMaxFuel;
}

float UShooterCharacterMovement::GetJetpackAvailableFuel()
{
	return JetpackAvailableFuel;
}

void UShooterCharacterMovement::FillJetpack(const float DeltaSeconds)
{
	if (IsMovingOnGround())
		if (JetpackAvailableFuel <= JetpackMaxFuel) 
			JetpackAvailableFuel = FMath::Clamp(JetpackAvailableFuel + JetpackFuelFillRate * DeltaSeconds, 0.0f, JetpackMaxFuel);
}

void UShooterCharacterMovement::MulticastPlaySound_Implementation(USoundBase* Sound, FVector Location)
{
	if (!GetPawnOwner()->IsLocallyControlled())
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, Location);
}

void UShooterCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToTeleport = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bWantsToJetpack = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

class FNetworkPredictionData_Client* UShooterCharacterMovement::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);

	if (!ClientPredictionData)
	{
		UShooterCharacterMovement* MutableThis = const_cast<UShooterCharacterMovement*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_NewSkills(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UShooterCharacterMovement::FSavedMove_NewSkills::Clear()
{
	Super::Clear();

	bSavedWantsToTeleport = 0;
	bSavedWantsToJetpack = 0;
	SavedMoveDirection = FVector(0);
}

uint8 UShooterCharacterMovement::FSavedMove_NewSkills::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedWantsToTeleport)
		Result |= FLAG_Custom_0;

	if (bSavedWantsToJetpack)
		Result |= FLAG_Custom_1;

	return Result;
}

bool UShooterCharacterMovement::FSavedMove_NewSkills::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	if (bSavedWantsToTeleport != ((FSavedMove_NewSkills*)& NewMove)->bSavedWantsToTeleport)
		return false;

	if (bSavedWantsToTeleport != ((FSavedMove_NewSkills*)& NewMove)->bSavedWantsToJetpack)
		return false;

	if (SavedJetpackAvailableFuel != ((FSavedMove_NewSkills*)& NewMove)->SavedJetpackAvailableFuel)
		return false;

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UShooterCharacterMovement::FSavedMove_NewSkills::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UShooterCharacterMovement* CharMov = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharMov)
	{
		bSavedWantsToTeleport = CharMov->bWantsToTeleport;
		bSavedWantsToJetpack = CharMov->bWantsToJetpack;
		SavedMoveDirection = CharMov->MoveDirection;
		SavedJetpackAvailableFuel = CharMov->JetpackAvailableFuel;
	}
}

void UShooterCharacterMovement::FSavedMove_NewSkills::PrepMoveFor(class ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UShooterCharacterMovement* CharMov = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharMov)
	{
		CharMov->bWantsToTeleport = bSavedWantsToTeleport;
		CharMov->bWantsToJetpack = bSavedWantsToJetpack;
		CharMov->MoveDirection = SavedMoveDirection;
		CharMov->JetpackAvailableFuel = SavedJetpackAvailableFuel;
	}
}

UShooterCharacterMovement::FNetworkPredictionData_Client_NewSkills::FNetworkPredictionData_Client_NewSkills(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UShooterCharacterMovement::FNetworkPredictionData_Client_NewSkills::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_NewSkills());
}
/**END: CODE ADDED BY VINCENZO PARRILLA*/
