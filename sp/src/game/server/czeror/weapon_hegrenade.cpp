//========== Copyright Bit Mage's Stuff, All rights probably reserved. ===========//
//
// Purpose: High explosive grenades
//
//===============================================================================//

#include "cbase.h"
#include "baseadvancedgrenades.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_plr_dmg_hegrenade	( "sk_plr_dmg_fraggrenade","0");
ConVar sk_npc_dmg_hegrenade	( "sk_npc_dmg_fraggrenade","0");
ConVar sk_hegrenade_radius	( "sk_hegrenade_radius"   ,"0");

class CWeaponHeGrenade: public CWeaponMultiGrenade
{
	DECLARE_CLASS( CWeaponHeGrenade, CWeaponMultiGrenade );
	DECLARE_SERVERCLASS();

public:
	void Spawn( void );
};

IMPLEMENT_SERVERCLASS_ST( CWeaponHeGrenade, DT_WeaponHeGrenade )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_hegrenade, CWeaponHeGrenade );
PRECACHE_WEAPON_REGISTER( weapon_hegrenade );

void CWeaponHeGrenade::Spawn( void )
{
	SetType( HI_EX_GRENADE );
	BaseClass::Spawn();
}