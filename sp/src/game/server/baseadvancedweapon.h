//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Base class for cs-like weapons
//
//=====================================================================
#ifndef BASEADVANCEDWEAPON_H
#define BASEADVANCEDWEAPON_H
#ifdef _WIN32
#pragma once
#endif

#include "basehlcombatweapon.h"
#include "hl2_player.h"

//-----------------------------------------------------------------------------
// CBaseAdvancedWeapon
//-----------------------------------------------------------------------------

class CBaseAdvancedWeapon : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CBaseAdvancedWeapon, CBaseHLCombatWeapon);

public:
	CBaseAdvancedWeapon(void);
	DECLARE_SERVERCLASS();

	virtual void ItemHolsterFrame(void);
	virtual void ItemPreFrame(void);
	virtual void ItemPostFrame(void);
	virtual void Equip(CBaseCombatCharacter *pOwner);
	virtual bool Reload(void);
	virtual bool FullAuto(void) { return BaseClass::FullAuto() && !GetBurstMode(); }
	virtual void PrimaryAttack(void);
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual void SecondaryAttack(void);
	virtual void AddViewKick(void);
	virtual bool ShouldDisplayAltFireHUDHint(void);
	virtual void Drop(const Vector &vecVelocity);
	virtual void DryFire(void);
	virtual void ChangeAccuracy(void);
	virtual void ResetBurstCount(void);
	virtual void DecreaseBurstCount(void);
	virtual void SetBurstMode(bool mode);
	virtual bool GetBurstMode(void) { return m_bBurstFire; }
	virtual bool SetZoomLevel(int level = 0);

	virtual Activity GetPrimaryAttackActivity(void);

	virtual const Vector GetBaseAccuracy(void);

	virtual bool IsWeaponZoomed() { return m_nZoomLevel.Get() ? true : false; }

	virtual int GetMinBurst() { return GetBurstMode() ? GetBurstBurst() : 1; }
	virtual int GetMaxBurst() { return GetBurstMode() ? GetBurstBurst() : 10; }
	virtual int GetBurstBurst() { return 3; }
	virtual int CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual const Vector& GetBulletSpread() { return m_vCone; }

protected:
	bool m_bHasLight; // Does this weapon have a flashlight?
	bool m_bHasModes; // Does this weapon have a burst-fire mode?

private:
	int m_iBurstSize; // How many rounds can fire without releasing the button
	int m_iBurst; // How many rounds are left (in this attack)

	bool m_bBurstFire; // Burst-fire mode is active
	bool m_bInBurst; // The weapon is currently bursting, tracked here to force more rounds from the weapon

	float m_flNextAccuracyTick; // When the next accuracy blend frame should run
	float m_flAttackEnds; // Length of the model's attack sequence (for animation)
	CHL2_Player *pPlayer; // The CBasePlayer in CBaseHLCombatWeapon doesn't know if he's crouched, walking or sprinting?

	bool m_bHasScope; // Does this weapon have a scope?
	bool m_bModeSpam; // The player needs to let go of the button

	bool m_bPullingBolt; // Some stuff should not happen if I'm pulling back a rifle bolt
};

#endif // BASEADVANCEDWEAPON_H