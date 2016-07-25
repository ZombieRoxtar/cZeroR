//========== Copyright Bit Mage's Stuff, All rights probably reserved. ===========//
//
// Purpose: Incindiary grenades
//
//===============================================================================//

#include "cbase.h"
#include "baseadvancedgrenades.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_plr_dmg_incgrenade	( "sk_plr_dmg_incgrenade","0");
ConVar sk_npc_dmg_incgrenade	( "sk_npc_dmg_incgrenade","0");
ConVar sk_incgrenade_radius		( "sk_incgrenade_radius" ,"0");

class CWeaponIncGrenade: public CWeaponMultiGrenade
{
	DECLARE_CLASS( CWeaponIncGrenade, CWeaponMultiGrenade );
	DECLARE_SERVERCLASS();

public:
	void Spawn( void );
};

IMPLEMENT_SERVERCLASS_ST( CWeaponIncGrenade, DT_WeaponIncGrenade )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_incgrenade, CWeaponIncGrenade );
PRECACHE_WEAPON_REGISTER( weapon_incgrenade );

void CWeaponIncGrenade::Spawn( void )
{
	SetType( INCEN_GRENADE );
	BaseClass::Spawn();
}