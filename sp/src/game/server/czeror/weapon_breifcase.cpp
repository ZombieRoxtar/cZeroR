//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Briefcase for Briefcase zones
//
//=============================================================================

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "trigger_special_zone.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define Briefcase_HUD_ERROR	 "NotInBriefcaseZone"

//-----------------------------------------------------------------------------
// CWeaponBriefcase
//-----------------------------------------------------------------------------

class CWeaponBriefcase : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponBriefcase, CBaseHLCombatWeapon );

	CWeaponBriefcase(void);

	DECLARE_SERVERCLASS();

	void PrimaryAttack( void );

	virtual float GetFireRate( void ) 
	{
		return 0.5f;
	}

private:
	float m_flSoonestPrimaryAttack;
	float m_flLastAttackTime;

	float m_flNextDisplayTime;

	CSpecialZone* m_zone;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponBriefcase, DT_WeaponBriefcase)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_Briefcase, CWeaponBriefcase );
PRECACHE_WEAPON_REGISTER( weapon_Briefcase );

BEGIN_DATADESC( CWeaponBriefcase )

	DEFINE_FIELD( m_flLastAttackTime, FIELD_TIME ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponBriefcase::CWeaponBriefcase( void )
{
	m_flNextDisplayTime =
		m_flSoonestPrimaryAttack = gpGlobals->curtime;

	m_fMinRange1 = 0;

	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: Primary fire activates viewer
//-----------------------------------------------------------------------------
void CWeaponBriefcase::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}
	if ( !pPlayer->IsPlayerInZoneBriefcase() )
	{
		if ( m_flNextDisplayTime < gpGlobals->curtime )
		{
			m_flNextDisplayTime = gpGlobals->curtime + HUD_ERROR_TIMEOUT;
			UTIL_ShowMessage( Briefcase_HUD_ERROR, pPlayer );
		}
		return;
	}

	if ( m_flSoonestPrimaryAttack > gpGlobals->curtime)
	{
		return;
	}

	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + GetFireRate();

	// Set each time you fire, in case you're in a new zone
	do
	{
		CBaseEntity* pResult = gEntList.FindEntityByClassnameNearest(
			"trigger_special_zone", GetAbsOrigin(), 8192 );
		m_zone = dynamic_cast < CSpecialZone* > ( pResult );
	}while( m_zone->GetType() != ZONE_BRF_CASE );
	
	m_zone->StartUsing( pPlayer );
}
