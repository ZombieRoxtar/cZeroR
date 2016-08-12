//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Kamikaze boom-vest
//
//=====================================================================
#include "cbase.h"
#include "basehlcombatweapon.h"
#include "soundent.h"
#include "explode.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_npc_dmg_boomvest ("sk_npc_dmg_boomvest","150");
extern ConVar sk_hegrenade_radius;

//-----------------------------------------------------------------------------
// CWeaponBoomVest
//-----------------------------------------------------------------------------
class CWeaponBoomVest : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponBoomVest, CBaseHLCombatWeapon );
	DECLARE_SERVERCLASS();

	CWeaponBoomVest(void);
	int CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	void PrimaryAttack( void );
	void Boom( void );

	virtual float GetFireRate( void ) 
	{
		return 3.0f;
	}

private:
	float m_flSoonestPrimaryAttack;
	float m_flfirerate;
	float m_flDamage;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponBoomVest, DT_WeaponBoomVest)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_boomvest, CWeaponBoomVest );
PRECACHE_WEAPON_REGISTER( weapon_boomvest );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponBoomVest::CWeaponBoomVest( void )
{
	m_flSoonestPrimaryAttack = gpGlobals->curtime + m_flfirerate;
	m_flfirerate = GetFireRate();
	m_fMinRange1 = 0;
	m_fMaxRange1 = sk_hegrenade_radius.GetFloat();
	m_flDamage = sk_npc_dmg_boomvest.GetFloat();
}
//-----------------------------------------------------------------------------
// Purpose: Primary fire
//-----------------------------------------------------------------------------
void CWeaponBoomVest::PrimaryAttack( void )
{
	if ( m_flSoonestPrimaryAttack >= gpGlobals->curtime )
	{
		return;
	}

	m_flSoonestPrimaryAttack = gpGlobals->curtime + m_flfirerate;
	CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin(), m_fMaxRange1, GetFireRate() );

	// TODO: wait time
	Boom();
}
//-----------------------------------------------------------------------------
void CWeaponBoomVest::Boom( void )
{
	WeaponSound(SINGLE);
	// Make sure that the user dies a horrible, gruesome death
	if( GetOwner()->IsAlive())
	{
		//How? This dosen't work.
		//CTakeDamageInfo info( this, this, 200, DMG_BLAST );
	}
	ExplosionCreate( GetOwner()->GetLocalOrigin(), GetAbsAngles(), this, 1024, m_fMaxRange1, true );
	//AddEFlags(EFL_KILLME);
	Warning("Boomvest went boom for %f damage.\n",m_flDamage);
}
