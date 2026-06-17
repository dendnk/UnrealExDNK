#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

// TODO: Need to add here only generic tags for this plugin, game specific tags should be moved to MainProject files and not be placed here. Should we delete these tags and make in HeliAce?
namespace MWSGameplayTags
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
        namespace Bullet
        {
            UE_DECLARE_GAMEPLAY_TAG_EXTERN(MachineGun)
            UE_DECLARE_GAMEPLAY_TAG_EXTERN(QuadGun)
            UE_DECLARE_GAMEPLAY_TAG_EXTERN(AAGun)
        }
        
        namespace Rocket
        {
            UE_DECLARE_GAMEPLAY_TAG_EXTERN(Standart)
            UE_DECLARE_GAMEPLAY_TAG_EXTERN(StandartNoGravity)
            UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cluster)
            UE_DECLARE_GAMEPLAY_TAG_EXTERN(Wobble)
            UE_DECLARE_GAMEPLAY_TAG_EXTERN(Homing)
        }
        
        UE_DECLARE_GAMEPLAY_TAG_EXTERN(Laser)
    }

}