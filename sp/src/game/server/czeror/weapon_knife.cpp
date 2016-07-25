//========= Copyright Bit Mage's Stuff, All rights probably reserved. =========
//
// Purpose: Combat Knife, also holds the machete, at least for now
//
//=============================================================================
#include "cbase.h"
#include "basehlcombatweapon.h"
#include "in_buttons.h"
#include "soundent.h"
#include "ai_condition.h"
#include "te_effect_dispatch.h"
#include "rumble_shared.h"
#include "GameStats.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BLUDGEON_HULL_DIM 16
static const Vector g_bludgeonMins( -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM );
static const Vector g_bludgeonMaxs( BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM );

ConVar sk_plr_dmg_knife ( "sk_plr_dmg_knife", "10");
ConVar sk_npc_dmg_knife ( "sk_npc_dmg_knife", "5" );
ConVar sk_plr_dmg_knife_stab ( "sk_plr_dmg_knife_stab", "50" );
ConVar sk_npc_dmg_knife_stab ( "sk_npc_dmg_knife_stab", "25" );

//=========================================================
// CWeaponKnife 
//=========================================================
class CWeaponKnife : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponKnife, CBaseHLCombatWeapon );
public:
	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponKnife();
	
	float		GetRange( void )	{ return 64.0f; }
	float		GetFireRate( void )	{ return 1.25f; }
	float		GetFireRate2( void ){ return 2.0f;  } // This is INCLUDING the time it takes to perform the attack

	virtual	void	PrimaryAttack( void );
	virtual	void	SecondaryAttack( void );
	
	virtual void	ItemPostFrame( void );

	virtual Activity	GetPrimaryAttackActivity( void )   { return ACT_VM_HITCENTER; }
	virtual Activity	GetSecondaryAttackActivity( void ) { return ACT_VM_HITCENTER2;} //FIXME: The Knife's stab anim is also ACT_VM_HITCENTER

	virtual	float	GetDamageForActivity( Activity hitActivity );

	virtual int		CapabilitiesGet( void ) { return bits_CAP_MELEE_ATTACK_GROUP; }
	virtual	int		WeaponMeleeAttack1Condition( float flDot, float flDist );

protected:
	virtual	void	ImpactEffect( trace_t &trace );

private:
	bool			ImpactWater( const Vector &start, const Vector &end );
	void			Swing( bool bIsSecondary );
	void			Hit( trace_t &traceHit, Activity nHitActivity, bool bIsSecondary );
	Activity		ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner, bool bIsSecondary );
};

IMPLEMENT_SERVERCLASS_ST( CWeaponKnife, DT_WeaponKnife )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_knife, CWeaponKnife );
LINK_ENTITY_TO_CLASS( weapon_machete, CWeaponKnife );
PRECACHE_WEAPON_REGISTER( weapon_knife );

acttable_t CWeaponKnife::m_acttable[] = 
{
	{ ACT_MELEE_ATTACK1, ACT_MELEE_ATTACK_SWING, true },
	{ ACT_MELEE_ATTACK2, ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,			 ACT_IDLE_ANGRY_MELEE,	 false},
	{ ACT_IDLE_ANGRY,	 ACT_IDLE_ANGRY_MELEE,	 false},
};

IMPLEMENT_ACTTABLE( CWeaponKnife );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponKnife::CWeaponKnife()
{
	m_fMaxRange1 = m_fMaxRange2 = 64;
	if( FStrEq( "weapon_machete", GetClassname() ))
	m_fMaxRange1 = m_fMaxRange2 = m_fMaxRange1 * 2;
}

int CWeaponKnife::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	if( flDist > m_fMaxRange1 )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	else if( flDot < 0.7 )
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}

float CWeaponKnife::GetDamageForActivity( Activity hitActivity )
{
	if( ( GetOwner() != NULL ) && ( GetOwner()->IsPlayer() ) )
	{
		if( hitActivity == ACT_VM_HITCENTER2 )
		{
			return sk_plr_dmg_knife_stab.GetFloat();
		}
		return sk_plr_dmg_knife.GetFloat();
	}
	if( hitActivity == ACT_VM_HITCENTER2 )
	{
		return sk_npc_dmg_knife_stab.GetFloat();
	}
	return sk_npc_dmg_knife.GetFloat();
}

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CWeaponKnife::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	if ( ( pOwner->m_nButtons & IN_ATTACK ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ))
	{
		PrimaryAttack();
	} 
	else if ( ( pOwner->m_nButtons & IN_ATTACK2 ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ))
	{
		SecondaryAttack();
	}
	else 
	{
		WeaponIdle();
		return;
	}
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CWeaponKnife::PrimaryAttack()
{
	Swing( false );
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CWeaponKnife::SecondaryAttack()
{
	Swing( true );
}


//------------------------------------------------------------------------------
// Purpose: Implement impact function
//------------------------------------------------------------------------------
void CWeaponKnife::Hit( trace_t &traceHit, Activity nHitActivity, bool bIsSecondary )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	//Do view kick
	AddViewKick();

	//Make sound for the AI
	CSoundEnt::InsertSound( SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer );

	// This isn't great, but it's something for when the crowbar hits.
	pPlayer->RumbleEffect( RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART );

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if ( pHitEntity != NULL )
	{
		Vector hitDirection;
		pPlayer->EyeVectors( &hitDirection, NULL, NULL );
		VectorNormalize( hitDirection );

		CTakeDamageInfo info( GetOwner(), GetOwner(), GetDamageForActivity( nHitActivity ), DMG_CLUB );

		if( pPlayer && pHitEntity->IsNPC() )
		{
			// If bonking an NPC, adjust damage.
			info.AdjustPlayerDamageInflictedForSkillLevel();
		}

		CalculateMeleeDamageForce( &info, hitDirection, traceHit.endpos );

		pHitEntity->DispatchTraceAttack( info, hitDirection, &traceHit ); 
		ApplyMultiDamage();

		// Now hit all triggers along the ray that... 
		TraceAttackToTriggers( info, traceHit.startpos, traceHit.endpos, hitDirection );

		if ( ToBaseCombatCharacter( pHitEntity ))
		{
			gamestats->Event_WeaponHit( pPlayer, !bIsSecondary, GetClassname(), info );
		}
	}

	// Apply an impact effect
	ImpactEffect( traceHit );
}

