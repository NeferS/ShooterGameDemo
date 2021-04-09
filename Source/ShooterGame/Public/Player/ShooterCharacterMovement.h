// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * Movement component meant for use with Pawns.
 */

#pragma once
#include "Sound/SoundBase.h"
#include "ShooterCharacterMovement.generated.h"

UCLASS()
class UShooterCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()
	
	virtual float GetMaxSpeed() const override;

	/**BEGIN: CODE ADDED BY VINCENZO PARRILLA*/

	// TELEPORT 
	/** Character pressed teleport key */
	bool bWantsToTeleport : 1;
	/** Teleport character forward */
	void DoTeleport();
	/** Character teleport distance */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Teleport")
	float TeleportDistance;
	/** Character teleport sound */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Teleport")
	USoundBase* TeleportSound;

	// GENERAL 
	UFUNCTION(reliable, NetMulticast)
	void MulticastPlaySound(USoundBase* Sound, FVector Location);

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	class FSavedMove_NewSkills : public FSavedMove_Character
	{
	public:

		typedef FSavedMove_Character Super;

		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(class ACharacter* Character) override;

		/** Character teleport */
		uint8 bSavedWantsToTeleport : 1;
	};

	class FNetworkPredictionData_Client_NewSkills : public FNetworkPredictionData_Client_Character
	{
	public:

		typedef FNetworkPredictionData_Client_Character Super;

		FNetworkPredictionData_Client_NewSkills(const UCharacterMovementComponent& ClientMovement);
		virtual FSavedMovePtr AllocateNewMove() override;
	};
	/**END: CODE ADDED BY VINCENZO PARRILLA*/
};

