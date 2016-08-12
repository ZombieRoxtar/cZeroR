//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Short range weapon with slowburn damage
//
//=============================================================================

#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon.h"
#include "AI_BaseNPC.h"
#include "gamestats.h"
#include "in_buttons.h"
#include "trigger_special_zone.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BLOWTORCH_HUD_ERROR "NotInBlowtorchZone"

//-----------------------------------------------------------------------------
// CWeaponBlowtorch
//-----------------------------------------------------------------------------

class CWeaponBlowtorch : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponBlowtorch, CBaseHLCombatWeapon );

	CWeaponBlowtorch(void);

	DECLARE_SERVERCLASS();

	void	PrimaryAttack( void );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void	ItemPreFrame( void );

	int			CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual bool UsesClipsForAmmo1() { return false; }

	virtual const Vector& GetBulletSpread( void )
	{
		m_vCone = VECTOR_CONE_4DEGREES;
		return m_vCone;
	}

	virtual float GetFireRate( void ) 
	{
		return 0.025f; 
	}
	
private:
	float m_flLastAttackTime;
	float m_flNextDisplayTime;

	bool m_bSpamming;

	CSpecialZone* m_zone;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponBlowtorch, DT_WeaponBlowtorch)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_blowtorch, CWeaponBlowtorch );
PRECACHE_WEAPON_REGISTER( weapon_blowtorch );

BEGIN_DATADESC( CWeaponBlowtorch )
	
	DEFINE_FIELD( m_flLastAttackTime, FIELD_TIME ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponBlowtorch::CWeaponBlowtorch( void )
{
	m_flNextDisplayTime = gpGlobals->curtime;

	m_fMinRange1 = 0;
	m_fMaxRange1 = 128;

	m_bSpamming = false;
}

//-----------------------------------------------------------------------------
void CWeaponBlowtorch::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_BLOWTORCH_FIRE:
		{
			Vector vecShootOrigin, vecShootDir;
			vecShootOrigin = pOperator->Weapon_ShootPosition();

			CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			ASSERT( npc != NULL );

			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(),
				SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );

			WeaponSound( SINGLE_NPC );
			pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
				MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );
			pOperator->DoMuzzleFlash();
		}
		break;
		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

//-----------------------------------------------------------------------------
void CWeaponBlowtorch::PrimaryAttack( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	if ( !pPlayer->IsPlayerInZoneBlowtorch() )
	{
		if ( m_flNextDisplayTime < gpGlobals->curtime )
		{
			m_flNextDisplayTime = gpGlobals->curtime + HUD_ERROR_TIMEOUT;
			UTIL_ShowMessage( BLOWTORCH_HUD_ERROR, pPlayer );
		}
		return;
	}

	m_flLastAttackTime = gpGlobals->curtime;
	if ( m_flNextPrimaryAttack < gpGlobals->curtime )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	}

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_EMPTY, 0.2, GetOwner() );

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( GetPrimaryAttackActivity() );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	FireBulletsInfo_t info;
	info.m_vecSrc = pPlayer->Weapon_ShootPosition();
	
	info.m_vecDirShooting = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );

	WeaponSound( SINGLE );

	info.m_flDistance = m_fMaxRange1;
	info.m_iAmmoType  = m_iPrimaryAmmoType;

	pPlayer->FireBullets( info );

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );

	// Set each time you fire, in case you're in a new zone
	do
	{
		CBaseEntity* pResult = gEntList.FindEntityByClassnameNearest(
			"trigger_special_zone", GetAbsOrigin(), 8192 );
		m_zone = dynamic_cast < CSpecialZone* > ( pResult );
	}while( m_zone->GetType() != ZONE_BLOWTORCH );

	// Once per mouse click is fine!
	if (!m_bSpamming)
	{
		m_bSpamming = true;
		m_zone->StartUsing(pPlayer);
		m_zone->StopUsing(pPlayer);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Reset a varible that stops outputs from spamming when the player lets go
//-----------------------------------------------------------------------------
void CWeaponBlowtorch::ItemPreFrame(void)
{
	BaseClass::ItemPreFrame();

	if (CBasePlayer *pPlayer = ToBasePlayer(GetOwner()))
	{
		if (!(pPlayer->m_nButtons & IN_ATTACK))
		{
			m_bSpamming = false;
		}
	}
}
