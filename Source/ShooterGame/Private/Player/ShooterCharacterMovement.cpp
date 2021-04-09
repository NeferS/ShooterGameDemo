// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterCharacterMovement.h"
#include "Kismet/GameplayStatics.h"

//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UShooterCharacterMovement::UShooterCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/**BEGIN: CODE ADDED BY VINCENZO PARRILLA*/
	bWantsToTeleport = false;
	TeleportDistance = 1000.0f;
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
void UShooterCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToTeleport = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
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

void UShooterCharacterMovement::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (bWantsToTeleport && (CharacterOwner->GetLocalRole() == ROLE_Authority || CharacterOwner->GetLocalRole() == ROLE_AutonomousProxy))
	{
		const FVector NewLocation = PawnOwner->GetActorLocation() + PawnOwner->GetActorForwardVector() * TeleportDistance;
		PawnOwner->SetActorLocation(NewLocation, true);
		if(GetPawnOwner()->IsLocallyControlled())
			UGameplayStatics::SpawnSoundAtLocation(GetWorld(), TeleportSound, NewLocation);
		if(GetOwner()->HasAuthority())
			MulticastPlaySound(TeleportSound, PawnOwner->GetActorLocation());
		bWantsToTeleport = false;
	}
}

void UShooterCharacterMovement::MulticastPlaySound_Implementation(USoundBase* Sound, FVector Location)
{
	if(!GetPawnOwner()->IsLocallyControlled())
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, Location);
}

void UShooterCharacterMovement::DoTeleport()
{
	bWantsToTeleport = true;
}

void UShooterCharacterMovement::FSavedMove_NewSkills::Clear()
{
	Super::Clear();
	bSavedWantsToTeleport = 0;
}

uint8 UShooterCharacterMovement::FSavedMove_NewSkills::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedWantsToTeleport)
		Result |= FLAG_Custom_0;

	return Result;
}

bool UShooterCharacterMovement::FSavedMove_NewSkills::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	if (bSavedWantsToTeleport != ((FSavedMove_NewSkills*)& NewMove)->bSavedWantsToTeleport)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UShooterCharacterMovement::FSavedMove_NewSkills::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UShooterCharacterMovement* CharMov = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharMov)
	{
		bSavedWantsToTeleport = CharMov->bWantsToTeleport;
	}
}

void UShooterCharacterMovement::FSavedMove_NewSkills::PrepMoveFor(class ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UShooterCharacterMovement* CharMov = Cast<UShooterCharacterMovement>(Character->GetCharacterMovement());
	if (CharMov)
	{
		CharMov->bWantsToTeleport = bSavedWantsToTeleport;
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
