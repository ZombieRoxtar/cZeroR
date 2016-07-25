//========== Copyright Bit Mage's Stuff, All rights probably reserved. ===========//
//
// Purpose: Molotovs
//
//===============================================================================//

#include "cbase.h"
#include "baseadvancedgrenades.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_plr_dmg_molotov	( "sk_plr_dmg_molotov","0");
ConVar sk_npc_dmg_molotov	( "sk_npc_dmg_molotov","0");
ConVar sk_molotov_radius	( "sk_molotov_radius" ,"0");

class CWeaponMolotov: public CWeaponMultiGrenade
{
	DECLARE_CLASS( CWeaponMolotov, CWeaponMultiGrenade );
	DECLARE_SERVERCLASS();

public:
	void Spawn( void );
};

IMPLEMENT_SERVERCLASS_ST( CWeaponMolotov, DT_WeaponMolotov )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_molotov, CWeaponMolotov );
PRECACHE_WEAPON_REGISTER( weapon_molotov );

void CWeaponMolotov::Spawn( void )
{
	SetType( MOLOT_GRENADE );
	BaseClass::Spawn();
}