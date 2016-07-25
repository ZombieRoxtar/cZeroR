//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Prop with a specialized Use function
//
//=============================================================================
#include "cbase.h"
#include "prop_bomb.h"
#include "trigger_special_zone.h"
#include "sprite.h"
#include "explode.h"
#include "in_buttons.h"
#include "vguiscreen.h"
#include "soundent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SF_BOMB_START_ON			 0x00000001
#define SF_BOMB_SHOULD_EXPLODE		 0x00000002 // Explode for real, most just fire the output
#define SF_BOMB_GLOW_AFTER_EXPLOSION 0x00000003 // Glow RED atfer the bomb is "done"
#define SF_BOMB_GLOW_OVERRIDE		 0x00000004

#define BOMB_ZONE_TYPE 5

#define BOMB_FUSE_TIME 30.0f
#define SPRITE_TOGGLE_TIME 0.5f
#define BOMB_BEEP_TIME 2.0f
#define BOMB_THINK_TIME 0.125f

#define BOMB_GLOW_COLOR 255, 0, 0, 160

#define C4_LED_GLOW "sprites/glow02.vmt"

LINK_ENTITY_TO_CLASS(prop_bomb, CPropBomb);
BEGIN_DATADESC(CPropBomb)
DEFINE_FIELD(m_iCaps, FIELD_INTEGER),
DEFINE_FIELD(m_hC4Screen, FIELD_EHANDLE),

DEFINE_KEYFIELD(m_iszZoneName, FIELD_STRING, "ZoneName"),

DEFINE_FUNCTION(Off),
DEFINE_FUNCTION(Tick),

DEFINE_INPUTFUNC(FIELD_VOID, "Start", Start),
DEFINE_INPUTFUNC(FIELD_VOID, "Explode", Explode),

DEFINE_OUTPUT(m_OnPlayerUse, "OnPlayerUse"),
DEFINE_OUTPUT(m_OnPlayerUnUse, "OnPlayerUnUse"),
DEFINE_OUTPUT(m_OnTimerExpired, "OnTimerExpired"),
END_DATADESC()

void CPropBomb::Spawn()
{
	BaseClass::Spawn();

	PrecacheScriptSound("c4.disarmstart");
	PrecacheScriptSound("c4.disarmfinish");
	PrecacheScriptSound("c4.click");
	PrecacheMaterial(C4_LED_GLOW);

	m_iCaps = NULL;

	m_flTickedTime = 0.0f;
	m_flNextBeepTime, m_flNextBlinkTime, m_flNextThinkTime = gpGlobals->curtime;

	if (HasSpawnFlags(SF_BOMB_START_ON))
	{
		variant_t emptyVariant;
		this->AcceptInput("Start", NULL, NULL, emptyVariant, 0);
	}
}

void CPropBomb::Activate()
{
	BaseClass::Activate();

	CBaseEntity* pResult = gEntList.FindEntityByNameWithin
		(NULL, m_iszZoneName.ToCStr(), GetLocalOrigin(), 256);
	m_pDefusezone = dynamic_cast < CSpecialZone* > (pResult);

	if (m_pDefusezone)
		if (m_pDefusezone->GetType() == BOMB_ZONE_TYPE)
			return;

	Warning(
		"The prop_bomb with an incompatible or missing zone at (%f, %f, %f) cannot be defused!\n",
		GetLocalOrigin().x, GetLocalOrigin().y, GetLocalOrigin().z);
}

void CPropBomb::Start(inputdata_t &inputData)
{
	if (!m_bActive)
	{
		m_bActive = true;
		m_iCaps = FCAP_CONTINUOUS_USE;
		m_flStartedTime = gpGlobals->curtime;
		SpawnPanel();
		SpriteStart();

		// We can glow red when either no glow is current, or the mapper has allowed override
		if ((!IsGlowEffectActive()) || (HasSpawnFlags(SF_BOMB_GLOW_OVERRIDE)))
		{
			if (IsGlowEffectActive())
			{
				m_bGlowOverriden = true;
				m_OldColor.r = m_fGlowRed;
				m_OldColor.g = m_fGlowGreen;
				m_OldColor.b = m_fGlowBlue;
				m_OldColor.a = m_fGlowAlpha;
			}
			else
			{
				m_bAutoGlowing = true;
			}
			color32 glowColor = { BOMB_GLOW_COLOR };
			SetGlowEffectColor(glowColor.r, glowColor.g, glowColor.b);
			SetGlowEffectAlpha(glowColor.a);
			AddGlowEffect();
		}

		if (!m_bPlayerOn) // We need to leave the think func set to Off() whilst a player is +USEing this
		{
			SetThink(&CPropBomb::Tick);
			SetNextThink(gpGlobals->curtime + BOMB_THINK_TIME);
		}

		if (!m_pDefusezone) // Just in case...
			Warning(
				"The prop_bomb with an incompatible or missing zone at (%f, %f, %f) cannot be defused!\n",
				GetLocalOrigin().x, GetLocalOrigin().y, GetLocalOrigin().z);
	}
}

