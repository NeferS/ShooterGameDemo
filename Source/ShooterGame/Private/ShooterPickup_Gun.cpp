/**ADDED BY VINCENZO PARRILLA*/

#include "ShooterGame.h"
#include "ShooterPickup_Gun.h"
#include "Net/UnrealNetwork.h"

AShooterPickup_Gun::AShooterPickup_Gun(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AmmoClips = 0;
	AmmoLoadedClip = 0;

	PickupSMC = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("PickupMesh"));
	PickupSMC->SetupAttachment(RootComponent);
}

void AShooterPickup_Gun::SetAmmoClips(int32 Clips)
{
	AmmoClips = Clips;
}

void AShooterPickup_Gun::SetAmmoLoadedClip(int32 Bullets)
{
	AmmoLoadedClip = Bullets;
}

void AShooterPickup_Gun::SetWeaponType(TSubclassOf<AShooterWeapon> Type)
{
	WeaponType = Type;
}

void AShooterPickup_Gun::SetWeaponPickupMesh(USkeletalMesh* WeapMesh)
{
	PickupSMC->SetSkeletalMesh(WeapMesh, false);
}

void AShooterPickup_Gun::OnRep_MaskMesh()
{
	SetWeaponPickupMesh(MaskMesh);
}

void AShooterPickup_Gun::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterPickup_Gun, MaskMesh);
}

void AShooterPickup_Gun::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeTime);
}

void AShooterPickup_Gun::OnPickedUp()
{
	Super::OnPickedUp();

	Destroy();
}

void AShooterPickup_Gun::GivePickupTo(class AShooterCharacter* Pawn)
{
	AShooterWeapon* Weapon = (Pawn ? Pawn->FindWeapon(WeaponType) : NULL);
	if (Weapon)
	{
		int32 Qty = AmmoClips * Weapon->GetAmmoPerClip() + AmmoLoadedClip;
		Weapon->GiveAmmo(Qty);

		// Fire event for collected ammo
		if (Pawn)
		{
			const UWorld* World = GetWorld();
			const IOnlineEventsPtr Events = Online::GetEventsInterface(World);
			const IOnlineIdentityPtr Identity = Online::GetIdentityInterface(World);

			if (Events.IsValid() && Identity.IsValid())
			{
				AShooterPlayerController* PC = Cast<AShooterPlayerController>(Pawn->Controller);
				if (PC)
				{
					ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PC->Player);

					if (LocalPlayer)
					{
						const int32 UserIndex = LocalPlayer->GetControllerId();
						TSharedPtr<const FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);
						if (UniqueID.IsValid())
						{
							FVector Location = Pawn->GetActorLocation();

							FOnlineEventParms Params;

							Params.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
							Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
							Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

							Params.Add(TEXT("ItemId"), FVariantData((int32)Weapon->GetAmmoType() + 1)); // @todo come up with a better way to determine item id, currently health is 0 and ammo counts from 1
							Params.Add(TEXT("AcquisitionMethodId"), FVariantData((int32)0)); // unused
							Params.Add(TEXT("LocationX"), FVariantData(Location.X));
							Params.Add(TEXT("LocationY"), FVariantData(Location.Y));
							Params.Add(TEXT("LocationZ"), FVariantData(Location.Z));
							Params.Add(TEXT("ItemQty"), FVariantData((int32)Qty));

							Events->TriggerEvent(*UniqueID, TEXT("CollectPowerup"), Params);
						}
					}
				}
			}
		}
	}
}
