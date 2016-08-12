//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Fiberoptic Camera controls the zone's point_viewcontrol
//
//=============================================================================

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "trigger_special_zone.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FIBEROPTICCAMERA_HUD_ERROR	 "NotInFiberopticCameraZone"
#define FIBEROPTICCAMERA_ZOOM_FOV 30

//-----------------------------------------------------------------------------
// CWeaponFiberopticCamera
//-----------------------------------------------------------------------------

class CWeaponFiberopticCamera : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponFiberopticCamera, CBaseHLCombatWeapon );

	CWeaponFiberopticCamera(void);

	DECLARE_SERVERCLASS();

	void PrimaryAttack( void );
	void SecondaryAttack( void );

	virtual float GetFireRate( void ) 
	{
		return 0.5f;
	}

private:
	float m_flSoonestPrimaryAttack;
	float m_flSoonestSecondaryAttack;
	float m_flLastAttackTime;
	float m_flfirerate;
	float m_flNextDisplayTime;

	bool m_bActive;
	bool m_bZoomed;

	int m_iZoomFOV;

	CSpecialZone* m_zone;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponFiberopticCamera, DT_WeaponFiberopticCamera)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_fiberopticcamera, CWeaponFiberopticCamera );
PRECACHE_WEAPON_REGISTER( weapon_fiberopticcamera );

BEGIN_DATADESC( CWeaponFiberopticCamera )

	DEFINE_FIELD( m_flLastAttackTime, FIELD_TIME ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponFiberopticCamera::CWeaponFiberopticCamera( void )
{
	m_flNextDisplayTime =
		m_flSoonestPrimaryAttack =
		m_flSoonestSecondaryAttack = gpGlobals->curtime;
	m_flfirerate = GetFireRate();
	m_iZoomFOV = 30;

	m_fMinRange1 =
		m_fMinRange2 = 0;

	m_bFiresUnderwater = true;
	m_bZoomed =
		m_bActive = false;
}

//-----------------------------------------------------------------------------
// Purpose: Primary fire activates viewer
//-----------------------------------------------------------------------------
void CWeaponFiberopticCamera::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	if ( !pPlayer->IsPlayerInZoneFiberopticCamera() )
	{
		if ( m_flNextDisplayTime < gpGlobals->curtime )
		{
			m_flNextDisplayTime = gpGlobals->curtime + HUD_ERROR_TIMEOUT;
			UTIL_ShowMessage( FIBEROPTICCAMERA_HUD_ERROR, pPlayer );
		}
		return;
	}

	if ( m_flSoonestPrimaryAttack > gpGlobals->curtime)
	{
		return;
	}

	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + m_flfirerate;

	// Set each time you fire, in case you're in a new zone
	do
	{
		CBaseEntity* pResult = gEntList.FindEntityByClassnameNearest(
			"trigger_special_zone", GetAbsOrigin(), 8192 );
		m_zone = dynamic_cast < CSpecialZone* > ( pResult );
	}while( m_zone->GetType() != ZONE_FO_CAMERA );

	if( !m_bActive )
	{
		m_bActive = true;
		m_zone->StartUsing( pPlayer );
	}
	else
	{
		if( m_bZoomed )
		{
			pPlayer->SetFOV( this, 0 );
			m_bZoomed = false;
		}
		m_zone->StopUsing( pPlayer );
		m_bActive = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Secondary fire zooms viewer
//-----------------------------------------------------------------------------
void CWeaponFiberopticCamera::SecondaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	if ( m_flSoonestSecondaryAttack > gpGlobals->curtime)
	{
		return;
	}
	m_flSoonestSecondaryAttack = gpGlobals->curtime + m_flfirerate;

	if( m_bActive )
	{
		if( !m_bZoomed )
		{
			m_bZoomed = true;
			pPlayer->SetFOV( this, FIBEROPTICCAMERA_ZOOM_FOV );
		}
		else
		{
			pPlayer->SetFOV( this, 0 );
			m_bZoomed = false;
		}
	}
}
