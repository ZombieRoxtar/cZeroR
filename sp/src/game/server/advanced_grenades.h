//========= Copyright Bit Mage's Stuff, All rights probably reserved. =========
//
// Purpose: Let's try putting all the grenades in one file...
//
//=============================================================================
#ifndef ADVANCED_GRENADES_H
#define ADVANCED_GRENADES_H
#pragma once

#define DECOY_GRENADE 1
#define FLASH_GRENADE 2
#define HI_EX_GRENADE 3
#define INCEN_GRENADE 4
#define MOLOT_GRENADE 5
#define SMOKE_GRENADE 6

#define FIRST_GRENADE_TYPE DECOY_GRENADE
#define LAST_GRENADE_TYPE  SMOKE_GRENADE

class CAdvancedGrenade;

CAdvancedGrenade *Grenade_Create(const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, int type);

#endif // ADVANCED_GRENADES_H