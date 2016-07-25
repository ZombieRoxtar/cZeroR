//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: The weapon formerly known as the M3 Super
//
//=====================================================================

#include "cbase.h"
#include "baseadvancedweapon.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "in_buttons.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sk_plr_num_shotgun_pellets;

class CWeaponNova : public CBaseAdvancedWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponNova, CBaseAdvancedWeapon);
	DECLARE_SERVERCLASS();

	DECLARE_ACTTABLE();

	CWeaponNova( void );

	virtual const Vector& GetBulletSpread() { return m_vCone; }

	virtual float GetFireRate() { return 1.1333333333333333333333333333333f; } // 68 RPM

	bool StartReload( void );
	bool Reload( void );
	void FillClip( void );
	void FinishReload( void );
	void ItemPostFrame( void );
	void PrimaryAttack( void );

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );
	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

private:
	bool	m_bDelayedFire; // Fire when finished reloading
};

IMPLEMENT_SERVERCLASS_ST( CWeaponNova, DT_WeaponNova )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_nova, CWeaponNova );
PRECACHE_WEAPON_REGISTER( weapon_nova );

BEGIN_DATADESC( CWeaponNova )
	DEFINE_FIELD( m_bDelayedFire, FIELD_BOOLEAN ),
END_DATADESC()

acttable_t CWeaponNova::m_acttable[] = 
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },

	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,		true },
	{ ACT_RELOAD,					ACT_RELOAD_SHOTGUN,				false },
	{ ACT_WALK,						ACT_WALK_RIFLE,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SHOTGUN,			true },

	{ ACT_IDLE_RELAXED,				ACT_IDLE_SHOTGUN_RELAXED,		false },
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SHOTGUN_STIMULATED,	false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_SHOTGUN_AGITATED,		false },

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },

	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },

	{ ACT_WALK_AIM,					ACT_WALK_AIM_SHOTGUN,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,				true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,			true },
	{ ACT_RUN,						ACT_RUN_RIFLE,						true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_SHOTGUN,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,				true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,			true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SHOTGUN_LOW,		true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SHOTGUN_LOW,				false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SHOTGUN,			false },
};

IMPLEMENT_ACTTABLE( CWeaponNova );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponNova::CWeaponNova( void )
{
	m_bHasLight = m_bReloadsSingly = true;
	m_bFiresUnderwater = m_bDelayedFire = false;
}

//-----------------------------------------------------------------------------
// Purpose:  NPC Override?
// Input  : *pOperator
//-----------------------------------------------------------------------------
void CWeaponNova::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles )
{
	Vector vecShootOrigin, vecShootDir;
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT( npc != NULL );
	WeaponSound( SINGLE_NPC );
	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;

	if ( bUseWeaponAngles )
	{
		QAngle	angShootDir;
		GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
		AngleVectors( angShootDir, &vecShootDir );
	}
	else 
	{
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );
	}
	pOperator->FireBullets( 8, vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: NPC Override?
//-----------------------------------------------------------------------------
void CWeaponNova::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;
	FireNPCPrimaryAttack( pOperator, true );
}

//-----------------------------------------------------------------------------
// Purpose: NPC Override?
//-----------------------------------------------------------------------------
void CWeaponNova::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
		case EVENT_WEAPON_SHOTGUN_FIRE:
		{
			FireNPCPrimaryAttack( pOperator, false );
		}
		break;

		default:
			CBaseCombatWeapon::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override so only the gun only reloads one shell at a time
