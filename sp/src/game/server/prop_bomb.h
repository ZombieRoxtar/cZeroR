//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Prop with a specialized Use function
//
//=============================================================================

#ifndef C_PROP_BOMB_H
#define C_PROP_BOMB_H
#ifdef _WIN32
#pragma once
#endif

#include "props.h"

class CSpecialZone;
class CSprite;

class CPropBomb : public CBaseProp
{
public:
	DECLARE_CLASS(CPropBomb, CBaseProp);

	void Spawn(void);
	void Precache(void);
	void Activate(void);
	void InputStart(inputdata_t &inputData);
	void Start(void);
	void Countdown(void);
	void SpriteStart(void);
	void Tick(void);
	void StopGlowing(bool force = false);
	void Explode(inputdata_t &inputData);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void Off(void);
	void Deactivate(void);
	virtual int	ObjectCaps(void) { return (BaseClass::ObjectCaps() | m_iCaps); }
	void SpawnPanel();

private:
	DECLARE_DATADESC();

	CBasePlayer* pPlayer;
	CSpecialZone* m_pDefusezone;
	CSprite* m_pSprite;

	string_t m_iszZoneName;

	EHANDLE m_hSprite;
	EHANDLE	m_hC4Screen;

	bool m_bActive;
	bool m_bPlayerOn;
	bool m_bSpriteReady;
	bool m_bAutoGlowing; // Are we using the automatic red glow?
	bool m_bGlowOverriden; // Did we override the color?

	color32 m_OldColor; // We can restore the old color after an override

	int	m_iCaps;

	float m_flStartedTime;
	float m_flTickedTime;
	float m_flNextBeepTime;
	float m_flNextBlinkTime;
	float m_flNextThinkTime;

	COutputEvent m_OnPlayerUse;
	COutputEvent m_OnPlayerUnUse;
	COutputEvent m_OnTimerExpired;
};
#endif // C_PROP_BOMB_H