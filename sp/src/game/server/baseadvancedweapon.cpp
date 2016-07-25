//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Base class for cs-like weapons
//
//=====================================================================
#include "cbase.h"
#include "baseadvancedweapon.h"
#include "in_buttons.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST(CBaseAdvancedWeapon, DT_BaseAdvancedWeapon)
END_SEND_TABLE()

#define ACCURACY_BLEND_SPEED 0.0625f // How often (in seconds) to bleed off inaccuracy from moving, shooting, etc.

//-----------------------------------------------------------------------------
// CBaseAdvancedWeapon
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CBaseAdvancedWeapon::CBaseAdvancedWeapon(void)
{
	m_bHasLight = m_bHasModes =	m_bModeSpam = m_bPullingBolt = false;
	m_bHasScope = GetZoomLevels() > 0;
	m_fMinRange1 = 0.0f;
	m_flNextAccuracyTick = gpGlobals->curtime;
	m_bFiresUnderwater = m_bAltFiresUnderwater = true; // Most CS weapons can fire underwater
}

//-----------------------------------------------------------------------------
// Purpose: When the player gets the gun
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::Equip(CBaseCombatCharacter *pOwner)
{
	BaseClass::Equip(pOwner);

	pPlayer = dynamic_cast < CHL2_Player* > (pOwner);

	ResetBurstCount();
	ChangeAccuracy();
}

//-----------------------------------------------------------------------------
// Purpose: Only two guns actually have a burst fire mode...
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::SetBurstMode(bool mode)
{
	m_bBurstFire = mode;
}

//-----------------------------------------------------------------------------
// Purpose: Reset the burst counter
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::ResetBurstCount(void)
{
	m_iBurstSize = FullAuto() ? GetMaxClip1() : GetMinBurst();
	m_iBurst = m_iBurstSize;
	m_bInBurst = false;
}

//-----------------------------------------------------------------------------
// Purpose: Lower the burst counter
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::DecreaseBurstCount(void)
{
	m_iBurst--;
}

