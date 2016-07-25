//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Brush entity that permits the use of the specified equipment
//
//=============================================================================
#include "cbase.h"
#include "trigger_special_zone.h"
#include "npc_hostage.h"
#include "prop_bomb.h"
#include "filters.h"
#include "env_screenoverlay.h"
#include "explode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BLOWTORCH_PROP_HEALTH 30
#define BLOWTORCH_DAMAGE_TYPE DMG_SLOWBURN // conventional fires\explosives do DMG_BURN
#define FIBEROPTICCAMERA_OVERLAY "effects/combine_binocoverlay"

LINK_ENTITY_TO_CLASS(trigger_special_zone, CSpecialZone);

BEGIN_DATADESC(CSpecialZone)

DEFINE_THINKFUNC(Think),

DEFINE_KEYFIELD(m_nType, FIELD_INTEGER, "Type"),
DEFINE_KEYFIELD(m_iszTargetEnt, FIELD_STRING, "TargetEnt"),
DEFINE_KEYFIELD(m_flDefuseTime, FIELD_FLOAT, "DefuseTime"),
DEFINE_KEYFIELD(m_flFieldOfView, FIELD_FLOAT, "FieldOfVeiw"),

DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable), // Overridden to force hudtext off when disabled

DEFINE_OUTPUT(OnStartUsing, "OnStartUsing"),
DEFINE_OUTPUT(OnStopUsing, "OnStopUsing"),
DEFINE_OUTPUT(OnSuccess, "OnSuccess"),
DEFINE_OUTPUT(OnAborted, "OnAborted"),

END_DATADESC()

