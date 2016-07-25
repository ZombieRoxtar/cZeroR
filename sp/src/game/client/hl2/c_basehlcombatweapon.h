//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "basehlcombatweapon_shared.h"

#ifndef C_BASEHLCOMBATWEAPON_H
#define C_BASEHLCOMBATWEAPON_H
#ifdef _WIN32
#pragma once
#endif

class C_HLMachineGun : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS( C_HLMachineGun, C_BaseHLCombatWeapon );
	DECLARE_CLIENTCLASS();
};

class C_HLSelectFireMachineGun : public C_HLMachineGun
{
public:
	DECLARE_CLASS( C_HLSelectFireMachineGun, C_HLMachineGun );
	DECLARE_CLIENTCLASS();
};

class C_BaseHLBludgeonWeapon : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS( C_BaseHLBludgeonWeapon, C_BaseHLCombatWeapon );
	DECLARE_CLIENTCLASS();
};

// CS-esque Weapons
class C_BaseAdvancedWeapon : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS(C_BaseAdvancedWeapon, C_BaseHLCombatWeapon);
	DECLARE_CLIENTCLASS();
};

class C_WeaponMultiGrenade : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS(C_WeaponMultiGrenade, C_BaseHLCombatWeapon);
	DECLARE_CLIENTCLASS();
};

#endif // C_BASEHLCOMBATWEAPON_H
