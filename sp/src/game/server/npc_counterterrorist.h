//========== Copyright Bit Mage's Stuff, All rights probably reserved. ==========//
//
// Purpose: Your everyday, multi-purpose CounterTerrorist
//
//===============================================================================//
#ifndef	NPC_COUNTERTERRORIST_H
#define	NPC_COUNTERTERRORIST_H

#include "npc_playercompanion.h"
#include "ai_behavior_functank.h"

struct SquadCandidate_t;

//-----------------------------------------------------------------------------
//
// CLASS: CNPC_CounterTerrorist
//
//-----------------------------------------------------------------------------

//-------------------------------------
// Spawnflags
//-------------------------------------
#define SF_CITIZEN_FOLLOW				( 1 << 16 )	//65536 Follow the player as soon as I spawn.
#define SF_CITIZEN_RANDOM_HEAD			( 1 << 17 )	//262144
//#define SF_CITIZEN_AMMORESUPPLIER		( 1 << 18 )	//524288
#define SF_CITIZEN_NOT_COMMANDABLE		( 1 << 19 ) //1048576
#define SF_CITIZEN_IGNORE_SEMAPHORE		( 1 << 20 ) //2097152 Work outside the speech semaphore system  // Obsolete?
#define SF_CITIZEN_RANDOM_HEAD_MALE		( 1 << 21 )	//4194304
#define SF_CITIZEN_RANDOM_HEAD_FEMALE	( 1 << 22 ) //8388608
#define SF_CITIZEN_USE_RENDER_BOUNDS	( 1 << 23 ) //16777216

//-------------------------------------
// Animation events
//-------------------------------------

enum CitizenType_t
{
	CT_DEFAULT,
	CT_DOWNTRODDEN,
	CT_REFUGEE,
	CT_REBEL,
	CT_UNIQUE
};

//-----------------------------------------------------------------------------
// Citizen expression types
//-----------------------------------------------------------------------------
enum CitizenExpressionTypes_t
{
	CT_EXP_UNASSIGNED,	// Defaults to this, selects other in spawn.
	CT_EXP_SCARED,
	CT_EXP_NORMAL,
	CT_EXP_ANGRY,
	CT_EXP_LAST_TYPE,
};

//-------------------------------------

class CNPC_CounterTerrorist : public CNPC_PlayerCompanion
{
	DECLARE_CLASS(CNPC_CounterTerrorist, CNPC_PlayerCompanion);
public:
	CNPC_CounterTerrorist()
		: m_iHead(-1)
	{
	}

	//---------------------------------
	bool			CreateBehaviors();
	void			Precache();
	void			PrecacheAllOfType(CitizenType_t);
	void			Spawn();
	void			PostNPCInit();
	virtual void	SelectModel();
	void			SelectExpressionType();

	virtual float	GetJumpGravity() const { return 1.8f; }

	void			OnRestore();

	//---------------------------------
	Class_T 		Classify();
	bool 			ShouldAlwaysThink();

	//---------------------------------
	// Behavior
	//---------------------------------
	bool			ShouldBehaviorSelectSchedule(CAI_BehaviorBase *pBehavior);
	void 			GatherConditions();
	void			PredictPlayerPush();
	void 			PrescheduleThink();
	void			BuildScheduleTestBits();