CSpecialZone::CSpecialZone()
{
	m_iszHudtext = castable_string_t("InStealthZone");
	m_iszUnHudtext = castable_string_t("OutStealthZone");
	m_flFieldOfView = 0.9f;
	m_flDefuseTime = 10.0f;
	m_flDefuseProgress = 0.0f;
	m_nType = ZONE_STEALTH;
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CSpecialZone::Spawn()
{
	BaseClass::Spawn();
	InitTrigger();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the a link with our target ent
//-----------------------------------------------------------------------------
void CSpecialZone::Activate()
{
	BaseClass::Activate();

	CBaseEntity* pResult = gEntList.FindEntityByNameWithin
		(NULL, m_iszTargetEnt.ToCStr(), GetAbsOrigin(), 0);
	m_pTarget = pResult;

	FilterDamageType* m_bt_filter;
	castable_string_t AutoEntName("SpecialZone_AutoEnt");

	switch (m_nType)
	{
		// case 0, Stealth Zone, No target
	case ZONE_RC_BOMB: // RC Bomb
					   /*
					   The (placeholder or "ghost") RC bomb is optional,
					   but some sort of (look) target is required.
					   (CBaseEntity already supports renderfx.)
					   */
		if (m_pTarget)
		{
			m_pTarget->SetRenderMode(kRenderWorldGlow);
			m_pTarget->m_nRenderFX = kRenderFxHologram;
			m_pTarget->SetRenderColor(255, 0, 0, 192);
			m_pTarget->SetSolid(SOLID_NONE);
		}
		else
		{
			Warning(
				"R/C Bomb zone with missing target at (%f, %f, %f).\n",
				GetLocalOrigin().x, GetLocalOrigin().y, GetLocalOrigin().z);
		}
		break;
	case ZONE_BLOWTORCH: // Blowtorch
						 /* optional, gets a damage filter and set amount of HP */
		if (m_pTarget)
		{
			m_bt_filter = dynamic_cast < FilterDamageType* >
				(Create("filter_damage_type", GetLocalOrigin(), GetLocalAngles()));
			AutoEntName = "filter_blowtorchzone_autocreated";
			if (m_bt_filter)
			{
				m_bt_filter->m_iDamageType |= BLOWTORCH_DAMAGE_TYPE;
				m_bt_filter->SetName(castable_string_t(AutoEntName));
				m_pTarget->m_iszDamageFilterName = castable_string_t(AutoEntName);
				m_pTarget->SetHealth(BLOWTORCH_PROP_HEALTH);
			}
			else
			{
				Warning(
					"Blowtorch zone at (%f, %f, %f) could not create damage filter.\n",
					GetLocalOrigin().x, GetLocalOrigin().y, GetLocalOrigin().z);
			}
		}
		break;
	case ZONE_FO_CAMERA: // Fiberoptic Camera
						 /* point_viewcontrol, activated by Fiberoptic Camera */
		m_pViewport = dynamic_cast < CTriggerCamera* > (pResult);
		if (m_pViewport)
		{
			m_pOverlay = dynamic_cast < CEnvScreenOverlay* >
				(Create("env_screenoverlay", GetLocalOrigin(), GetLocalAngles()));
			AutoEntName = "overlay_fiberopticcamerazone_autocreated";
			if (m_pOverlay)
			{
				PrecacheMaterial(FIBEROPTICCAMERA_OVERLAY);
				m_pOverlay->KeyValue("OverlayName1", FIBEROPTICCAMERA_OVERLAY);
				m_pOverlay->KeyValue("OverlayTime1", "-1.0");
				m_pOverlay->SetName(castable_string_t(AutoEntName));
			}
			else
			{
				Warning(
					"Fiberoptic Camera zone at (%f, %f, %f) could not create overlay.\n",
					GetLocalOrigin().x, GetLocalOrigin().y, GetLocalOrigin().z);
			}
		}
		else
		{
			Warning(
				"Fiberoptic Camera zone with incompatible or missing target at (%f, %f, %f).\n",
				GetLocalOrigin().x, GetLocalOrigin().y, GetLocalOrigin().z);
		}
		break;
		// case 4, Radio Zone, No target
	case ZONE_DEFUSE: // Defuse, Bomb
		m_pBomb = dynamic_cast < CPropBomb* > (pResult);
		if (!m_pBomb)
		{
			Warning(
				"Defuse zone with incompatible or missing target at (%f, %f, %f).\n",
				GetLocalOrigin().x, GetLocalOrigin().y, GetLocalOrigin().z);
		}
		break;
	case ZONE_CAMERA: // Camera, needs a look target
		if (!m_pTarget)
		{
			Warning(
				"Camera zone with missing target at (%f, %f, %f).\n",
				GetLocalOrigin().x, GetLocalOrigin().y, GetLocalOrigin().z);
		}
		break;
	case ZONE_BRF_CASE: // Breifcase, needs a look target
		if (!m_pTarget)
		{
			Warning(
				"Breifcase zone with missing target at (%f, %f, %f).\n",
				GetLocalOrigin().x, GetLocalOrigin().y, GetLocalOrigin().z);
		}
		break;
		// case 8, Rescue Zone, No target
	default:
		break;
	}
}

//------------------------------------------------------------------------------
// Purpose: Input handler to turn off this trigger.
//------------------------------------------------------------------------------
void CSpecialZone::InputDisable(inputdata_t &inputdata)
{
	Disable(); // Overridden to force hudtext off when disabled
}

//------------------------------------------------------------------------------
// Purpose: Turn off this trigger.
//------------------------------------------------------------------------------
void CSpecialZone::Disable()
{
	// Get rid of that hud text
	if (Player)
	{
		EndTouch(Player);
	}
	BaseClass::Disable();
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity starts touching us.
// Input  : pOther - the entity that is touching us.
//-----------------------------------------------------------------------------
void CSpecialZone::StartTouch(CBaseEntity *pOther)
{
	if (FClassnameIs(pOther, "npc_hostage"))
	{
		if (m_nType == ZONE_RESCUE)
		{
			m_pHostage = dynamic_cast<CNPC_Hostage *>(pOther);
			if (m_pHostage)
			{
				m_pHostage->Rescue(this);
			}
			Success(this);
		}
		return;
	}

	// if it's not a player, ignore
	if (!pOther->IsPlayer())
		return;

	Player = dynamic_cast<CBasePlayer *>(pOther);
	if (Player)
	{
		m_bWereLooking = Looking();
		switch (m_nType)
		{
		case ZONE_STEALTH:
			Player->AddFlag(FL_NOTARGET);
			/*
			NPC's will still be able to determine your location
			when you do things that make noise, such as shooting or running.
			*/
			Player->SetPlayerInZoneStealth(true);
			m_iszHudtext = castable_string_t("InStealthZone");
			break;
		case ZONE_RC_BOMB:
			Player->SetPlayerInZoneRcBomb(m_bWereLooking);
			m_iszHudtext = castable_string_t("InRcBombZone");
			break;
		case ZONE_BLOWTORCH:
			Player->SetPlayerInZoneBlowtorch(true);
			m_iszHudtext = castable_string_t("InBlowtorchZone");
			break;
		case ZONE_FO_CAMERA:
			Player->SetPlayerInZoneFiberopticCamera(true);
			m_iszHudtext = castable_string_t("InFiberopticCameraZone");
			break;
		case ZONE_RADIO:
			Player->SetPlayerInZoneRadio(true);
			m_iszHudtext = castable_string_t("InRadioZone");
			break;
		case ZONE_DEFUSE:
			Player->SetPlayerInZoneDefuse(m_bWereLooking);
			m_iszHudtext = castable_string_t("InDefuseZone");
			break;
		case ZONE_CAMERA:
			Player->SetPlayerInZoneDigitalCamera(m_bWereLooking);
			m_iszHudtext = castable_string_t("InCameraZone");
			break;
		case ZONE_BRF_CASE:
			Player->SetPlayerInZoneBriefcase(m_bWereLooking);
			m_iszHudtext = castable_string_t("InBriefcaseZone");
			break;
		case ZONE_RESCUE:
			Player->SetPlayerInZoneRescue(true);
			m_iszHudtext = castable_string_t("InRescueZone");
			break;
		default:
			break;
		}

		// Only display the HUD message if the player is looking or this zone type dosn't care (also, don't show while using fiberoptic camera)
		if (m_bWereLooking || (
			(m_nType == ZONE_STEALTH) || (m_nType == ZONE_BLOWTORCH) || (m_nType == ZONE_FO_CAMERA && !m_bUsing) || (m_nType == ZONE_RADIO) || (m_nType == ZONE_RESCUE)))
		{
			UTIL_ShowMessage(STRING(m_iszHudtext), ToBasePlayer(Player));
		}
	}
	BaseClass::StartTouch(pOther);
}

//-----------------------------------------------------------------------------
// Purpose: Handle a touch from another entity (Runs constantly!)
//-----------------------------------------------------------------------------
void CSpecialZone::Touch(CBaseEntity *pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
		return;

	if (Player)
	{
		switch (m_nType)
		{
			// case 0 (Stealth) is FOV-agnostic
		case ZONE_RC_BOMB:
			Player->SetPlayerInZoneRcBomb(Looking());
			m_iszHudtext = castable_string_t("InRcBombZone");
			m_iszUnHudtext = castable_string_t("OutRcBombZone");
			break;
			// cases 2-4 (Blowtorch, Fiboptic Camera, and Radio) are FOV-agnostic
		case ZONE_DEFUSE:
			Player->SetPlayerInZoneDefuse(Looking());
			m_iszHudtext = castable_string_t("InDefuseZone");
			m_iszUnHudtext = castable_string_t("OutDefuseZone");
			break;
		case ZONE_CAMERA:
			Player->SetPlayerInZoneDigitalCamera(Looking());
			m_iszHudtext = castable_string_t("InCameraZone");
			m_iszUnHudtext = castable_string_t("OutCameraZone");
			break;
		case ZONE_BRF_CASE:
			Player->SetPlayerInZoneBriefcase(Looking());
			m_iszHudtext = castable_string_t("InBriefcaseZone");
			m_iszUnHudtext = castable_string_t("OutBriefcaseZone");
			break;
			// case 8 (Rescue) is FOV-agnostic
		default:
			break;
		}

		if (m_bWereLooking != Looking())
		{
			// m_iszHudtext was set for every zone type by StartTouch(), so don't display it (again) for FOV-agnostic zones!
			if (Looking() && (
				(m_nType == 1) || (m_nType == 5) || (m_nType == 6) || (m_nType == 7)))
			{
				UTIL_ShowMessage(STRING(m_iszHudtext), ToBasePlayer(Player));
			}
			else
			{
				UTIL_ShowMessage(STRING(m_iszUnHudtext), ToBasePlayer(Player));
			}
		}
		m_bWereLooking = Looking();
	}

	BaseClass::Touch(pOther);
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity stops touching us.
// Input  : pOther - the entity that was touching us.
//-----------------------------------------------------------------------------
void CSpecialZone::EndTouch(CBaseEntity *pOther)
{
	/* Ignore if it's not a player. */
	if (!pOther->IsPlayer())
	{
		return;
	}

	Player = dynamic_cast < CBasePlayer* > (pOther);
	if (Player)
	{
		switch (m_nType)
		{
		case ZONE_STEALTH:
			Player->SetPlayerInZoneStealth(false);
			m_iszUnHudtext = castable_string_t("OutStealthZone");
			Player->RemoveFlag(FL_NOTARGET);
			break;
		case ZONE_RC_BOMB:
			Player->SetPlayerInZoneRcBomb(false);
			m_iszUnHudtext = castable_string_t("OutRcBombZone");
			break;
		case ZONE_BLOWTORCH:
			Player->SetPlayerInZoneBlowtorch(false);
			m_iszUnHudtext = castable_string_t("OutBlowtorchZone");
			break;
		case ZONE_FO_CAMERA:
			Player->SetPlayerInZoneFiberopticCamera(false);
			m_iszUnHudtext = castable_string_t("OutFiberopticCameraZone");
			break;
		case ZONE_RADIO:
			Player->SetPlayerInZoneRadio(false);
			m_iszUnHudtext = castable_string_t("OutRadioZone");
			break;
		case ZONE_DEFUSE:
			Player->SetPlayerInZoneDefuse(false);
			m_iszUnHudtext = castable_string_t("OutDefuseZone");
			break;
		case ZONE_CAMERA:
			Player->SetPlayerInZoneDigitalCamera(false);
			m_iszUnHudtext = castable_string_t("OutCameraZone");
			break;
		case ZONE_BRF_CASE:
			Player->SetPlayerInZoneBriefcase(false);
			m_iszUnHudtext = castable_string_t("OutBriefcaseZone");
			break;
		case ZONE_RESCUE:
			Player->SetPlayerInZoneRescue(false);
			m_iszUnHudtext = castable_string_t("OutRescueZone");
			break;
		default:
			break;
		}
		UTIL_ShowMessage(STRING(m_iszUnHudtext), ToBasePlayer(Player));
	}
	BaseClass::EndTouch(pOther);
}

//-----------------------------------------------------------------------------
// Purpose: Toggles defusing, blowtorching, ...
//-----------------------------------------------------------------------------
void CSpecialZone::ToggleUse(CBasePlayer *pPlayer)
{
	if (m_bUsing)
	{
		StopUsing(pPlayer);
	}
	else
	{
		StartUsing(pPlayer);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Player has begun defusing, blowtorching, ...
//-----------------------------------------------------------------------------
void CSpecialZone::StartUsing(CBasePlayer *pPlayer)
{
	OnStartUsing.FireOutput(pPlayer, this);
	m_bUsing = true;

	if (m_nType == ZONE_RC_BOMB && Looking()) // RC Bomb
	{
		m_pTarget->SetRenderMode(kRenderNormal);
		m_pTarget->m_nRenderFX = kRenderFxMax;
		m_pTarget->SetRenderColor(255, 255, 255, 255);
		// Leaving the bomb non-solid is important in case something like an info_target is used
		//m_pTarget->SetSolid( SOLID_BBOX );
	}

	if (m_nType == ZONE_FO_CAMERA && m_pViewport) // Fiberoptic Camera zone and Viewport is valid
	{
		if (m_pOverlay)
		{
			m_pOverlay->InputStartOverlay(inputdata_t());
		}
		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_ALL;
		pPlayer->AddFlag(FL_ATCONTROLS);
		m_pViewport->Enable();
	}

	if (m_nType == ZONE_RADIO) // Radio
	{
		StopUsing(pPlayer);
	}

	if ((m_nType == ZONE_DEFUSE) && m_flDefuseTime) // Defuse zone; you DID set a defusal time, right?
	{
		if (!m_DefuseTimer.HasStarted())
		{
			m_DefuseTimer.Start();
		}
		else
		{
			m_DefuseTimer.Reset();
		}
		SetThink(&CSpecialZone::Think);
		SetNextThink(gpGlobals->curtime + 0.125f);
	}

	if (m_nType == ZONE_BRF_CASE && Looking()) // Briefcase
	{
		StopUsing(pPlayer);
	}
}

//-----------------------------------------------------------------------------
// Purpose: think
//-----------------------------------------------------------------------------
void CSpecialZone::Think(void)
{
	if (m_bUsing)
	{
		if (m_nType == ZONE_DEFUSE)
		{
			m_flDefuseProgress = m_DefuseTimer.GetElapsedTime() / m_flDefuseTime;
			Player->SetDefuseProgress(m_flDefuseProgress);
			if (m_DefuseTimer.GetElapsedTime() >= m_flDefuseTime)
			{
				m_pBomb->Deactivate();
				m_flDefuseProgress = 0;
				Player->SetDefuseProgress(m_flDefuseProgress);
			}
			SetNextThink(gpGlobals->curtime + 0.125f);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Determine where the player is looking
//-----------------------------------------------------------------------------
bool CSpecialZone::Looking(void)
{
	if (!m_pTarget)
	{
		return false;
	}

	Vector vLookDir = Player->EyeDirection3D();
	Vector vTargetDir = m_pTarget->GetAbsOrigin() - Player->EyePosition();

	VectorNormalize(vTargetDir);

	float fDotPr = DotProduct(vLookDir, vTargetDir);

	return (fDotPr > m_flFieldOfView);
}

//-----------------------------------------------------------------------------
// Purpose: Player quit defusing, blowtorching, ...
//-----------------------------------------------------------------------------
void CSpecialZone::StopUsing(CBasePlayer *pPlayer)
{
	OnStopUsing.FireOutput(pPlayer, this);
	m_bUsing = false;

	if (m_nType == ZONE_BLOWTORCH && m_pTarget) // Blowtorch Zone & we have target
		if (!m_pTarget->GetHealth()) // Target is dead
		{
			Success(pPlayer);
		}

	if (m_nType == ZONE_FO_CAMERA) // Fiberoptic Camera
	{
		if (m_pOverlay)
		{
			m_pOverlay->InputStopOverlay(inputdata_t());
		}
		pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_ALL;
		pPlayer->RemoveFlag(FL_ATCONTROLS);
		m_pViewport->Disable();
		Success(pPlayer);
	}

	if (m_nType == ZONE_RADIO) // Radio
	{
		Success(pPlayer);
	}

	if (m_nType == ZONE_DEFUSE) // Defuse
	{
		if (m_DefuseTimer.GetElapsedTime() >= m_flDefuseTime)
		{
			Success(pPlayer);
		}
		else
		{
			Aborted(pPlayer);
			m_flDefuseProgress = 0;
			pPlayer->SetDefuseProgress(m_flDefuseProgress);
		}
	}

	if (m_nType == ZONE_CAMERA && Looking()) // Digital Camera & looking within FOV of Target
	{
		Success(pPlayer);
	}

	if (m_nType == ZONE_BRF_CASE && Looking()) // Briefcase & looking within FOV of Target
	{
		Success(pPlayer);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Player let go too soon
//-----------------------------------------------------------------------------
void CSpecialZone::Aborted(CBasePlayer *pPlayer)
{
	OnAborted.FireOutput(pPlayer, this);
}

//-----------------------------------------------------------------------------
// Purpose: weapon fired, bomb defused, Hostage rescued, ...
//-----------------------------------------------------------------------------
void CSpecialZone::Success(CBaseEntity *pActivator)
{
	if (m_nType == ZONE_RC_BOMB) // RC Bomb
	{
		ExplosionCreate(m_pTarget->GetLocalOrigin(), GetAbsAngles(), this, 1024, 128, true);
		m_pTarget->Remove();
	}
	OnSuccess.FireOutput(pActivator, this);
}
