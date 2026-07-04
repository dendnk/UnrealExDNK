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
}