	int				SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode);

	int 			SelectSchedulePriorityAction();
	int 			SelectScheduleRetrieveItem();
	int 			SelectScheduleNonCombat();
	int 			TranslateSchedule(int scheduleType);

	bool			ShouldAcceptGoal(CAI_BehaviorBase *pBehavior, CAI_GoalEntity *pGoal);
	void			OnClearGoal(CAI_BehaviorBase *pBehavior, CAI_GoalEntity *pGoal);

	Activity		NPC_TranslateActivity(Activity eNewActivity);
	void 			HandleAnimEvent(animevent_t *pEvent);
	void			TaskFail(AI_TaskFailureCode_t code);

	void 			SimpleUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	bool			IgnorePlayerPushing(void);

	virtual const char *SelectRandomExpressionForState(NPC_STATE state);

	//---------------------------------
	// Combat
	//---------------------------------
	bool 			OnBeginMoveAndShoot();
	void 			OnEndMoveAndShoot();

	virtual bool	UseAttackSquadSlots() { return false; }

	void 			OnChangeActiveWeapon(CBaseCombatWeapon *pOldWeapon, CBaseCombatWeapon *pNewWeapon);

	bool			ShouldLookForBetterWeapon();

	//---------------------------------
	// Damage handling
	//---------------------------------
	int 			OnTakeDamage_Alive(const CTakeDamageInfo &info);

	//---------------------------------
	// Commander mode
	//---------------------------------
	bool 			IsCommandable();
	bool			CanJoinPlayerSquad();
	bool			WasInPlayerSquad();
	bool			HaveCommandGoal() const;
	bool			IsCommandMoving();
	bool 			IsValidCommandTarget(CBaseEntity *pTarget);
	bool 			NearCommandGoal();
	bool 			VeryFarFromCommandGoal();
	bool 			TargetOrder(CBaseEntity *pTarget, CAI_BaseNPC **Allies, int numAllies);
	void 			MoveOrder(const Vector &vecDest, CAI_BaseNPC **Allies, int numAllies);
	void			OnMoveOrder();
	void 			CommanderUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	bool			ShouldSpeakRadio(CBaseEntity *pListener);
	void			OnMoveToCommandGoalFailed();
	void			AddToPlayerSquad();
	void			RemoveFromPlayerSquad();
	void 			TogglePlayerSquadState();
	void			UpdatePlayerSquad();
	static int __cdecl PlayerSquadCandidateSortFunc(const SquadCandidate_t *, const SquadCandidate_t *);
	void 			FixupPlayerSquad();
	void 			ClearFollowTarget();
	void 			UpdateFollowCommandPoint();
	bool			IsFollowingCommandPoint();
	CAI_BaseNPC *	GetSquadCommandRepresentative();
	void			SetSquad(CAI_Squad *pSquad);
	bool			SpeakCommandResponse(AIConcept_t concept, const char *modifiers = NULL);

	//---------------------------------
	// Interaction
	//---------------------------------
	bool			HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);

	//---------------------------------
	// Hints
	//---------------------------------
	bool			FValidateHintType(CAI_Hint *pHint);

	//---------------------------------
	// Inputs
	//---------------------------------
	void			InputRemoveFromPlayerSquad(inputdata_t &inputdata) { RemoveFromPlayerSquad(); }
	void 			InputStartPatrolling(inputdata_t &inputdata);
	void 			InputStopPatrolling(inputdata_t &inputdata);
	void			InputSetCommandable(inputdata_t &inputdata);
	void			InputSpeakIdleResponse(inputdata_t &inputdata);

	//---------------------------------
	//	Sounds & speech
	//---------------------------------
	void			DeathSound(const CTakeDamageInfo &info);
	bool			UseSemaphore(void);

	virtual void	OnChangeRunningBehavior(CAI_BehaviorBase *pOldBehavior, CAI_BehaviorBase *pNewBehavior);

private:
	//-----------------------------------------------------
	// Conditions, Schedules, Tasks
	//-----------------------------------------------------
	enum
	{
		COND_CIT_HURTBYFIRE = BaseClass::NEXT_CONDITION,

		SCHED_CITIZEN_PATROL = BaseClass::NEXT_SCHEDULE,
		SCHED_CITIZEN_MOURN_PLAYER,

		TASK_CIT_SPEAK_MOURNING = BaseClass::NEXT_TASK,
	};

	//-----------------------------------------------------	
	float			m_flNextFearSoundTime;
	bool			m_bShouldPatrol;
	string_t		m_iszOriginalSquad;
	float			m_flTimeJoinedPlayerSquad;
	bool			m_bWasInPlayerSquad;
	float			m_flTimeLastCloseToPlayer;
	string_t		m_iszDenyCommandConcept;

	CSimpleSimTimer	m_AutoSummonTimer;
	Vector			m_vAutoSummonAnchor;

	CitizenType_t	m_Type;
	CitizenExpressionTypes_t	m_ExpressionType;

	int				m_iHead;

	static CSimpleSimTimer gm_PlayerSquadEvaluateTimer;

	float			m_flTimePlayerStare;	// The game time at which the player started staring at me.
	float			m_flTimeNextHealStare;	// Next time I'm allowed to heal a player who is staring at me.

											//-----------------------------------------------------
											//	Outputs
											//-----------------------------------------------------
	COutputEvent		m_OnJoinedPlayerSquad;
	COutputEvent		m_OnLeftPlayerSquad;
	COutputEvent		m_OnFollowOrder;
	COutputEvent		m_OnStationOrder;
	COutputEvent		m_OnPlayerUse;
	COutputEvent		m_OnNavFailBlocked;

	//-----------------------------------------------------
	CAI_FuncTankBehavior	m_FuncTankBehavior;

	CHandle<CAI_FollowGoal>	m_hSavedFollowGoalEnt;

	bool					m_bNotifyNavFailBlocked;
	bool					m_bNeverLeavePlayerSquad; // Don't leave the player squad unless killed, or removed via Entity I/O. 

													  //-----------------------------------------------------
	DECLARE_DATADESC();
#ifdef _XBOX
protected:
#endif
	DEFINE_CUSTOM_AI;
};

//---------------------------------------------------------
inline bool CNPC_CounterTerrorist::NearCommandGoal()
{
	const float flDistSqr = COMMAND_GOAL_TOLERANCE * COMMAND_GOAL_TOLERANCE;
	return ((GetAbsOrigin() - GetCommandGoal()).LengthSqr() <= flDistSqr);
}

//---------------------------------------------------------
inline bool CNPC_CounterTerrorist::VeryFarFromCommandGoal()
{
	const float flDistSqr = 360000; // 600*600 // 12*50
	return ((GetAbsOrigin() - GetCommandGoal()).LengthSqr() > flDistSqr);
}

#endif	//NPC_COUNTERTERRORIST_H