void CPropBomb::SpriteStart()
{
	if (!m_bSpriteReady)
	{
		CBaseAnimating *pEntityToSpawnOn = this;

		int nLedAttachmentIndex = pEntityToSpawnOn->LookupAttachment("led");

		if (nLedAttachmentIndex <= 0)
		{
			return;
		}

		Vector vecAttachment;
		pEntityToSpawnOn->GetAttachment(nLedAttachmentIndex, vecAttachment);

		m_hSprite = CSprite::SpriteCreate(C4_LED_GLOW, vecAttachment, false);
		m_pSprite = (CSprite *)m_hSprite.Get();
		if (m_pSprite)
		{
			m_pSprite->SetTransparency(kRenderTransAdd, BOMB_GLOW_COLOR, kRenderFxNone);
			m_pSprite->SetScale(0.125f);
			m_bSpriteReady = true;
		}
	}
	else
	{
		m_pSprite->TurnOn();
	}
}

void CPropBomb::Tick()
{
	m_flTickedTime = gpGlobals->curtime - m_flStartedTime;

	if (m_flNextBeepTime <= gpGlobals->curtime)
	{
		EmitSound("c4.click");
		m_flNextBeepTime = gpGlobals->curtime + (BOMB_BEEP_TIME - (BOMB_BEEP_TIME * (m_flTickedTime / BOMB_FUSE_TIME))); // Beep faster as time ticks away
	}

	if (m_flNextBlinkTime <= gpGlobals->curtime)
	{
		m_pSprite->InputToggleSprite(inputdata_t());
		m_flNextBlinkTime = gpGlobals->curtime + SPRITE_TOGGLE_TIME;
		// Scare NPCs?
		CSoundEnt::InsertSound(SOUND_DANGER, GetAbsOrigin(), 512, SPRITE_TOGGLE_TIME);
	}

	if (!m_bPlayerOn && m_bActive) // We need to leave the think func set to Off() whilst a player is +USEing this
	{
		SetThink(&CPropBomb::Tick);
		SetNextThink(gpGlobals->curtime + BOMB_THINK_TIME);
	}

	if (m_flTickedTime >= BOMB_FUSE_TIME)
	{
		m_OnTimerExpired.FireOutput(this, this);
		m_bActive = false;
		m_iCaps = NULL;
		m_pSprite->TurnOff();
		StopGlowing();

		if (HasSpawnFlags(SF_BOMB_SHOULD_EXPLODE))
		{
			variant_t emptyVariant;
			this->AcceptInput("Explode", NULL, NULL, emptyVariant, 0);
		}
	}
}

void CPropBomb::SpawnPanel()
{
	CBaseAnimating *pEntityToSpawnOn = this;
	char *pOrgLL = "controlpanel0_ll";
	char *pOrgUR = "controlpanel0_ur";

	Assert(pEntityToSpawnOn);

	// Lookup the attachment point...
	int nLLAttachmentIndex = pEntityToSpawnOn->LookupAttachment(pOrgLL);
	int nURAttachmentIndex = pEntityToSpawnOn->LookupAttachment(pOrgUR);

	if ((nLLAttachmentIndex <= 0) || (nURAttachmentIndex <= 0))
	{
		return;
	}

	const char *pScreenName = "c4_panel";
	const char *pScreenClassname = "vgui_screen";

	// Compute the screen size from the attachment points...
	matrix3x4_t	panelToWorld;
	pEntityToSpawnOn->GetAttachment(nLLAttachmentIndex, panelToWorld);

	matrix3x4_t	worldToPanel;
	MatrixInvert(panelToWorld, worldToPanel);

	// Now get the lower right position + transform into panel space
	Vector lr, lrlocal;
	pEntityToSpawnOn->GetAttachment(nURAttachmentIndex, panelToWorld);
	MatrixGetColumn(panelToWorld, 3, lr);
	VectorTransform(lr, worldToPanel, lrlocal);

	float flWidth = lrlocal.x;
	float flHeight = lrlocal.y;

	CVGuiScreen *pScreen = CreateVGuiScreen(pScreenClassname, pScreenName, pEntityToSpawnOn, this, nLLAttachmentIndex);
	pScreen->SetActualSize(flWidth, flHeight);
	pScreen->SetActive(true);
	pScreen->SetTransparency(true);

	m_hC4Screen.Set(pScreen);
}

