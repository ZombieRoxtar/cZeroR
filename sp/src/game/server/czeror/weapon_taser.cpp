//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Zeus x27 Stun Gun
//
//=============================================================================
#include "cbase.h"
#include "baseadvancedweapon.h"
#include "gamestats.h"
#include "soundent.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MAX_TRACE_LENGTH_TASER 64.0f

//-----------------------------------------------------------------------------
// CWeaponTaser
//-----------------------------------------------------------------------------
class CWeaponTaser : public CBaseAdvancedWeapon
{
	public:
		DECLARE_CLASS( CWeaponTaser, CBaseAdvancedWeapon );
		DECLARE_NETWORKCLASS();
		DECLARE_PREDICTABLE();
		DECLARE_ACTTABLE();
		virtual void PrimaryAttack();
};

IMPLEMENT_SERVERCLASS_ST( CWeaponTaser, DT_WeaponTaser )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_taser, CWeaponTaser );
PRECACHE_WEAPON_REGISTER( weapon_taser );

void CWeaponTaser::PrimaryAttack()
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
	pPlayer->FireBullets( 1, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH_TASER, m_iPrimaryAmmoType, 0, -1, -1, 0, NULL, true, true );
	
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, GetOwner() );

	Vector vForward, vRight, vUp;
	pPlayer->EyeVectors( &vForward, &vRight, &vUp );
	Vector muzzlePoint = vecSrc + vForward * MAX_TRACE_LENGTH_TASER + vRight * 6.0f + vUp * 3.0f;
	QAngle vecAngles;
	VectorAngles( vForward, vecAngles );
	// Only do these effects if we're not submerged
	if (! ( UTIL_PointContents( muzzlePoint ) & CONTENTS_WATER ))
	{
		// draw sparks
		for ( int i = 0; i < random->RandomInt( 1, 4 ); i++ )
		{
			Create( "spark_shower", muzzlePoint, vecAngles, NULL );
		}
	}

	DecreaseBurstCount();
	// Does the accuracy need to shift?
	ChangeAccuracy();

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );

	// If I'm now out of ammo, switch away
	if ( !HasPrimaryAmmo() )
	{
		pPlayer->SwitchToNextBestWeapon( this );
	}
}

acttable_t CWeaponTaser::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
	{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },

	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },

	{ ACT_WALK,						ACT_WALK_PISTOL,				true },

	{ ACT_IDLE_RELAXED,				ACT_IDLE_PISTOL,				false },
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_PISTOL,				false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_PISTOL,			false },

	{ ACT_WALK_RELAXED,				ACT_WALK_PISTOL,				false },
	{ ACT_WALK_STIMULATED,			ACT_WALK_PISTOL,				false },
	{ ACT_WALK_AGITATED,			ACT_WALK_PISTOL,				false },

	{ ACT_RUN_RELAXED,				ACT_RUN_PISTOL,					false },
	{ ACT_RUN_STIMULATED,			ACT_RUN_PISTOL,					false },
	{ ACT_RUN_AGITATED,				ACT_RUN_PISTOL,					false },

	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_PISTOL,				false },
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_PISTOL,				false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_PISTOL,			false },

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_AIM_PISTOL,			false },
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_PISTOL,			false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_PISTOL,			false },

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_AIM_PISTOL,				false },
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_PISTOL,				false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_PISTOL,				false },

	{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,				true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,			true },
	{ ACT_RUN,						ACT_RUN_PISTOL,						true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,					true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,				true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,			true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,	false },
	{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,		true  },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,				false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,			true  },
};

IMPLEMENT_ACTTABLE( CWeaponTaser );