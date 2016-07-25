//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Mag-7 Weapon
//
//=============================================================================
#include "cbase.h"
#include "baseadvancedweapon.h"
#include "soundent.h"
#include "gamestats.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sk_plr_num_shotgun_pellets;

//-----------------------------------------------------------------------------
// CWeaponMag7
//-----------------------------------------------------------------------------
class CWeaponMag7 : public CBaseAdvancedWeapon
{
	public:
		DECLARE_CLASS( CWeaponMag7, CBaseAdvancedWeapon );
		DECLARE_NETWORKCLASS();
		DECLARE_PREDICTABLE();
		DECLARE_ACTTABLE();

		float GetFireRate() { return 0.6521739130434782608695652173913f; } // 92 Rounds per minute
		void PrimaryAttack( void );

private:
	bool	m_bDelayedFire; // Fire when finished reloading
};

IMPLEMENT_SERVERCLASS_ST( CWeaponMag7, DT_WeaponMag7 )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_mag7, CWeaponMag7 );
PRECACHE_WEAPON_REGISTER( weapon_mag7 );

acttable_t CWeaponMag7::m_acttable[] =
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },

	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,		true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				false },
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
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,				false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,			false },
};

IMPLEMENT_ACTTABLE( CWeaponMag7 );

//-----------------------------------------------------------------------------
void CWeaponMag7::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	WeaponSound( SINGLE );

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_iClip1--;

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );	

	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 1.0 );
	
	// Fire the bullets, and force the first shot to be perfectly accurate
	pPlayer->FireBullets( sk_plr_num_shotgun_pellets.GetInt(), vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0, -1, -1, 0, NULL, true, true );
	
	pPlayer->ViewPunch( QAngle( random->RandomFloat( -2, -1 ), random->RandomFloat( -2, 2 ), 0 ) );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2, GetOwner() );

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );

	BaseClass::DecreaseBurstCount();

	// Does the accuracy need to shift?
	ChangeAccuracy();
}