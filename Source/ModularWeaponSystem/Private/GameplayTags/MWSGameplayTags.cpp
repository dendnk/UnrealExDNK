#include "GameplayTags/MWSGameplayTags.h"


namespace ModularWeaponSystem
{
	UE_DEFINE_GAMEPLAY_TAG(None, "ModularWeaponSystem.None")

	namespace Faction
	{
		UE_DEFINE_GAMEPLAY_TAG(Neutral, "ModularWeaponSystem.Faction.Neutral")
		UE_DEFINE_GAMEPLAY_TAG(Player, "ModularWeaponSystem.Faction.Player")
		UE_DEFINE_GAMEPLAY_TAG(Enemy, "ModularWeaponSystem.Faction.Enemy")
	}

	namespace Projectile
	{
		UE_DEFINE_GAMEPLAY_TAG(Bullet, "ModularWeaponSystem.Projectile.Bullet")
		UE_DEFINE_GAMEPLAY_TAG(Rocket, "ModularWeaponSystem.Projectile.Rocket")
		UE_DEFINE_GAMEPLAY_TAG(Laser, "ModularWeaponSystem.Projectile.Laser")
	}
}