void CPropBomb::StopGlowing(bool force)
{
	if (!force)
	{
		// If we did an override (if/when we armed) put back the old color, if that's what the mapper wants
		if (m_bGlowOverriden && !HasSpawnFlags(SF_BOMB_GLOW_AFTER_EXPLOSION))
		{
			SetGlowEffectColor(m_OldColor.r, m_OldColor.g, m_OldColor.b);
			SetGlowEffectAlpha(m_OldColor.a);
		}
		else
		{
			if (m_bAutoGlowing) // I want defused (or "exploded") bombs to stop glowing ONLY when auto-red. The mapper can handle it otherwise.
			{
				RemoveGlowEffect();
			}
		}
	}
	else
	{
		RemoveGlowEffect();
	}
	m_bGlowOverriden = false;
	m_bAutoGlowing = false;
}

void CPropBomb::Explode(inputdata_t &inputData)
{
	Assert(m_hC4Screen != NULL);
	m_hC4Screen->SUB_Remove();
	m_pSprite->Remove();
	StopGlowing(true);

	ExplosionCreate(GetLocalOrigin(), GetAbsAngles(), this, 1024, 128, true);
	Remove();
}

void CPropBomb::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!m_bActive)
		return;

	/*
	We need to leave the think func set to Off() whilst a player is +USEing this
	but it's okay, this is func is spammed during that time.
	*/
	Tick();

	// If it's not a player, ignore
	if (!pActivator || !pActivator->IsPlayer())
		return;

	SetThink(&CPropBomb::Off);
	SetNextThink(gpGlobals->curtime + BOMB_THINK_TIME);

	if (!m_bPlayerOn)
	{
		pPlayer = static_cast < CBasePlayer* >(pActivator);

		m_bPlayerOn = true;
		m_OnPlayerUse.FireOutput(pActivator, this);
		if (pPlayer->GetActiveWeapon())
		{
			if (!pPlayer->GetActiveWeapon()->CanHolster() || !pPlayer->GetActiveWeapon()->Holster())
			{
				/* The weapon cannot be holstered
				return;
				//?*/
			}
		}
		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_WEAPONSELECTION;

		if (m_pDefusezone) // Just in case...
			m_pDefusezone->StartUsing(pPlayer);
		EmitSound("c4.disarmstart");
	}
}

// Called by player un-use
void CPropBomb::Off(void)
{
	if (m_bPlayerOn)
	{
		m_bPlayerOn = false;
		m_OnPlayerUnUse.FireOutput(this, this);

		if (m_pDefusezone) // Just in case...
			m_pDefusezone->StopUsing(pPlayer);
		EmitSound("c4.disarmfinish");

		if (pPlayer->GetActiveWeapon())
		{
			if (!pPlayer->GetActiveWeapon()->Deploy())
			{
				if (pPlayer->SwitchToNextBestWeapon(NULL))
				{
					// What if there is no weapon?
				}
			}
		}
		pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_WEAPONSELECTION;
	}
	/* This forces us to pretend the Use key isn't being held down.
	Just in case... */
	pPlayer->m_afButtonPressed |= IN_USE;

	if (m_bActive)
	{
		SetThink(&CPropBomb::Tick);
		SetNextThink(gpGlobals->curtime + BOMB_THINK_TIME);
	}
	else
	{
		SetThink(NULL);
	}
}

// Defused, this is called by the zone
void CPropBomb::Deactivate(void)
{
	m_bActive = false;
	m_hC4Screen->SUB_Remove();
	m_iCaps = NULL;
	m_pSprite->TurnOff();
	StopGlowing();
}
