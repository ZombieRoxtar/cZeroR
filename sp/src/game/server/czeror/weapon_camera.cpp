//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Digital Camera for Camera zones
//
//=============================================================================

#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"
#include "trigger_special_zone.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define Camera_HUD_ERROR	 "NotInCameraZone"

//-----------------------------------------------------------------------------
// CWeaponCamera
//-----------------------------------------------------------------------------

class CWeaponCamera : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponCamera, CBaseHLCombatWeapon );

	CWeaponCamera(void);

	DECLARE_SERVERCLASS();

	void PrimaryAttack( void );

	Activity GetPrimaryAttackActivity( void );

	virtual float GetFireRate( void ) 
	{
		return 0.5f;
	}

	DECLARE_ACTTABLE();

private:
	float m_flSoonestPrimaryAttack;
	float m_flLastAttackTime;

	float m_flNextDisplayTime;
	float m_flDisplayCooldown;

	CSpecialZone* m_zone;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponCamera, DT_WeaponCamera)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_camera, CWeaponCamera );
PRECACHE_WEAPON_REGISTER( weapon_camera );

BEGIN_DATADESC( CWeaponCamera )

	DEFINE_FIELD( m_flLastAttackTime, FIELD_TIME ),

END_DATADESC()

acttable_t CWeaponCamera::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	false },
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_WALK,						ACT_WALK_PISTOL,				false },
	{ ACT_RUN,						ACT_RUN_PISTOL,					false },
};

IMPLEMENT_ACTTABLE( CWeaponCamera );
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponCamera::CWeaponCamera( void )
{
	m_flNextDisplayTime,m_flSoonestPrimaryAttack = gpGlobals->curtime;
	m_flDisplayCooldown = 5.0f;

	m_fMinRange1 = 0;

	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: Primary fire activates viewer
//-----------------------------------------------------------------------------
void CWeaponCamera::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}
	if ( !pPlayer->IsPlayerInZoneDigitalCamera() )
	{
		if ( m_flNextDisplayTime < gpGlobals->curtime )
		{
			m_flNextDisplayTime = gpGlobals->curtime + m_flDisplayCooldown;
			UTIL_ShowMessage( Camera_HUD_ERROR, pPlayer );
		}
		return;
	}

	if ( m_flSoonestPrimaryAttack > gpGlobals->curtime)
	{
		return;
	}

	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Set each time you fire, in case you're in a new zone
	do
	{
		CBaseEntity* pResult = gEntList.FindEntityByClassnameNearest(
			"trigger_special_zone", GetAbsOrigin(), 8192 );
		m_zone = dynamic_cast < CSpecialZone* > ( pResult );
	}while( m_zone->GetType() != 6 );
	
	m_zone->StartUsing( pPlayer );
	m_zone->StopUsing ( pPlayer );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
Activity CWeaponCamera::GetPrimaryAttackActivity( void )
{
	return ACT_VM_PRIMARYATTACK;
}
