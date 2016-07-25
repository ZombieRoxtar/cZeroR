//========== Copyright Bit Mage's Stuff, All rights probably reserved. ===========//
//
// Purpose: Smoke grenades
//
//===============================================================================//

#include "cbase.h"
#include "baseadvancedgrenades.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_plr_dmg_smokegrenade	( "sk_plr_dmg_smokegrenade","0");
ConVar sk_npc_dmg_smokegrenade	( "sk_npc_dmg_smokegrenade","0");
ConVar sk_smokegrenade_radius	( "sk_smokegrenade_radius" ,"0");

class CWeaponSmokeGrenade: public CWeaponMultiGrenade
{
	DECLARE_CLASS( CWeaponSmokeGrenade, CWeaponMultiGrenade );
	DECLARE_SERVERCLASS();

public:
	void Spawn( void );
};

IMPLEMENT_SERVERCLASS_ST( CWeaponSmokeGrenade, DT_WeaponSmokeGrenade )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_smokegrenade, CWeaponSmokeGrenade );
PRECACHE_WEAPON_REGISTER( weapon_smokegrenade );

void CWeaponSmokeGrenade::Spawn( void )
{
	SetType( SMOKE_GRENADE );
	BaseClass::Spawn();
}