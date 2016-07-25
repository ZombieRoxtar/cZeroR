//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Mac-10 Weapon
//
//=============================================================================
#include "cbase.h"
#include "baseadvancedweapon.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// CWeaponMac10
//-----------------------------------------------------------------------------
class CWeaponMac10 : public CBaseAdvancedWeapon
{
	public:
		DECLARE_CLASS( CWeaponMac10, CBaseAdvancedWeapon );
		DECLARE_NETWORKCLASS();
		DECLARE_PREDICTABLE();
		DECLARE_ACTTABLE();

		float GetFireRate() { return 0.075f; } // 800 Rounds per minute
};

IMPLEMENT_SERVERCLASS_ST( CWeaponMac10, DT_WeaponMac10 )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_mac10, CWeaponMac10 );
PRECACHE_WEAPON_REGISTER( weapon_mac10 );

acttable_t CWeaponMac10::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },

	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },

	{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },

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

	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	false },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_AR2_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true  },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true  },
};
IMPLEMENT_ACTTABLE( CWeaponMac10 );