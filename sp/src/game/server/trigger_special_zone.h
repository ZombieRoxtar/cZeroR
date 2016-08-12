//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====

// Purpose: Brush entity that permits the use of the specified equipment
//
//=============================================================================

#ifndef C_SpecialZone_H
#define C_SpecialZone_H
#ifdef _WIN32
#pragma once
#endif

#include "triggers.h"

// How long to hold an error on the HUD (e.g. You are not in a radio zone.)
#define HUD_ERROR_TIMEOUT 5.0f

class CEnvScreenOverlay;
class CPropBomb;
class CNPC_Hostage;

/*
Type of zone:
0 = Stealth
1 = RC Bomb
2 = Blowtorch
3 = Fiberoptic Camera
4 = Radio
5 = (Bomb) Defuse
6 = (Digital) Camera
7 = Briefcase
8 = (Hostage) Rescue
*/
enum zoneType
{
	ZONE_STEALTH, // Stealth Zone
	ZONE_RC_BOMB, // Radio-Controlled Bomb Zone
	ZONE_BLOWTORCH, // Blowtorch Zone
	ZONE_FO_CAMERA, // Fiberoptic Camera Zone
	ZONE_RADIO, // Radio Zone
	ZONE_DEFUSE, // Bomb Defusal Zone
	ZONE_CAMERA, // Digital Camera Zone
	ZONE_BRF_CASE, // Briefcase Zone
	ZONE_RESCUE // Hostage Rescue Zone
};

//=============================================================================
class CSpecialZone : public CBaseTrigger
{
public:
	DECLARE_CLASS(CSpecialZone, CBaseTrigger);
	DECLARE_DATADESC();

	CSpecialZone();
	void Spawn();
	void Activate();
	virtual void InputDisable(inputdata_t &inputdata); // Overridden to force hudtext off when disabled
	void Disable(void);
	zoneType GetType() { return m_nType; };
	void Touch(CBaseEntity *pOther);
	void StartTouch(CBaseEntity *pOther);
	void EndTouch(CBaseEntity *pOther);
	void Success(CBaseEntity *pActivator);
	void StartUsing(CBasePlayer *pPlayer);
	void StopUsing(CBasePlayer *pPlayer);
	void ToggleUse(CBasePlayer *pPlayer);
	void Aborted(CBasePlayer *pPlayer);
	void Think();
	bool Looking();


private:

	COutputEvent OnSuccess; /* weapon fired, bomb defused, Hostage rescued, ... */
	COutputEvent OnAborted; /* Player quit too soon */
	COutputEvent OnStartUsing;/* Player has begun defusing, blowtorching, ... */
	COutputEvent OnStopUsing; /* Player quit defusing, blowtorching, ... */

							  /*
							  The placeholder R\C Bomb
							  Blowtorch lock?
							  Fiberoptic Camera point_viewcontroller
							  Bomb (prop) to defuse
							  Where you need to be looking
							  for Camera and Briefcase
							  */
	string_t m_iszTargetEnt;
	string_t m_iszHudtext; /* The text "You are in a ... Zone." */
	string_t m_iszUnHudtext; /* Titles.txt entry to clear m_iszHudtext */

	CBasePlayer *Player;
	CBaseEntity *m_pTarget; /* Should hold our "Target" */
	CPropBomb *m_pBomb; /* Cast from m_pTarget to call bomb functions */
	CEnvScreenOverlay *m_pOverlay; /* Cast from m_pTarget to lay over fiberoptic camera */
	CTriggerCamera *m_pViewport; /* Cast from m_pTarget to call fiberoptic camera functions */
	CNPC_Hostage *m_pHostage; /* Calls Rescue */

	zoneType m_nType; /* Type of zone */

	float m_flFieldOfView; /* The look tolerance */
	float m_flDefuseTime; /* How long it takes to defuse bombs (for defuse zones). */
	float m_flDefuseProgress; /* How long the player has been defusing (for defuse zones). */

	bool m_bUsing; /* Player is begun defusing, blowtorching, ... */

				   /* For deciding if the HUD should be updated for look-dependent zone states */
	bool m_bWereLooking;

	IntervalTimer m_DefuseTimer;
};

#endif // C_SpecialZone_H
