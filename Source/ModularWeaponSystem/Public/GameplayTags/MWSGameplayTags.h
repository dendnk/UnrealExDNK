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

    namespace Projectile
    {
        UE_DECLARE_GAMEPLAY_TAG_EXTERN(Bullet)
        UE_DECLARE_GAMEPLAY_TAG_EXTERN(Rocket)
        UE_DECLARE_GAMEPLAY_TAG_EXTERN(Laser)
    }

}
