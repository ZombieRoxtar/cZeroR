//========== Copyright Bit Mage's Stuff, All rights probably reserved. ===========//
//
// Purpose: Flash bang grenades
//
//===============================================================================//

#include "cbase.h"
#include "baseadvancedgrenades.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_plr_dmg_flashgrenade	( "sk_plr_dmg_flashgrenade","0");
ConVar sk_npc_dmg_flashgrenade	( "sk_npc_dmg_flashgrenade","0");
ConVar sk_flashgrenade_radius	( "sk_flashgrenade_radius" ,"0");

class CWeaponFlashBang: public CWeaponMultiGrenade
{
	DECLARE_CLASS( CWeaponFlashBang, CWeaponMultiGrenade );
	DECLARE_SERVERCLASS();

public:
	void Spawn( void );
};

IMPLEMENT_SERVERCLASS_ST( CWeaponFlashBang, DT_WeaponFlashBang )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_flashbang, CWeaponFlashBang );
PRECACHE_WEAPON_REGISTER( weapon_flashbang );

void CWeaponFlashBang::Spawn( void )
{
	SetType( FLASH_GRENADE );
	BaseClass::Spawn();
}