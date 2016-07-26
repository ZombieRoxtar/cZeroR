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

#define	RC_BOMB_REPLANT_TIME 0.5f
#define	RCBOMB_HUD_TIMEOUT	 5.0f
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

	bool Plant( void );
	bool Detonate( void );

	Activity GetPrimaryAttackActivity( void ) { return ACT_VM_PRIMARYATTACK; }

	virtual float GetFireRate( void ) 
	{
		return RC_BOMB_REPLANT_TIME;
	}

	DECLARE_ACTTABLE();

private:
	float m_flSoonestPrimaryAttack;
	float m_flSoonestSecondaryAttack;
	float m_flLastAttackTime;

	float m_flNextDisplayTime;

	CSpecialZone* m_zone;
	CSpecialZone* m_zonesWithBombs[64];
	int m_nBombsPlanted;
	bool m_bDetonatorOn;
};

IMPLEMENT_SERVERCLASS_ST( CWeaponRcBomb, DT_WeaponRcBomb )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_rcbomb, CWeaponRcBomb );
PRECACHE_WEAPON_REGISTER( weapon_rcbomb );

BEGIN_DATADESC( CWeaponRcBomb )

	DEFINE_FIELD( m_flLastAttackTime, FIELD_TIME ),

END_DATADESC()

acttable_t CWeaponRcBomb::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			false},
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	false},
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			false},
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		false},
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false},
	{ ACT_WALK,						ACT_WALK_PISTOL,				false},
	{ ACT_RUN,						ACT_RUN_PISTOL,					false},
};

IMPLEMENT_ACTTABLE( CWeaponRcBomb );
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponRcBomb::CWeaponRcBomb( void )
{
	m_flNextDisplayTime, m_flSoonestPrimaryAttack, m_flSoonestSecondaryAttack = gpGlobals->curtime;

	m_nBombsPlanted = 0;

	m_bFiresUnderwater = true;
	m_bDetonatorOn = false;
}

//-----------------------------------------------------------------------------
// Purpose: Plants or Detonates
//-----------------------------------------------------------------------------
void CWeaponRcBomb::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	bool shouldSwitch = false;

	if ( !pPlayer )
	{
		return;
	}

	if ( m_bDetonatorOn )
	{
		if ( Detonate() )
		{
			shouldSwitch = true;
		}
	}
	else
	{
		if ( Plant() )
		{
			shouldSwitch = true;
		}
	}

	// Swap modes after a SUCCESSFULL plant/detonate
	if ( shouldSwitch )
	{
		SecondaryAttack();
	}
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
			m_flNextDisplayTime = gpGlobals->curtime + RCBOMB_HUD_TIMEOUT;
			UTIL_ShowMessage( RCBOMB_HUD_ERROR, pPlayer );
		}
		return false;
	}

	if ( m_flSoonestPrimaryAttack >= gpGlobals->curtime )
	{
		return false;
	}

	//TODO: Wait for the anim to finish

	// Set each time you fire, in case you're in a new zone
	do
	{
		CBaseEntity* pResult = gEntList.FindEntityByClassnameNearest(
			"trigger_special_zone", GetAbsOrigin(), 8192 );
		m_zone = dynamic_cast < CSpecialZone* > ( pResult );
	}while( m_zone->GetType() != 1 );

	m_zone->StartUsing( pPlayer );
	
	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + RC_BOMB_REPLANT_TIME;

	SendWeaponAnim( GetPrimaryAttackActivity() );

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

	if ( m_flSoonestPrimaryAttack >= gpGlobals->curtime )
	{
		return false;
	}
	m_flLastAttackTime = gpGlobals->curtime;
	m_flSoonestPrimaryAttack = gpGlobals->curtime + RC_BOMB_REPLANT_TIME;

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
		return true;
	}

	// The player has not planted any bombs to detonate
	//TODO: A dryfire anim?
	return false;
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

	// We use the an attack cooldown, otherwise (as long the button is held) it'll swap modes every frame!
	//TODO: Just wait for the anim to finish
	if ( m_flSoonestSecondaryAttack < gpGlobals->curtime )
	{
		m_flSoonestSecondaryAttack = gpGlobals->curtime + RC_BOMB_REPLANT_TIME;
	}
	else
	{
		return;
	}

	//HACK: Remove these console messages once we have art!
	if ( m_bDetonatorOn && HasPrimaryAmmo() )
	{
		m_bDetonatorOn = false;
		Msg("You have the bombs.\n");
	}
	else
	{
		m_bDetonatorOn = true;
		Msg("You have the detonator.\n");
	}
}