Activity CWeaponKnife::ChooseIntersectionPointAndActivity( trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner, bool bIsSecondary )
{
	int			i, j, k;
	float		distance;
	const float	*minmaxs[2] = { mins.Base(), maxs.Base() };
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ( ( vecHullEnd - vecSrc ) * 2 );
	UTIL_TraceLine( vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
	if ( tmpTrace.fraction == 1.0 )
	{
		for ( i = 0; i < 2; i++ )
		{
			for ( j = 0; j < 2; j++ )
			{
				for ( k = 0; k < 2; k++ )
				{
					vecEnd.x = vecHullEnd.x + minmaxs [ i ] [ 0 ];
					vecEnd.y = vecHullEnd.y + minmaxs [ j ] [ 1 ];
					vecEnd.z = vecHullEnd.z + minmaxs [ k ] [ 2 ];

					UTIL_TraceLine( vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace );
					if ( tmpTrace.fraction < 1.0 )
					{
						float thisDistance = ( tmpTrace.endpos - vecSrc ).Length();
						if ( thisDistance < distance )
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}

	if( bIsSecondary )
	{
		return ACT_VM_HITCENTER2;
	}
	return ACT_VM_HITCENTER;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &traceHit - 
//-----------------------------------------------------------------------------
bool CWeaponKnife::ImpactWater( const Vector &start, const Vector &end )
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...
	
	// We must start outside the water
	if ( UTIL_PointContents( start ) & ( CONTENTS_WATER | CONTENTS_SLIME ))
		return false;

	// We must end inside of water
	if ( !( UTIL_PointContents( end ) & ( CONTENTS_WATER | CONTENTS_SLIME )))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine( start, end, ( CONTENTS_WATER | CONTENTS_SLIME ), GetOwner(), COLLISION_GROUP_NONE, &waterTrace );

	if ( waterTrace.fraction < 1.0f )
	{
		CEffectData	data;

		data.m_fFlags  = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if ( waterTrace.contents & CONTENTS_SLIME )
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect( "watersplash", data );			
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponKnife::ImpactEffect( trace_t &traceHit )
{
	// See if we hit water (we don't do the other impact effects in this case)
	if ( ImpactWater( traceHit.startpos, traceHit.endpos ))
		return;

	//FIXME: need new decals
	UTIL_ImpactTrace( &traceHit, DMG_CLUB );
}


//------------------------------------------------------------------------------
// Purpose : Starts the swing of the weapon and determines the animation
// Input   : bIsSecondary - is this a secondary attack?
//------------------------------------------------------------------------------
void CWeaponKnife::Swing( bool bIsSecondary )
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !pOwner )
		return;

	pOwner->RumbleEffect( RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART );

	Vector swingStart = pOwner->Weapon_ShootPosition( );
	Vector forward;

	forward = pOwner->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT, GetRange() );

	Vector swingEnd = swingStart + forward * GetRange();
	UTIL_TraceLine( swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
	Activity nHitActivity = bIsSecondary ? GetSecondaryAttackActivity() : GetPrimaryAttackActivity();

	// Like bullets, bludgeon traces have to trace against triggers
	CTakeDamageInfo triggerInfo( GetOwner(), GetOwner(), GetDamageForActivity( nHitActivity ), DMG_CLUB );
	triggerInfo.SetDamagePosition( traceHit.startpos );
	triggerInfo.SetDamageForce( forward );
	TraceAttackToTriggers( triggerInfo, traceHit.startpos, traceHit.endpos, forward );

	if ( traceHit.fraction == 1.0 )
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull( swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
		if ( traceHit.fraction < 1.0 && traceHit.m_pEnt )
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize( vecToTarget );

			float dot = vecToTarget.Dot( forward );

			// YWB:  Make sure they are sort of facing the guy at least...
			if ( dot < 0.70721f )
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = ChooseIntersectionPointAndActivity( traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner, bIsSecondary );
			}
		}
	}

	if ( !bIsSecondary )
	{
		m_iPrimaryAttacks++;
	} 
	else 
	{
		m_iSecondaryAttacks++;
	}

	gamestats->Event_WeaponFired( pOwner, !bIsSecondary, GetClassname() );

	// -------------------------
	//	Miss
	// -------------------------
	if ( traceHit.fraction == 1.0f )
	{
		nHitActivity = bIsSecondary ? ACT_VM_MISSCENTER : GetPrimaryAttackActivity(); // The only "miss" anim in the knife model is for the STAB

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetRange();
		
		// See if we happened to hit water
		ImpactWater( swingStart, testEnd );
	}
	else
	{
		Hit( traceHit, nHitActivity, bIsSecondary ? true : false );
	}

	// Send the anim
	SendWeaponAnim( nHitActivity );

	// Setup our next attack times
	float fCoolDown = GetFireRate();
	if ( bIsSecondary )
	{
		fCoolDown = GetFireRate2();
	}
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + fCoolDown;

	// Play swing sound
	WeaponSound( SINGLE );
}
