//========== Copyright Bit Mage's Stuff, All rights probably reserved. ===========//
//
// Purpose: Baseclass for CZ's grenades
//
//===============================================================================//
#ifndef BASE_ADVANCED_GRENADES_H
#define BASE_ADVANCED_GRENADES_H
#ifdef _WIN32
#pragma once
#endif

#include "basehlcombatweapon.h"
#include "advanced_grenades.h"

//-----------------------------------------------------------------------------
// Grenades
//-----------------------------------------------------------------------------
class CWeaponMultiGrenade : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponMultiGrenade, CBaseHLCombatWeapon);
	DECLARE_SERVERCLASS();

	CWeaponMultiGrenade();

	void	Precache(void);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	void	PrimaryAttack(void);

	void	DecrementAmmo(CBaseCombatCharacter *pOwner);
	void	ItemPostFrame(void);

	bool	Deploy(void);
	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	void	SetType(int type);
	int		GetType(void) { return m_nType; }

	bool	Reload(void);

private:
	void	ThrowGrenade(CBasePlayer *pPlayer);
	void	LobGrenade(CBasePlayer *pPlayer);

	void	CheckThrowPosition(CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc);

	bool	m_bRedraw;
	bool	m_fDrawbackFinished;

	int		m_nType;
	int		m_AttackPaused;

	DECLARE_DATADESC();
	DECLARE_ACTTABLE();
};
#endif // BASE_ADVANCED_GRENADES_H