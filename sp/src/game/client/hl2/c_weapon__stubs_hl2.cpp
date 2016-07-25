//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

STUB_WEAPON_CLASS( cycler_weapon, WeaponCycler, C_BaseCombatWeapon );

STUB_WEAPON_CLASS( weapon_binoculars, WeaponBinoculars, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_bugbait, WeaponBugBait, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_flaregun, Flaregun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_annabelle, WeaponAnnabelle, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_gauss, WeaponGaussGun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_cubemap, WeaponCubemap, C_BaseCombatWeapon );
STUB_WEAPON_CLASS( weapon_alyxgun, WeaponAlyxGun, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_citizenpackage, WeaponCitizenPackage, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_citizensuitcase, WeaponCitizenSuitcase, C_WeaponCitizenPackage );

#ifndef HL2MP
STUB_WEAPON_CLASS( weapon_ar2, WeaponAR2, C_HLMachineGun );
STUB_WEAPON_CLASS( weapon_frag, WeaponFrag, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_rpg, WeaponRPG, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_pistol, WeaponPistol, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_shotgun, WeaponShotgun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_smg1, WeaponSMG1, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_357, Weapon357, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_crossbow, WeaponCrossbow, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_slam, Weapon_SLAM, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_crowbar, WeaponCrowbar, C_BaseHLBludgeonWeapon );
#ifdef HL2_EPISODIC
STUB_WEAPON_CLASS( weapon_hopwire, WeaponHopwire, C_BaseHLCombatWeapon );
//STUB_WEAPON_CLASS( weapon_proto1, WeaponProto1, C_BaseHLCombatWeapon );
#endif

STUB_WEAPON_CLASS(weapon_blowtorch, WeaponBlowtorch, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_rcbomb, WeaponRcBomb, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_fiberopticcamera, WeaponFiberopticCamera, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_radio, WeaponRadio, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_camera, WeaponCamera, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_briefcase, WeaponBriefcase, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_taser, WeaponTaser, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_boomvest, WeaponBoomvest, C_BaseHLCombatWeapon);

STUB_WEAPON_CLASS(weapon_ak47, WeaponAK47, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_awp, WeaponAWP, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_aug, WeaponAug, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_bizon, WeaponBizon, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_deagle, WeaponDEagle, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_decoy, WeaponDecoy, C_WeaponMultiGrenade);
STUB_WEAPON_CLASS(weapon_elite, WeaponElite, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_famas, WeaponFamas, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_fiveseven, WeaponFiveSeven, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_flashbang, WeaponFlashBang, C_WeaponMultiGrenade);
STUB_WEAPON_CLASS(weapon_g3sg1, WeaponG3SG1, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_galilar, WeaponGalilAR, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_glock, WeaponGlock, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_hegrenade, WeaponHeGrenade, C_WeaponMultiGrenade);
STUB_WEAPON_CLASS(weapon_hkp2000, WeaponHKP2000, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_incgrenade, WeaponIncGrenade, C_WeaponMultiGrenade);
STUB_WEAPON_CLASS(weapon_knife, WeaponKnife, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_law, WeaponLAW, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_m4a4, WeaponM4A4, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_m60, WeaponM60, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_m249, WeaponM249, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_mac10, WeaponMac10, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_mag7, WeaponMag7, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_molotov, WeaponMolotov, C_WeaponMultiGrenade);
STUB_WEAPON_CLASS(weapon_mp7, WeaponMp7, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_mp9, WeaponMp9, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_negev, WeaponNegev, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_nova, WeaponNova, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_p90, WeaponP90, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_p250, WeaponP250, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_sawedoff, WeaponSawedOff, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_scar20, WeaponScar20, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_sg556, WeaponSG556, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_ssg08, WeaponSSG08, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_smokegrenade, WeaponSmokeGrenade, C_WeaponMultiGrenade);
STUB_WEAPON_CLASS(weapon_tec9, WeaponTec9, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_ump45, WeaponUMP45, C_BaseAdvancedWeapon);
STUB_WEAPON_CLASS(weapon_xm1014, WeaponXM1014, C_BaseAdvancedWeapon);

#endif


