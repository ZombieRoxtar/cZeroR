//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Hostages
//
//=============================================================================
#ifndef	NPC_HOSTAGE_H
#define	NPC_HOSTAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "npc_playercompanion.h"
#include "trigger_special_zone.h"

struct SquadCandidate_t;

class CSpecialZone;

//-----------------------------------------------------------------------------
//
// CLASS: CNPC_Hostage
//
//-----------------------------------------------------------------------------

//-------------------------------------
// Spawnflags
//-------------------------------------
#define SF_HOSTAGE_FOLLOW				( 1 << 16 )	//65536, follow the player as soon as I spawn.
#define SF_HOSTAGE_RANDOM_HEAD			( 1 << 17 )	//131072
#define SF_HOSTAGE_RANDOM_HEAD_MALE		( 1 << 18 )	//262144
#define SF_HOSTAGE_RANDOM_HEAD_FEMALE	( 1 << 19 ) //524288
#define SF_HOSTAGE_CUSTOM_RESCUE		( 1 << 20 ) //1048576, Do NOT fade out on rescue, the mapper has a scripted sequence or something.

//-------------------------------------
// Models - These will be replaced with races
//-------------------------------------

enum HostageType_t
{
	CT_DEFAULT,
	CT_DOWNTRODDEN,
	CT_REFUGEE,
	CT_REBEL,
	CT_UNIQUE
};

//-----------------------------------------------------------------------------
// Hostage expression types
//-----------------------------------------------------------------------------
enum HostageExpressionTypes_t
{
	CIT_EXP_UNASSIGNED,	// Defaults to this, selects other in spawn.

	CIT_EXP_SCARED,
	CIT_EXP_NORMAL,
	CIT_EXP_ANGRY,

	CIT_EXP_LAST_TYPE,
};

//-------------------------------------

class CNPC_Hostage : public CNPC_PlayerCompanion
{
	DECLARE_CLASS(CNPC_Hostage, CNPC_PlayerCompanion);
public:
	CNPC_Hostage()
		: m_iHead(-1)
	{
	}

	//---------------------------------
	void			Precache();
	void			PrecacheAllOfType(HostageType_t);
	void			Spawn();
	void			PostNPCInit();
	virtual void	SelectModel();
	void			SelectExpressionType();
	bool			ShouldAutosquad();

	virtual float	GetJumpGravity() const { return 1.8f; }

	void			OnRestore();

	//---------------------------------
	Class_T 		Classify();

	bool 			ShouldAlwaysThink();

	//---------------------------------
	// Behavior
	//---------------------------------
	void 			GatherConditions();
	void 			PrescheduleThink();

	int 			TranslateSchedule(int scheduleType);

	bool			ShouldAcceptGoal(CAI_BehaviorBase *pBehavior, CAI_GoalEntity *pGoal);
	void			OnClearGoal(CAI_BehaviorBase *pBehavior, CAI_GoalEntity *pGoal);

	void 			StartTask(const Task_t *pTask);
	void 			RunTask(const Task_t *pTask);

	void 			HandleAnimEvent(animevent_t *pEvent);
	void			TaskFail(AI_TaskFailureCode_t code);

	virtual const char *SelectRandomExpressionForState(NPC_STATE state);

	void			Rescue(CSpecialZone *zone);

	//---------------------------------
	// Combat
	//---------------------------------	
	virtual bool	UseAttackSquadSlots() { return false; }

	//---------------------------------
	// Commander mode
	//---------------------------------
	bool			CanJoinPlayerSquad();
	bool			WasInPlayerSquad();
	bool			HaveCommandGoal() const;
	bool 			NearCommandGoal();
	bool 			VeryFarFromCommandGoal();
	void 			CommanderUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	bool			ShouldSpeakRadio(CBaseEntity *pListener);
	void			AddToPlayerSquad();
	void			RemoveFromPlayerSquad();
	void			UpdatePlayerSquad();
	void 			FixupPlayerSquad();
	void 			ClearFollowTarget();
	bool			SpeakCommandResponse(AIConcept_t concept, const char *modifiers = NULL);

	//---------------------------------
	// Hints
	//---------------------------------
	bool			FValidateHintType(CAI_Hint *pHint);

	//---------------------------------
	// Inputs
	//---------------------------------
	void			InputSpeakIdleResponse(inputdata_t &inputdata);

	//---------------------------------
	//	Sounds & speech
	//---------------------------------
	void			FearSound(void);
	void			DeathSound(const CTakeDamageInfo &info);

private:
	//-----------------------------------------------------
	// Conditions, Schedules, Tasks
	//-----------------------------------------------------
	enum
	{
		SCHED_HOSTAGE_MOURN_PLAYER = BaseClass::NEXT_SCHEDULE,
		SCHED_HOSTAGE_RESCUE_WAIT,
		TASK_HOSTAGE_SPEAK_MOURNING = BaseClass::NEXT_TASK,
		TASK_HOSTAGE_RESCUE_SPEAK,
		TASK_HOSTAGE_RESCUE_WAIT
	};

	//-----------------------------------------------------
	float			m_flNextFearSoundTime;
	string_t		m_iszOriginalSquad;
	float			m_flTimeJoinedPlayerSquad;
	bool			m_bWasInPlayerSquad;
	float			m_flTimeLastCloseToPlayer;
	string_t		m_iszDenyCommandConcept;

	HostageType_t	m_Type;
	HostageExpressionTypes_t	m_ExpressionType;

	int				m_iHead;

	bool			m_bShouldBeFollowing;
	bool			m_bWaitingInRescue; // In Rescue Zone, just waiting to be handled

	float m_flFadeOutStartTime;

	static CSimpleSimTimer gm_PlayerSquadEvaluateTimer;

	// The game time at which the player started staring at me.
	float			m_flTimePlayerStare;

	//-----------------------------------------------------
	//	Outputs
	//-----------------------------------------------------
	COutputEvent		m_OnPlayerUse;
	COutputEvent		m_OnNavFailBlocked;
	COutputEvent		m_OnRescued;

	//-----------------------------------------------------
	CHandle<CAI_FollowGoal>	m_hSavedFollowGoalEnt;

	bool					m_bNotifyNavFailBlocked;

	// Don't leave the player squad unless killed, or removed via Entity I/O. 
	bool					m_bNeverLeavePlayerSquad;
	//-----------------------------------------------------

	DECLARE_DATADESC();
#ifdef _XBOX
protected:
#endif
	DEFINE_CUSTOM_AI;
};

#endif //NPC_HOSTAGE_H
