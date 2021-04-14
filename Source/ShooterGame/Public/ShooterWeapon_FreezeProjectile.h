/**HEADER ADDED BY VINCENZO PARRILLA*/

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ShooterWeapon_Projectile.h"
#include "ShooterWeapon_FreezeProjectile.generated.h"

//empty class, used just to recognize objects related to the Freezing Gun
UCLASS()
class AShooterWeapon_FreezeProjectile : public AShooterWeapon_Projectile
{
	GENERATED_UCLASS_BODY()
	
protected:

	virtual EAmmoType GetAmmoType() const override
	{
		return EAmmoType::EFreezeRocket;
	}
};
