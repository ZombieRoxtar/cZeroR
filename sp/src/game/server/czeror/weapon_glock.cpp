//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Glock Weapon
//
//=============================================================================
#include "cbase.h"
#include "baseadvancedweapon.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// CWeaponGlock
//-----------------------------------------------------------------------------

class CWeaponGlock : public CBaseAdvancedWeapon
{
	public:
		CWeaponGlock() { m_bHasModes = true; }
		DECLARE_CLASS( CWeaponGlock, CBaseAdvancedWeapon );
		DECLARE_NETWORKCLASS();
		DECLARE_PREDICTABLE();

		float GetFireRate();

		DECLARE_ACTTABLE();
};

float CWeaponGlock::GetFireRate()
{
	if( GetBurstMode() )
	{
		return 0.05f; // 1,200 RPM (Burst fire)
	}
	else
	{
		return 0.15f; // 400 RPM (Semi auto)
	}
}

IMPLEMENT_SERVERCLASS_ST( CWeaponGlock, DT_WeaponGlock )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_glock, CWeaponGlock );
PRECACHE_WEAPON_REGISTER( weapon_glock );

acttable_t CWeaponGlock::m_acttable[] =
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

IMPLEMENT_ACTTABLE( CWeaponGlock );