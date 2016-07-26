//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Nightvision, kinda hacky
//	Do NOT place this item in Hammer - It does not wait for MyTouch()
//		Use a game_player_equip or other means
//
//=====================================================================
#include "cbase.h"
#include "items.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar cl_nightvisioncolor_r("cl_nightvisioncolor_r", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar cl_nightvisioncolor_g("cl_nightvisioncolor_g", "128", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar cl_nightvisioncolor_b("cl_nightvisioncolor_b", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar cl_nightvisioncolor_a("cl_nightvisioncolor_a", "128", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);

class CNightVision : public CItem
{
public:
	DECLARE_CLASS(CNightVision, CItem);
	DECLARE_SERVERCLASS()

	~CNightVision();

	int UpdateTransmitState() // Always send to all clients
	{
		return SetTransmitState(FL_EDICT_ALWAYS);
	}

	virtual void Spawn(void);
	virtual void Precache(void);
	virtual void Activate(void);

	void StartUp(void);
	void Shutdown(void);
	void Toggle(void);

private:
	int m_nTint_r;
	int m_nTint_g;
	int m_nTint_b;
	int m_nTint_a;

	CNetworkVar(bool, m_bEquipped);
	CNetworkVar(bool, m_bActive);

};

LINK_ENTITY_TO_CLASS(item_nightvision, CNightVision);

IMPLEMENT_SERVERCLASS_ST(CNightVision, DT_NightVision)
SendPropBool(SENDINFO(m_bEquipped)),
SendPropBool(SENDINFO(m_bActive)),
END_SEND_TABLE()

CNightVision *pEquippedNVGs;

//-----------------------------------------------------------------------------
// Purpose: Get color values, spawn, precache, activate
//-----------------------------------------------------------------------------
void CNightVision::Spawn(void)
{
	m_nTint_r = cl_nightvisioncolor_r.GetInt();
	m_nTint_g = cl_nightvisioncolor_g.GetInt();
	m_nTint_b = cl_nightvisioncolor_b.GetInt();
	m_nTint_a = cl_nightvisioncolor_a.GetInt();
	Precache();
	BaseClass::Spawn();
	Activate();
}

CNightVision::~CNightVision()
{
	if (m_bActive)
		Shutdown();
	if(pEquippedNVGs == this)
		pEquippedNVGs = nullptr;
}

void CNightVision::Precache()
{
	BaseClass::Precache();
	PrecacheScriptSound("Player.NightVisionEquip");
	PrecacheScriptSound("Player.NightVisionOn");
	PrecacheScriptSound("Player.NightVisionOff");
}

void CNightVision::Activate()
{
	BaseClass::Activate();
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer)
	{
		EHANDLE activeNVGs = pPlayer->PlayerHasNVGs();
		if (!activeNVGs)
		{
			pEquippedNVGs = this;
			m_bEquipped.Set(true);
			EHANDLE myHandle = this;
			pPlayer->SetPlayerNVGs(myHandle);

			CPASAttenuationFilter filter(pPlayer, "Player.NightVisionEquip");
			EmitSound(filter, pPlayer->entindex(), "Player.NightVisionEquip");
		}
		else
			if (activeNVGs != this)
				Remove();
	}
	else
	{
		DevWarning("Nightvision couldn't find the player!\n");
		Remove();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Console command to force the goggles to turn on.
//-----------------------------------------------------------------------------
static void CC_FadeOut(const CCommand &args)
{
	if (!pEquippedNVGs)
	{
		return;
	}
	pEquippedNVGs->StartUp();
}
static ConCommand nvg_on("nvg_on", CC_FadeOut, "Turns on the NVG test effect.", FCVAR_CHEAT);
//-----------------------------------------------------------------------------
// Purpose: Console command to force the goggles to turn off.
//-----------------------------------------------------------------------------
static void CC_FadeIn(const CCommand &args)
{
	if (!pEquippedNVGs)
	{
		return;
	}
	pEquippedNVGs->Shutdown();
}
static ConCommand nvg_off("nvg_off", CC_FadeIn, "Turns off the NVG test effect.", FCVAR_CHEAT);
//-----------------------------------------------------------------------------
// Purpose: Toggle the goggles!
//-----------------------------------------------------------------------------
static void ToggleNVGs(const CCommand &args)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer)
	{
		if (pPlayer->PlayerHasNVGs())
		{
			pEquippedNVGs->Toggle();
		}
	}
	return;
}
static ConCommand nightvision("nightvision", ToggleNVGs, "Toggles the nightvision goggles (if equipped).");

void CNightVision::Toggle(void)
{
	if (pEquippedNVGs != this) // These goggles are not equipped
	{
		return;
	}
	if (!m_bActive)
	{
		StartUp();
	}
	else
	{
		Shutdown();
	}
}

void CNightVision::StartUp(void)
{
	color32 clrFade;
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

	clrFade.r = cl_nightvisioncolor_r.GetInt();
	clrFade.g = cl_nightvisioncolor_g.GetInt();
	clrFade.b = cl_nightvisioncolor_b.GetInt();
	clrFade.a = cl_nightvisioncolor_a.GetInt();

	UTIL_ScreenFade(pPlayer, clrFade, 0, -1, FFADE_OUT | FFADE_PURGE | FFADE_STAYOUT);
	CPASAttenuationFilter filter(pPlayer, "Player.NightVisionOn");
	EmitSound(filter, pPlayer->entindex(), "Player.NightVisionOn");
	m_bActive.Set(true);
}
void CNightVision::Shutdown(void)
{
	color32 clrFade;
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

	clrFade.r = cl_nightvisioncolor_r.GetInt();
	clrFade.g = cl_nightvisioncolor_g.GetInt();
	clrFade.b = cl_nightvisioncolor_b.GetInt();
	clrFade.a = cl_nightvisioncolor_a.GetInt();
	CPASAttenuationFilter filter(pPlayer, "Player.NightVisionOff");
	EmitSound(filter, pPlayer->entindex(), "Player.NightVisionOff");
	UTIL_ScreenFade(pPlayer, clrFade, 0, 0, FFADE_IN | FFADE_PURGE);
	m_bActive.Set(false);
}