//-----------------------------------------------------------------------------
bool CWeaponNova::StartReload( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	
	if ( pOwner == NULL )
		return false;

	if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return false;

	if ( m_iClip1 >= GetMaxClip1() )
		return false;

	int j = MIN( 1, pOwner->GetAmmoCount( m_iPrimaryAmmoType ));

	if (j <= 0 )
		return false;

	SendWeaponAnim( ACT_SHOTGUN_RELOAD_START );

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack  = gpGlobals->curtime + SequenceDuration();

	m_bInReload = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only the gun only reloads one shell at a time
//-----------------------------------------------------------------------------
bool CWeaponNova::Reload( void )
{
	// Check that StartReload was called first
	if ( !m_bInReload )
	{
		Warning( "ERROR: Nova Reload called incorrectly!\n" );
	}

	CBaseCombatCharacter *pOwner = GetOwner();
	
	if ( pOwner == NULL )
		return false;

	if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType) <= 0 )
		return false;

	if ( m_iClip1 >= GetMaxClip1() )
		return false;

	int j = MIN( 1, pOwner->GetAmmoCount( m_iPrimaryAmmoType ));

	if (j <= 0 )
		return false;

	FillClip();
	// Play reload on different channel as otherwise it steals the channel away from the fire sound
	WeaponSound( RELOAD );
	SendWeaponAnim( ACT_VM_RELOAD );

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack  = gpGlobals->curtime + SequenceDuration();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim
//-----------------------------------------------------------------------------
void CWeaponNova::FinishReload( void )
{
	// Make shotgun shell invisible
	SetBodygroup( 1, 1 );

	CBaseCombatCharacter *pOwner = GetOwner();
	
	if ( pOwner == NULL )
		return;

	m_bInReload = false;

	// Finish reload animation
	SendWeaponAnim( ACT_SHOTGUN_RELOAD_FINISH );

	BaseClass::ResetBurstCount();

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack  = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: Add shells to the "clip"
//-----------------------------------------------------------------------------
void CWeaponNova::FillClip( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	
	if ( pOwner == NULL )
		return;

	// Add them to the clip
	if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) > 0 )
	{
		if ( Clip1() < GetMaxClip1() )
		{
			m_iClip1++;
			pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Primary Attack
//-----------------------------------------------------------------------------
void CWeaponNova::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound( SINGLE );

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_iClip1--;

	Vector vecSrc	 = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );

	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 1.0 );
	
	// Fire the bullets, and force the first shot to be perfectly accuracy
	pPlayer->FireBullets( sk_plr_num_shotgun_pellets.GetInt(), vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0, -1, -1, 0, NULL, true, true );
	
	pPlayer->ViewPunch( QAngle( random->RandomFloat( -2, -1 ), random->RandomFloat( -2, 2 ), 0 ) );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2, GetOwner() );

	BaseClass::DecreaseBurstCount();
	
	// Does the accuracy need to shift?
	ChangeAccuracy();

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );
}

//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do mulitple reloads in a row
//-----------------------------------------------------------------------------
void CWeaponNova::ItemPostFrame( void )
{
	// Does the accuracy need to shift?
	ChangeAccuracy();
	
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
	{
		return;
	}

	if ( m_bInReload )
	{
		// If I'm primary firing and have one round stop reloading and fire
		if (( pOwner->m_nButtons & IN_ATTACK ) && ( m_iClip1 >=1 ))
		{
			m_bInReload	= false;
			m_bDelayedFire = true;
		}
		else if ( m_flNextPrimaryAttack <= gpGlobals->curtime )
		{
			// If out of ammo end reload
			if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
			{
				FinishReload();
				return;
			}
			// If clip not full reload again
			if ( m_iClip1 < GetMaxClip1() )
			{
				Reload();
				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload();
				return;
			}
		}
	}

	if ( ( m_bDelayedFire || pOwner->m_nButtons & IN_ATTACK ) && m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		m_bDelayedFire = false;
		if ( ( m_iClip1 <= 0 && UsesClipsForAmmo1()) || ( !UsesClipsForAmmo1() && !pOwner->GetAmmoCount( m_iPrimaryAmmoType )))
		{
			if ( !pOwner->GetAmmoCount( m_iPrimaryAmmoType))
			{
				DryFire();
			}
			else
			{
				StartReload();
			}
		}
		// Fire underwater?
		else if ( pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false )
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			// If the firing button was just pressed, reset the firing time
			CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
			if ( pPlayer && pPlayer->m_afButtonPressed & IN_ATTACK )
			{
				 m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			PrimaryAttack();
		}
	}

	if ( ( pOwner->m_nButtons & IN_ATTACK2 )  )
	{
		// Fire underwater?
		if ( pOwner->GetWaterLevel() == 3 && m_bAltFiresUnderwater == false )
		{
			WeaponSound( EMPTY );
			return;
		}
		else
		{
			SecondaryAttack();
		}
	}

	if ( pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload ) 
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		StartReload();
	}
	else 
	{
		// no fire buttons down
		m_bFireOnEmpty = false;

		if ( !HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime ) 
		{
			// weapon isn't useable, switch.
			if ( !( GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY ) && pOwner->SwitchToNextBestWeapon( this ))
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
				return;
			}
		}
		else
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if ( m_iClip1 <= 0 && !( GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD ) && m_flNextPrimaryAttack < gpGlobals->curtime )
			{
				if ( StartReload() )
				{
					// if we've successfully started to reload, we're done
					return;
				}
			}
		}
		WeaponIdle();
		return;
	}
}