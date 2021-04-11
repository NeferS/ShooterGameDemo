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
	virtual float GetMaxAcceleration() const override;

	virtual FVector NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const override;

	//DebugMessageExample: GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, FString::Printf(TEXT("Some debug string")));

	// TELEPORT 
	/** Character pressed teleport key */
	bool bWantsToTeleport : 1;
	/** Character teleport distance */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Teleport")
	float TeleportDistance = 1000.0f;
	/** Character teleport sound */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Teleport")
	USoundBase* TeleportSound;

	//JETPACK
	/** Character pressed jetpack key */
	bool bWantsToJetpack : 1;
	/** Character last move direction */
	FVector MoveDirection;
	/** Current available fuel for jetpack movement */
	float JetpackAvailableFuel;
	/** Maximum available fuel for jetpack movement */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Jetpack")
	float JetpackMaxFuel = 100.0f;
	/** Consumed fuel value for each second */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Jetpack")
	float JetpackFuelConsumeRate = 20.0f;
	/** Filled fuel value for each second */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Jetpack")
	float JetpackFuelFillRate = 10.0f;
	/** Force applied to the character when using the jetpack */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Jetpack")
	float JetpackForce = 700.0f;
	/** Jetpack acceleration modifier */
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement: Jetpack")
	float JetpackAccelerationModifier = 2.5f;
	/** Reduce the gravity while using jetpack */
	UPROPERTY(BlueprintReadOnly, Category = "Character Movement: Jetpack")
	float GravityScaleWhileJetpack = 0;

	/** check if pawn can use the jetpack */
	bool CanUseJetpack();
	/** check if the fuel is completely empty */
	bool IsFuelEmpty();
	/** returns the maximum amount of fuel for jetpacking */
	float GetJetpackMaxFuel();
	/** returns the available fuel for jetpacking */
	float GetJetpackAvailableFuel();
	/** fills the jetpack if some conditions are met */
	void FillJetpack(const float DeltaSeconds);

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
		/** Character jetpack */
		uint8 bSavedWantsToJetpack : 1;
		/** Currently available fuel */
		float SavedJetpackAvailableFuel;
		/** Character move direction */
		FVector SavedMoveDirection;
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

