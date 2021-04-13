/**HEADER ADDED BY VINCENZO PARRILLA*/

#pragma once

#include "Pickups/ShooterPickup_Ammo.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "ShooterPickup_Gun.generated.h"

// A pickup object that replenishes ammunition for a weapon (dropped by a character)
UCLASS(Abstract, Blueprintable)
class AShooterPickup_Gun : public AShooterPickup_Ammo
{
	GENERATED_UCLASS_BODY()

	/** Mask used to replicate a mesh change */
	UPROPERTY(ReplicatedUsing=OnRep_MaskMesh)
	USkeletalMesh* MaskMesh;

	/** Replicates a mesh change on the clients (mask for a call to SetWeaponPickupMesh) */
	UFUNCTION()
	void OnRep_MaskMesh();

	/** Set how many clips this pickup holds */
	void SetAmmoClips(int32 Clips);

	/** Set bullets in the current loaded clip */
	void SetAmmoLoadedClip(int32 Bullets);

	/** Set the weapon type */
	void SetWeaponType(TSubclassOf<AShooterWeapon> Type);

	/** Set the weapon mesh */
	void SetWeaponPickupMesh(USkeletalMesh* WeapMesh);
	
protected:
	virtual void BeginPlay() override;

	virtual void OnPickedUp() override;

	virtual void GivePickupTo(AShooterCharacter* Pawn) override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	/** how much ammo there are in the currently loaded clip? */
	UPROPERTY(EditDefaultsOnly, Category = Pickup)
	int32 AmmoLoadedClip;

	/** how much this pickup lives? */
	UPROPERTY(EditDefaultsOnly, Category = Pickup)
	float LifeTime = 10.0f;

private:
	/** Mesh component */
	UPROPERTY(VisibleDefaultsOnly, Category = "Mesh")
	USkeletalMeshComponent* PickupSMC;

protected:
	/** Returns PickupSMC subobject **/
	FORCEINLINE USkeletalMeshComponent* GetPickupSMC() const { return PickupSMC; }
};