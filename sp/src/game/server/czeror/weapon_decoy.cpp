//========== Copyright Bit Mage's Stuff, All rights probably reserved. ===========//
//
// Purpose: Decoy grenades
//
//===============================================================================//

#include "cbase.h"
#include "baseadvancedgrenades.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_plr_dmg_decoygrenade ( "sk_plr_dmg_decoygrenade","0");
ConVar sk_npc_dmg_decoygrenade ( "sk_npc_dmg_decoygrenade","0");
ConVar sk_decoygrenade_radius  ( "sk_decoygrenade_radius" ,"0");

class CWeaponDecoy: public CWeaponMultiGrenade
{
	DECLARE_CLASS( CWeaponDecoy, CWeaponMultiGrenade );
	DECLARE_SERVERCLASS();

public:
	void Spawn( void );
	void Precache( void );
};

IMPLEMENT_SERVERCLASS_ST( CWeaponDecoy, DT_WeaponDecoy )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_decoy, CWeaponDecoy );
PRECACHE_WEAPON_REGISTER( weapon_decoy );

void CWeaponDecoy::Spawn( void )
{
	SetType( DECOY_GRENADE );
	BaseClass::Spawn();
}

void CWeaponDecoy::Precache( void )
{
	PrecacheScriptSound( "DecoyGrenade.Decoy" );
	BaseClass::Precache();
}