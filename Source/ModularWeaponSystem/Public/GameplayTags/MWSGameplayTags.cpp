#include "MWSGameplayTags.h"


namespace MWSGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(None, "MWSGameplayTags.None")

	namespace Faction
	{
		UE_DEFINE_GAMEPLAY_TAG(Neutral, "MWSGameplayTags.Faction.Neutral")
		UE_DEFINE_GAMEPLAY_TAG(Player, "MWSGameplayTags.Faction.Player")
		UE_DEFINE_GAMEPLAY_TAG(Enemy, "MWSGameplayTags.Faction.Enemy")
	}

	namespace Projectile
	{
		namespace Bullet
		{
			UE_DEFINE_GAMEPLAY_TAG(MachineGun, "MWSGameplayTags.Projectile.Bullet.MachineGun")
			UE_DEFINE_GAMEPLAY_TAG(QuadGun, "MWSGameplayTags.Projectile.Bullet.QuadGun")
			UE_DEFINE_GAMEPLAY_TAG(AAGun, "MWSGameplayTags.Projectile.Bullet.AAGun")
		}

		namespace Rocket
		{
			UE_DEFINE_GAMEPLAY_TAG(Standart, "MWSGameplayTags.Projectile.Rocket.Standart")
			UE_DEFINE_GAMEPLAY_TAG(StandartNoGravity, "MWSGameplayTags.Projectile.Rocket.")
			UE_DEFINE_GAMEPLAY_TAG(Cluster, "MWSGameplayTags.Projectile.Rocket.Cluster")
			UE_DEFINE_GAMEPLAY_TAG(Wobble, "MWSGameplayTags.Projectile.Rocket.Wobble")
			UE_DEFINE_GAMEPLAY_TAG(Homing, "MWSGameplayTags.Projectile.Rocket.Homing")
		}

		UE_DEFINE_GAMEPLAY_TAG(Laser, "MWSGameplayTags.Laser")
		UE_DEFINE_GAMEPLAY_TAG(Grenade, "MWSGameplayTags.Grenade")
	}
}
