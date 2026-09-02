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

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Projectile, "ModularWeaponSystem.Projectile",
	                               "Physical ammo with speed and travel time, for example bullets or rockets.")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitScan, "ModularWeaponSystem.HitScan",
	                               "Instant line checks that apply damage immediately when the shot is fired.")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Beam, "ModularWeaponSystem.Beam",
	                               "Continuous damage streams that require staying aimed at the target over time.")

	namespace ProjectileClass
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(RocketClass, "ModularWeaponSystem.Projectile.RocketClass",
		                               "Explosive, AoE-capable shell: rockets, grenades, etc.")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(Bullet, "ModularWeaponSystem.Projectile.Bullet",
		                               "Rapid-fire, non-explosive round.")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(None, "ModularWeaponSystem.Projectile.None",
		                               "Reserved placeholder no real projectile is ever tagged with; exists so "
		                               "ValidTargetProjectileTags can express \"matches nothing\" (an empty "
		                               "container instead means \"matches everything\").")
	}
}
