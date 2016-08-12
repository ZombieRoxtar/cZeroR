//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Radio-Controlled Bombs
//
//=============================================================================

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "trigger_special_zone.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define RCBOMB_HUD_ERROR	 "NotInRcBombZone"

//-----------------------------------------------------------------------------
// CWeaponRcBomb
//-----------------------------------------------------------------------------

class CWeaponRcBomb : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponRcBomb, CBaseHLCombatWeapon );

	CWeaponRcBomb(void);

	DECLARE_SERVERCLASS();

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void WeaponIdle( void );
	bool Deploy( void );

	bool Plant( void );
	bool Detonate( void );

private:
	float m_flSoonestPrimaryAttack;
	float m_flSoonestSecondaryAttack;
	float m_flLastAttackTime;

	float m_flNextDisplayTime;

	CSpecialZone* m_zone;
	CSpecialZone* m_zonesWithBombs[64];
	int m_nBombsPlanted;
	bool m_bDetonatorOn;
	bool m_bWasInZone;
	bool m_bInAnim;
};

IMPLEMENT_SERVERCLASS_ST( CWeaponRcBomb, DT_WeaponRcBomb )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_rcbomb, CWeaponRcBomb );
PRECACHE_WEAPON_REGISTER( weapon_rcbomb );

BEGIN_DATADESC( CWeaponRcBomb )

	DEFINE_FIELD( m_flLastAttackTime, FIELD_TIME ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponRcBomb::CWeaponRcBomb( void )
{
	m_flNextDisplayTime =
		m_flSoonestPrimaryAttack =
		m_flSoonestSecondaryAttack = gpGlobals->curtime;

	m_nBombsPlanted = 0;

	m_bFiresUnderwater = true;
	m_bDetonatorOn = 
		m_bWasInZone =
		m_bInAnim = false;
}

//-----------------------------------------------------------------------------
// Purpose: Plants or Detonates
//-----------------------------------------------------------------------------
void CWeaponRcBomb::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	bool shouldSwitch = false;

	if ( !pPlayer )
		return;

	if (m_flSoonestPrimaryAttack >= gpGlobals->curtime)
		return;

	if ( m_bDetonatorOn )
		shouldSwitch = Detonate();
	else
		shouldSwitch = Plant();
	
	// Swap modes after a SUCCESSFULL plant/detonate
	if ( shouldSwitch )
	{
		m_flLastAttackTime = gpGlobals->curtime;
		SecondaryAttack();
	}

	m_flSoonestPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: Plants bombs
//
// Returns: bool - Should we switch to the detonator?
//-----------------------------------------------------------------------------
bool CWeaponRcBomb::Plant( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( ( !pPlayer ) || !m_iClip1 )
	{
		return true;
	}

	if ( !pPlayer->IsPlayerInZoneRcBomb() )
	{
		if ( m_flNextDisplayTime < gpGlobals->curtime )
		{
			m_flNextDisplayTime = gpGlobals->curtime + HUD_ERROR_TIMEOUT;
			UTIL_ShowMessage( RCBOMB_HUD_ERROR, pPlayer );
		}
		return false;
	}

	// Set each time you fire, in case you're in a new zone
	do
	{
		CBaseEntity* pResult = gEntList.FindEntityByClassnameNearest(
			"trigger_special_zone", GetAbsOrigin(), 8192 );
		m_zone = dynamic_cast < CSpecialZone* > ( pResult );
	}while( m_zone->GetType() != ZONE_RC_BOMB );

	m_zone->StartUsing( pPlayer );

	/*
		FIXME: This doesn't even play.
		FIXME: The model's hands don't move away until ACT_SLAM_TRIPMINE_ATTACH2
		FIXME: Will the mode switch play a draw anim on time?
	*/
	m_bInAnim = true;
	SendWeaponAnim(ACT_SLAM_TRIPMINE_ATTACH);

	m_iClip1--;

	m_zone->StopUsing ( pPlayer );

	m_zonesWithBombs[m_nBombsPlanted] = m_zone;
	m_nBombsPlanted++;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Detonates bombs
//
// Returns: bool - Should we switch to the bomb(s)?
//-----------------------------------------------------------------------------
bool CWeaponRcBomb::Detonate( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
	{
		return false;
	}

	if ( m_nBombsPlanted )
	{
		for ( int i = 0; i <= m_nBombsPlanted; i++ )
		{
			if ( m_zonesWithBombs[i] )
			{
				m_zonesWithBombs[i]->Success( pPlayer );
				m_zonesWithBombs[i] = 0;
			}
		}
		m_nBombsPlanted = 0;
	}

	
	// The detonation animation plays even if no bombs were found, like a "dry fire" anim.
	// FIXME: There is no delay, so we never see the anim
	m_bInAnim = true;
	SendWeaponAnim(ACT_SLAM_DETONATOR_DETONATE);

	// Only switch to bombs if we have any.
	return HasPrimaryAmmo();
}

//-----------------------------------------------------------------------------
// Purpose: Switches detonator/bombs
//-----------------------------------------------------------------------------
void CWeaponRcBomb::SecondaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	if ( m_flSoonestSecondaryAttack >= gpGlobals->curtime )
	{
		return;
	}

	int iAnim = 0;
	if ( m_bDetonatorOn && HasPrimaryAmmo() )
	{
		m_bDetonatorOn = false;
		// FIXME: What about ACT_SLAM_DETONATOR_HOLSTER ?
		iAnim = ACT_SLAM_THROW_ND_DRAW;
		if (pPlayer->IsPlayerInZoneRcBomb())
		{
			iAnim = ACT_SLAM_TRIPMINE_DRAW;
			m_bWasInZone = true;
		}
	}
	else
	{
		m_bDetonatorOn = true;
		// There are no ACT_SLAM_*_ND_HOLSTER anims? Can we FIXME?
		iAnim = ACT_SLAM_DETONATOR_DRAW;
	}
	m_bInAnim = true;
	SendWeaponAnim(iAnim);

	m_flSoonestSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponRcBomb::WeaponIdle(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	if ((m_bInAnim) && IsSequenceFinished())
	{
		// FIXME: Trying to get "slam" to switch to "tripmine" as visual feedback for being in the zone
		int iAnim = 0;
		if (m_bDetonatorOn)
		{
			iAnim = ACT_SLAM_DETONATOR_IDLE;
			m_bInAnim = false;
		}
		else
			if (pPlayer->IsPlayerInZoneRcBomb())
			{
				iAnim = ACT_SLAM_TRIPMINE_IDLE;
				m_bInAnim = false;
				if (!m_bWasInZone)
				{
					iAnim = ACT_SLAM_TRIPMINE_TO_THROW_ND;
					m_bInAnim = true;
				}
				m_bWasInZone = true;
			}
			else
			{
				iAnim = ACT_SLAM_THROW_ND_IDLE;
				m_bInAnim = false;
				if (m_bWasInZone)
				{
					iAnim = ACT_SLAM_TRIPMINE_TO_THROW_ND;
					m_bInAnim = true;
				}
				m_bWasInZone = false;
			}
		SendWeaponAnim(iAnim);
	}
}

bool CWeaponRcBomb::Deploy(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
	{
		return false;
	}

	m_bWasInZone = false;
	// Let the idle func switch it if it's in a bomb zone.
	int iAnim = m_bDetonatorOn ? ACT_SLAM_DETONATOR_DRAW : ACT_SLAM_THROW_ND_DRAW;
	m_bInAnim = true;
	SendWeaponAnim(iAnim);

	return DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), iAnim, (char*)GetAnimPrefix());
}