//-----------------------------------------------------------------------------
// Purpose: The gun is empty, play a clicking noise with a dryfire anim
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::DryFire(void)
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: This happens if you click (and hold) the primary fire button
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::PrimaryAttack(void)
{
	// Do we have any bullets left from the current burst cycle?
	if (m_iBurst)
	{
		m_bInBurst = true;

		if (!pPlayer)
		{
			return;
		}

		WeaponSound(SINGLE);
		pPlayer->DoMuzzleFlash();
		SendWeaponAnim(ACT_VM_PRIMARYATTACK);
		pPlayer->SetAnimation(PLAYER_ATTACK1);

		BaseClass::PrimaryAttack();

		// We fired one shot, decrease the number of bullets available for this burst cycle
		DecreaseBurstCount();

		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
		m_flAttackEnds = gpGlobals->curtime + SequenceDuration();
	}
	// Does the accuracy need to shift?
	ChangeAccuracy();

	if (!FullAuto() && m_nZoomLevel.Get()) // BUGBUG: What about a semi-auto rifle?
	{
		int oldZoomLevel = m_nZoomLevel.Get();
		SetZoomLevel();
		// HACKHACK: Set the set level var but do not change it yet
		m_nZoomLevel.Set(oldZoomLevel);
		m_bPullingBolt = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override CBaseHLCombatWeapon's magic holster reloads
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::ItemHolsterFrame(void)
{
	CBaseCombatWeapon::ItemHolsterFrame();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::ItemPreFrame(void)
{
	BaseClass::ItemPreFrame();

	// Does the accuracy need to shift?
	ChangeAccuracy();

	if (CBasePlayer *pPlayer = ToBasePlayer(GetOwner()))
	{
		if (!(pPlayer->m_nButtons & IN_ATTACK2))
		{
			m_bModeSpam = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as button is pressed
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	/* If the fire rate is permitting another attack, and there are still rounds in the clip, and burst-fire mode is on, and
	the burst count has not been met, and we are in a burst cycle, then we are not done attacking; go back to that. */
	if ((m_flNextPrimaryAttack <= gpGlobals->curtime) && m_iClip1 && GetBurstMode() && m_iBurst && m_bInBurst)
	{
		PrimaryAttack();
		return;
	}


	if (m_bInReload)
	{
		return;
	}

	if (pPlayer == NULL)
	{
		return;
	}

	if ((pPlayer->m_nButtons & IN_ATTACK) || m_bInBurst) // If the player is holding +ATTACK, or the weapon is doing it for them
	{
		if ((m_flNextPrimaryAttack <= gpGlobals->curtime) && (m_iClip1 <= 0) && m_iBurst)
		{
			DryFire();
			return;
		}
		if (m_flAttackEnds < gpGlobals->curtime)
		{
			SendWeaponAnim(ACT_VM_IDLE);
		}
	}

	if (!GetBurstMode() || (m_iBurst <= m_iBurstSize - GetMinBurst())) // If burst mode is off or the minimum number of rounds has been fired
	{
		m_bInBurst = false;
	}

	if ((!(pPlayer->m_nButtons & IN_ATTACK)) && !m_bInBurst) // Make sure the player let go of +ATTACK and we finished the burst
	{
		ResetBurstCount(); // The burst counter needs to be reset
	}

	// Does the accuracy need to shift?
	ChangeAccuracy();

	if (m_bPullingBolt)
	{
		if (IsViewModelSequenceFinished())
		{
			m_bPullingBolt = false;
			// HACKHACK: This is how we stored this when we pulled back the bolt
			SetZoomLevel(m_nZoomLevel.Get());
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: If we have bullets left then play the attack anim, otherwise idle
//-----------------------------------------------------------------------------
Activity CBaseAdvancedWeapon::GetPrimaryAttackActivity(void)
{
	if (m_iBurst)
	{
		return ACT_VM_PRIMARYATTACK;
	}
	else
	{
		return ACT_VM_IDLE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Drop the gun
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::Drop(const Vector &vecVelocity)
{
	SetZoomLevel(); // Unzoom if needed
	BaseClass::Drop(vecVelocity);
}

//-----------------------------------------------------------------------------
// Purpose: The gun is being reloaded
//-----------------------------------------------------------------------------
bool CBaseAdvancedWeapon::Reload(void)
{
	SetZoomLevel();
	if (DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD))
	{
		WeaponSound(RELOAD);
		ResetBurstCount(); // Reset the burst counter
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Calculate the viewkick
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::AddViewKick(void)
{
	if (pPlayer == NULL)
	{
		return;
	}

	RandomSeed(CBaseEntity::GetPredictionRandomSeed() & 65536);

	QAngle viewPunch;

	viewPunch.x = random->RandomFloat(GetRecoilMagnitude() - GetRecoilMagnitudeVariance(), GetRecoilMagnitude() + GetRecoilMagnitudeVariance());
	viewPunch.y = random->RandomFloat(GetRecoilAngle() - GetRecoilAngleVariance(), GetRecoilAngle() + GetRecoilAngleVariance());
	viewPunch.z = 0.0f;

	// Add it to the view punch
	pPlayer->ViewPunch(viewPunch);
}

//-----------------------------------------------------------------------------
// Purpose: Get the stance-only portion of accuracy
//-----------------------------------------------------------------------------
const Vector CBaseAdvancedWeapon::GetBaseAccuracy(void)
{
	float fBigAcc = GetInaccuracyStand(); // From the script, these are stored as RADIANS (i.e. "9.16")

										  // Check the character's current stance
	if (pPlayer == NULL)
	{
		return Vector(sin(DEG2RAD(fBigAcc / 2.0f)));
	}

	// Movement-based stances
	if (pPlayer->IsDucking())
	{
		fBigAcc = GetInaccuracyCrouch();
	}
	else // You can't run or sprint while crouching...
	{
		// SPRINTING (+speed) and WALKING (+walk) are NOT RUNNING (+forwad, etc., ...); there can only be one!
		if (pPlayer->IsSprinting())
		{
			fBigAcc = GetInaccuracyMove() * 2; // Sprint is worse than move (run) for now
		}
		else
		{
			if (pPlayer->IsWalking())
			{
				fBigAcc = GetInaccuracyStand(); // Leaving walk at stand for now
			}
			else
			{
				if (pPlayer->m_flForwardMove || pPlayer->m_flSideMove)
				{
					fBigAcc = GetInaccuracyMove();
				}
			}
		}
	}
	if (!pPlayer->GetGroundEntity())
	{
		fBigAcc += GetInaccuracyJump(); // This is a very small value in the script
	}
	if (pPlayer->IsOnLadder())
	{
		fBigAcc = GetInaccuracyLadder();
	}

	// Valve calculates accuracy cones with sin(degrees/2); sin expects radians...
	return Vector(sin(DEG2RAD(fBigAcc / 2.0f)));
}

//-----------------------------------------------------------------------------
// Purpose: Slowly blend our accuracy
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::ChangeAccuracy(void)
{
	// Slow down, son!
	if (m_flNextAccuracyTick < gpGlobals->curtime)
	{
		Vector accuracy = GetBaseAccuracy(); // Accuracy to blend to as soon as recoil is checked
		accuracy = accuracy / 2; // BUGBUG:? There's no way that the accuracy is THAT BAD!

								 // Add recoil
		for (int i = 0; i < (m_iBurstSize - m_iBurst); i++)
		{
			accuracy += accuracy * (GetInaccuracyFire() / 100); // Large values in the scripts, are they supposed to make you % less accurate?
		}

		// Vector doesn't support < or > so Vector.x is compared
		if (m_vCone.m_Value.x > accuracy.x)
		{
			m_vCone = Vector(abs(m_vCone.m_Value.x - (VECTOR_CONE_1DEGREES + (VECTOR_CONE_1DEGREES * (m_vCone.m_Value - accuracy))).x));
		}
		if (m_vCone.m_Value.x < accuracy.x)
		{
			// Accuracy losses should be immediate
			m_vCone = accuracy;
		}
		m_flNextAccuracyTick = gpGlobals->curtime + ACCURACY_BLEND_SPEED;
	}
}

//-----------------------------------------------------------------------------
// Weapon Special Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Alt Fire
//-----------------------------------------------------------------------------
void CBaseAdvancedWeapon::SecondaryAttack()
{
	if (!m_bModeSpam)
	{
		m_bModeSpam = true;
		if (m_bHasModes)
		{
			SetBurstMode(!GetBurstMode());
			ResetBurstCount();
			castable_string_t iSzText = "BurstModeClearS";
			if (CBasePlayer* pOwner = ToBasePlayer(GetOwner()))
			{
				if (GetBurstMode())
				{
					UTIL_ShowMessage(STRING(castable_string_t("BurstModeClearF")), pOwner);
					UTIL_ShowMessage(STRING(castable_string_t("BurstModeClearS")), pOwner);
					iSzText = "BurstModeOn";
				}
				else
				{
					if (FullAuto())
					{
						iSzText = "BurstModeFA";
					}
					else
					{
						iSzText = "BurstModeSA";
					}
				}
				UTIL_ShowMessage(STRING(castable_string_t(iSzText)), pOwner);
			}
		}
		if (m_bHasLight)
		{
			if (pPlayer->FlashlightIsOn())
			{
				pPlayer->FlashlightTurnOff();
			}
			else
			{
				pPlayer->FlashlightTurnOn();
			}
		}
		if (m_bHasScope)
			SetZoomLevel(m_nZoomLevel.Get() + 1);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Reset the weapon when holstered
//-----------------------------------------------------------------------------
bool CBaseAdvancedWeapon::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if (BaseClass::Holster(pSwitchingTo))
	{
		if (pPlayer)
		{
			if (m_bHasModes)
			{
				UTIL_ShowMessage(STRING(castable_string_t("BurstModeClearF")), pPlayer);
				UTIL_ShowMessage(STRING(castable_string_t("BurstModeClearS")), pPlayer);
				SetBurstMode(false);
			}
			if (m_bHasLight)
				pPlayer->FlashlightTurnOff();
			if (m_bHasScope)
				SetZoomLevel();
			m_bModeSpam = false;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Would it be okay for the game to display a hint about our alt-fire?
//-----------------------------------------------------------------------------
bool CBaseAdvancedWeapon::ShouldDisplayAltFireHUDHint()
{
	bool retval = BaseClass::ShouldDisplayAltFireHUDHint();
	if (pPlayer)

		if (m_bHasLight && pPlayer->FlashlightIsOn())
			retval = false;
		if (m_bHasModes && GetBurstMode())
			retval = false;
		if (m_bHasScope && IsWeaponZoomed())
			retval = false;

	return retval;
}

//-----------------------------------------------------------------------------
// Purpose: Zooms in and out
//-----------------------------------------------------------------------------
bool CBaseAdvancedWeapon::SetZoomLevel(int level)
{
	if (!pPlayer)
	{
		return false;
	}

	if (level > GetZoomLevels())
		level = 0;

	CSingleUserRecipientFilter scope_filter(pPlayer);
	if (HideViewModelWhenZoomed())
		UserMessageBegin(scope_filter, "ShowScope");

	if (level > 0)
	{
		if (HideViewModelWhenZoomed())
			WRITE_BYTE(1);
		CPASAttenuationFilter zoomIn_filter(pPlayer, GetZoomInSound());
		EmitSound(zoomIn_filter, pPlayer->entindex(), GetZoomInSound());
		if (!pPlayer->SetFOV(this, GetZoomFov(level), GetZoomTime(level)))
			DevWarning("Failed to zoom!\n", level, GetZoomFov(level));
	}
	else
	{
		CPASAttenuationFilter zoomOut_filter(pPlayer, GetZoomOutSound());
		EmitSound(zoomOut_filter, pPlayer->entindex(), GetZoomOutSound());
		if (!pPlayer->SetFOV(this, 0, GetZoomTime(0)))
			DevWarning("Failed to unzoom!\n", level, GetZoomFov(level));
		if (HideViewModelWhenZoomed())
			WRITE_BYTE(0);
	}
	m_nZoomLevel.Set(level);

	if (HideViewModelWhenZoomed())
		MessageEnd(); // "ShowScope"

	pPlayer->ShowViewModel(!(m_nZoomLevel.Get() && HideViewModelWhenZoomed()));

	return true;
}
