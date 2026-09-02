#pragma once

#include "NativeGameplayTags.h"

namespace ModularWeaponSystem
{
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(None)

    namespace Faction
    {
        UE_DECLARE_GAMEPLAY_TAG_EXTERN(Neutral)
        UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player)
        UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy)
    }

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitScan)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Beam)

    // Projectile-vs-projectile interception classification (FProjectileCollisionRuleConfig.ProjectileTypeTag).
    // Named ProjectileClass in C++ to avoid colliding with the Projectile tag above; the tag strings
    // still nest under "ModularWeaponSystem.Projectile" as required by callers.
    namespace ProjectileClass
    {
        UE_DECLARE_GAMEPLAY_TAG_EXTERN(RocketClass)
        UE_DECLARE_GAMEPLAY_TAG_EXTERN(Bullet)
        UE_DECLARE_GAMEPLAY_TAG_EXTERN(None)
    }
}
