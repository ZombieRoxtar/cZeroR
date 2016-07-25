//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Handling the kevlar item
//
//=============================================================================

#include "cbase.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "gamerules.h"
#include "items.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/* Don't give 100 suit armor, just equip it.
If this is set and the player has a suit then this item does nothing... */
#define SF_ITEM_ARMOR_SKIP_ARMOR 0x0001

class CItemArmor : public CItem
{
public:
	DECLARE_CLASS(CItemArmor, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/items/armor.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/items/armor.mdl");
		PrecacheScriptSound("BaseCombatCharacter.ItemPickup2");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player *>(pPlayer);
		return (pHL2Player && pHL2Player->ApplyArmor(HasSpawnFlags(SF_ITEM_ARMOR_SKIP_ARMOR)));
	}
};

LINK_ENTITY_TO_CLASS(item_armor, CItemArmor);
PRECACHE_REGISTER(item_armor);
