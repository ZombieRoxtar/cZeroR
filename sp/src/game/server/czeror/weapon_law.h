//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: The L.A.W. (Light, Anti-vehicle Weapon), single-shot RPG
//
//=============================================================================
#ifndef WEAPON_LAW_H
#define WEAPON_LAW_H

#ifdef _WIN32
#pragma once
#endif

#include "basehlcombatweapon.h"
#include "npcevent.h"

class CWeaponLAW;
class RocketTrail;
 
//=============================================================================
//	CRocket
//=============================================================================
class CRocket : public CBaseCombatCharacter
{
	DECLARE_CLASS( CRocket, CBaseCombatCharacter );

public:
	static const int EXPLOSION_RADIUS = 200;

	CRocket();

	Class_T Classify( void ) { return CLASS_MISSILE; }
	
	void	Spawn( void );
	void	Precache( void );
	void	MissileTouch( CBaseEntity *pOther );
	void	Explode( void );
	void	ShotDown( void );
	void	AccelerateThink( void );
	void	AugerThink( void );
	void	IgniteThink( void );
	void	DumbFire( void );
	void	SetGracePeriod( float flGracePeriod );

	int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void	Event_Killed( const CTakeDamageInfo &info );
	
	virtual float	GetDamage() { return m_flDamage; }
	virtual void	SetDamage(float flDamage) { m_flDamage = flDamage; }

	unsigned int PhysicsSolidMaskForEntity( void ) const;

	CHandle<CWeaponLAW> m_hOwner;

	static CRocket *Create( const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner );

	void CreateDangerSounds( bool bState ){ m_bCreateDangerSounds = bState; }

protected:
	virtual void DoExplosion();	
	virtual int AugerHealth() { return m_iMaxHealth - 20; }

	// Creates the smoke trail
	void CreateSmokeTrail( void );

	CHandle<RocketTrail>	m_hRocketTrail;
	float					m_flAugerTime; // Amount of time to auger before blowing up anyway
	float					m_flMarkDeadTime;
	float					m_flDamage;

	struct CustomDetonator_t
	{
		EHANDLE hEntity;
		float radiusSq;
		float halfHeight;
	};

	static CUtlVector<CustomDetonator_t> gm_CustomDetonators;

private:
	float					m_flGracePeriodEndsAt;
	bool					m_bCreateDangerSounds;

	DECLARE_DATADESC();
};

//-----------------------------------------------------------------------------
// LAW
//-----------------------------------------------------------------------------
class CWeaponLAW : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponLAW, CBaseHLCombatWeapon );
public:

	CWeaponLAW();

	DECLARE_SERVERCLASS();

	void	Precache( void );

	void	PrimaryAttack( void );
	virtual float GetFireRate( void ) { return 1; };
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );

	bool	Reload( void );
	bool	WeaponShouldBeLowered( void );

	int		GetMinBurst() { return 1; }
	int		GetMaxBurst() { return 1; }
	float	GetMinRestTime() { return 4.0; }
	float	GetMaxRestTime() { return 4.0; }

	bool	WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );
	int		WeaponRangeAttack1Condition( float flDot, float flDist );

	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	void	NotifyRocketDied( void );

	int	CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual const Vector& GetBulletSpread( void ) {
		static const Vector cone = VECTOR_CONE_3DEGREES;
		return cone;
	}
	
	CBaseEntity *GetMissile( void ) { return m_hMissile; }

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
	
protected:
	CHandle<CRocket>	m_hMissile;
};

#endif // WEAPON_LAW_H