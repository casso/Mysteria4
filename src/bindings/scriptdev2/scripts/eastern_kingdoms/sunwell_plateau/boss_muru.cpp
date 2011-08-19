/* ScriptData
SDName: boss_muru_entropius
SD%Complete:
SDComment:
EndScriptData */
 
#include "precompiled.h"
#include "sunwell_plateau.h"

#define ENRAGE 26662
#define DARKNESS 45996
#define SHADOW_PORTAL 40280
#define ENTROPIUS_EFFECT 46223
#define SUMMON_BERSERKER 46037
#define SUMMON_ENTROPIUS 46217
#define SUMMON_FURY_MAGE 46038
#define SUMMON_VOID_SENTINEL 45988
#define NEGATIVE_ENERGY_TRIGGER 46009
 
#define ENTROPIUS_DARKNESS 46269
#define ENTROPIUS_VOID_ZONE 46263
#define ENTROPIUS_NEGATIVE_ENERGY_TRIGGER 46284

#define ENRAGE_TIMER 600000
#define DARKNESS_TIMER 50000
#define SUMMON_HUMANOIDS_TIMER 8000
#define SUMMON_VOID_SENTINEL_TIMER 20000
 
#define SINGULARITY_TIMER 100000

#define ID_SWB 25799
#define ID_SWM 25798
#define ID_DARK_FIEND 25744
#define ID_VOID_SENTINEL 25772
 
#define ID_SINGULARITY 25855
 
#define DARK_FIEND_AURA 45934
#define DARK_FIEND_DEBUFF 45944

#define MIN 5
#define MAX 10

#define SOUND_CHANGE_PHASE 12560

#define SP_SPAWN_Z 78
#define ID_WORLD_TRIGGER 12999
 
float ShadowPortalSpawn[5][3] =
{
        {1797.644f,613.744f,0.56290f},
        {1822.956f,646.437f,4.40262f},
        {1794.275f,626.539f,6.23522f},
        {1809.726f,646.494f,5.01348f},     
        {1798.854f,638.952f,5.62435f}
};
 
uint32 DarkFiendsSpell[8] ={46000,46001,46002,46003,
                            46004,46005,46006,46007};
 
Creature* WayPoint[5] = {NULL, NULL, NULL, NULL, NULL};
 
uint32 EnrageTimer;
uint32 DarknessTimer;
 
float GetRandom()
    {
        float r = MIN + rand()%(MAX-MIN+1);
            
        switch(rand()%2)
        {
                case 0:
                        r = -r;
                        break;
                default:
                        break;
        }
        
        return r;
    }

void KillMob(Creature* Mob)
{
        if(Mob && Mob->isAlive())
                Mob->DealDamage(Mob, Mob->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
}
    
struct MANGOS_DLL_DECL boss_muruAI : public Scripted_NoMovementAI
{
    boss_muruAI(Creature *c) : Scripted_NoMovementAI(c) { Reset(); pInstance = (ScriptedInstance*)c->GetInstanceData(); }
 
    bool AuraApplied;
    uint8 ShadowPortalN;
    uint32 SummonHumanoidsTimer;
    uint32 SummonVoidSentinelTimer;
	bool justdiedd;
	bool entropvisual;
	ScriptedInstance* pInstance;
	Creature* SummonHumanoids[2];
    
    void Reset()
    {
        ShadowPortalN = 0;
        EnrageTimer = ENRAGE_TIMER;
        DarknessTimer = DARKNESS_TIMER;
        m_creature->SetVisibility(VISIBILITY_ON);
        SummonHumanoidsTimer = SUMMON_HUMANOIDS_TIMER;
        SummonVoidSentinelTimer = SUMMON_VOID_SENTINEL_TIMER;
        m_creature->RemoveAurasDueToSpell(NEGATIVE_ENERGY_TRIGGER);
        justdiedd = false;
		entropvisual = false;

        if(m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        
        for (uint8 i = 0; i < 5; i++)
        {
			//if(WayPoint[i])
            //    KillMob(WayPoint[i]);
			WayPoint[i] = NULL;
        }
    }
    
    void Aggro(Unit *who)
    {
        if(!m_creature->HasAura(NEGATIVE_ENERGY_TRIGGER))
            m_creature->CastSpell(m_creature, NEGATIVE_ENERGY_TRIGGER, true);
		pInstance->SetData(TYPE_MURU,IN_PROGRESS);
    }
 
    void JustDied(Unit* Killer)
    {
        //for (uint8 i = 0; i < 5; i++)
        //{
        //        KillMob(WayPoint[i]);
        //        WayPoint[i] = NULL;
        //}
        ShadowPortalN = 0;
    }
    
    void KilledUnit(Unit *Victim) {}
    
    void UpdateAI(const uint32 diff)
    {       
		if (justdiedd) return;
        //Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
                return;

		if(pInstance && pInstance->GetData(TYPE_MURU) == DONE)
		{
			m_creature->DealDamage(m_creature,m_creature->GetHealth(),0,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,0,false);
			m_creature->SetVisibility(VISIBILITY_OFF);
		}

		if(Creature* entrop = GetClosestCreatureWithEntry(m_creature,25840,100.0f))
		{
			if(entrop->isAlive())
			{
				m_creature->DealDamage(m_creature,m_creature->GetHealth(),0,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,0,false);
				m_creature->SetVisibility(VISIBILITY_OFF);
			}
		}

		if(m_creature->GetHealth() < m_creature->GetMaxHealth() / 100 * 3)
		{
			if(!entropvisual)
			{
			    m_creature->CastSpell(m_creature, ENTROPIUS_EFFECT, true);
				entropvisual = true;
			}
		}

		if(m_creature->GetHealth() < m_creature->GetMaxHealth() / 100 * 2)
		{
		if(justdiedd) return;
		justdiedd = true;
		m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->CastSpell(m_creature, SUMMON_ENTROPIUS, true);
        m_creature->SetVisibility(VISIBILITY_OFF);
            
        KillMob(m_creature);
		}
        
        //Goes in enrage
        if(EnrageTimer < diff)
        {
                //m_creature->CastSpell(m_creature, ENRAGE, true);
                EnrageTimer = 20000;
        }
        else EnrageTimer -= diff;
        
        //Cast Darkness
        if(DarknessTimer < diff)
        {
                m_creature->CastSpell(m_creature, DARKNESS, false);
                
                for(uint8 i = 0; i < 4; i++) 
                        m_creature->CastSpell(m_creature, DarkFiendsSpell[i], true);
                
                DarknessTimer = DARKNESS_TIMER+6000;
        }
        else DarknessTimer -= diff;

        if(SummonVoidSentinelTimer < diff)
        {
                float SP_SPAWN_X, SP_SPAWN_Y, SP_SPAWN_O = 0;
                
                if(ShadowPortalN < 5)
                {
                        SP_SPAWN_X = ShadowPortalSpawn[ShadowPortalN][0];
                        SP_SPAWN_Y = ShadowPortalSpawn[ShadowPortalN][1];
                        SP_SPAWN_O = ShadowPortalSpawn[ShadowPortalN][2];
                                //ID_WORLD_TRIGGER
                        WayPoint[ShadowPortalN] = m_creature->SummonCreature(17096, SP_SPAWN_X, SP_SPAWN_Y, SP_SPAWN_Z, SP_SPAWN_O, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 10000);
                        if(WayPoint[ShadowPortalN])
                        {
                                WayPoint[ShadowPortalN]->StopMoving();
                                WayPoint[ShadowPortalN]->GetMotionMaster()->Clear(false); // No moving!
								WayPoint[ShadowPortalN]->SetVisibility(VISIBILITY_ON);
								WayPoint[ShadowPortalN]->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
								WayPoint[ShadowPortalN]->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
								WayPoint[ShadowPortalN]->SetFloatValue(OBJECT_FIELD_SCALE_X, 0.5f);
                                WayPoint[ShadowPortalN]->CastSpell(WayPoint[ShadowPortalN], SHADOW_PORTAL, true);
                        }
                        ShadowPortalN++;
                }
                else
                {
                        uint8 RandSpwn = rand()%5;
                        
                        SP_SPAWN_X = ShadowPortalSpawn[RandSpwn][0];
                        SP_SPAWN_Y = ShadowPortalSpawn[RandSpwn][1];
                        SP_SPAWN_O = ShadowPortalSpawn[RandSpwn][2];
                }

                Creature* VoidSentinel = m_creature->SummonCreature(ID_VOID_SENTINEL, SP_SPAWN_X, SP_SPAWN_Y, m_creature->GetPositionZ(), SP_SPAWN_O, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);
                if(VoidSentinel)
				{
					VoidSentinel->AI()->AttackStart(m_creature->getVictim());
					//VoidSentinel->CastSpell(VoidSentinel,40280,false);
				}

                
                SummonVoidSentinelTimer = SUMMON_VOID_SENTINEL_TIMER + 40000 + rand()%5000;
        }
        else SummonVoidSentinelTimer -= diff;
        
        if(SummonHumanoidsTimer < diff)
        {
            SummonHumanoids[0] = m_creature->SummonCreature(25799,1785.4f,664.27f,71.19f,0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,10000);
			SummonHumanoids[0]->AI()->AttackStart(m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0));

			SummonHumanoids[1] = m_creature->SummonCreature(25798,1777.7f,659.63f,71.19f,0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,10000);
			SummonHumanoids[1]->AI()->AttackStart(m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0));

			SummonHumanoidsTimer = SUMMON_HUMANOIDS_TIMER+75000;
        }      
        else SummonHumanoidsTimer -= diff;
    }
};
 
struct MANGOS_DLL_DECL boss_entropiusAI : public ScriptedAI
{
    boss_entropiusAI(Creature *c) : ScriptedAI(c) { Reset();
	pInstance = (ScriptedInstance*)c->GetInstanceData(); }
 
    uint32 SingularityTimer;
	Creature* Singularity;
	bool singon;
	ScriptedInstance* pInstance;
	uint32 targetsearch;
    
    void Reset()
    {
        DarknessTimer = DARKNESS_TIMER;
        SingularityTimer = SINGULARITY_TIMER;
		targetsearch = 0;
		singon = false;
    }
    
    void Aggro(Unit *who)
    {
        if(!m_creature->HasAura(NEGATIVE_ENERGY_TRIGGER))
            m_creature->CastSpell(m_creature, NEGATIVE_ENERGY_TRIGGER, true);
    }
    
    void JustDied(Unit* Killer) {
	if (Singularity && singon)
		if(Singularity->isAlive())
			KillMob(Singularity);
	if(pInstance)
		pInstance->SetData(TYPE_MURU,DONE);
	}
    void KilledUnit(Unit *Victim) {}
            
    void UpdateAI(const uint32 diff)
    {
		if(targetsearch)
		{
			if(targetsearch < diff)
			{
				if(Creature* muru = GetClosestCreatureWithEntry(m_creature,25741,100.0f))
				{
					muru->Respawn();
				}
				m_creature->DealDamage(m_creature,m_creature->GetHealth(),0,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,0,false);
				m_creature->SetVisibility(VISIBILITY_OFF);
				if(pInstance)
					pInstance->SetData(TYPE_MURU,NOT_STARTED);
				targetsearch = 0;
			} else targetsearch -= diff;
		}

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
		{
			if(!targetsearch)
				targetsearch = 10000;
           return;
		}

		targetsearch = 0;

		if(pInstance && pInstance->GetData(TYPE_MURU) == DONE)
		{
			m_creature->SetVisibility(VISIBILITY_OFF);
			m_creature->DealDamage(m_creature,m_creature->GetHealth(),0,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,0,false);			
		}

        if(EnrageTimer < diff)
        {
                //m_creature->CastSpell(m_creature, ENRAGE, true);
                EnrageTimer = 20000;
        }
        else EnrageTimer -= diff;

        if(DarknessTimer < diff)
        {
                m_creature->CastSpell(m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0), ENTROPIUS_VOID_ZONE, false);
                DarknessTimer = DARKNESS_TIMER+6000;
        }
        else DarknessTimer -= diff;

        if(SingularityTimer < diff)
        {
			if (!singon) {
                Unit* sTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_BOTTOMAGGRO,0);
                
                Singularity = m_creature->SummonCreature(ID_SINGULARITY, sTarget->GetPositionX(), sTarget->GetPositionY(), sTarget->GetPositionZ(), m_creature->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, SINGULARITY_TIMER-5000);
				Singularity->SetVisibility(VISIBILITY_OFF);
                if(Singularity)
                {
						singon = true;
                        Singularity->CastSpell(Singularity, 46228, true);
                        Singularity->AI()->AttackStart(sTarget);
                }              
                SingularityTimer = SINGULARITY_TIMER;
			}
        }
        else SingularityTimer -= diff;

		if(singon)
		{
			if(Singularity)
			{
				Unit* trg = Singularity->getVictim();
				if(!trg || (trg && trg->isDead()))
				{
					singon = false;
					if(Singularity)
						KillMob(Singularity);
					SingularityTimer = SINGULARITY_TIMER;
				}
			}
		}
        
        DoMeleeAttackIfReady();
    }
};
 
struct MANGOS_DLL_DECL dark_fiendAI : public ScriptedAI
{
    dark_fiendAI(Creature *c) : ScriptedAI(c) { Reset(); }
 
    uint32 SingularityTimer;
    
    void Reset() {}
    
    void Aggro(Unit *who)
    {
        if(!m_creature->HasAura(DARK_FIEND_AURA))
            m_creature->CastSpell(m_creature, DARK_FIEND_AURA, true);
    }
    
    void JustDied(Unit* Killer) {}
    void KilledUnit(Unit *Victim) {}
            
    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
                return;
        
        if(m_creature->isAttackReady() && !m_creature->IsNonMeleeSpellCasted(false))
        {
                if( m_creature->IsWithinDistInMap(m_creature->getVictim(), ATTACK_DISTANCE))
                {
                    m_creature->CastSpell(m_creature->getVictim(), DARK_FIEND_DEBUFF, true);
                    m_creature->resetAttackTimer();
                }
        }
    }
};

struct MANGOS_DLL_DECL void_sentinel_muruAI : public ScriptedAI
{
    void_sentinel_muruAI(Creature *c) : ScriptedAI(c) { Reset(); }
 
    uint32 PulseTimer;
    
    void Reset() { PulseTimer = urand(10000,15000); }
    
    void Aggro(Unit *who)
    {
    }
    
    void JustDied(Unit* Killer) {
		for(int i=0;i<8;++i)
			m_creature->SummonCreature(25824,0,0,0,0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,10000);
	}
    void KilledUnit(Unit *Victim) {}
            
    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
                return;
        
		if (PulseTimer < diff)
		{
			m_creature->CastSpell(m_creature->getVictim(),46087,false);
			PulseTimer = 12000;
		} else PulseTimer -= diff;

		DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_muru(Creature *_Creature)
{
    return new boss_muruAI(_Creature);
}
 
CreatureAI* GetAI_boss_entropius(Creature *_Creature)
{
    return new boss_entropiusAI(_Creature);
}
 
CreatureAI* GetAI_dark_fiend(Creature *_Creature)
{
    return new dark_fiendAI(_Creature);
}

CreatureAI* GetAI_void_sentinel_muru(Creature *_Creature)
{
    return new void_sentinel_muruAI(_Creature);
}
 
void AddSC_boss_muru_entropius()
{
    Script *newscript;
        
    newscript = new Script;
    newscript->Name="boss_muru";
    newscript->GetAI = GetAI_boss_muru;
    newscript->RegisterSelf();
 
    newscript = new Script;
    newscript->Name="boss_entropius";
    newscript->GetAI = GetAI_boss_entropius;
    newscript->RegisterSelf();
 
    newscript = new Script;
    newscript->Name="dark_fiend";
    newscript->GetAI = GetAI_dark_fiend;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="void_sentinel_muru";
    newscript->GetAI = GetAI_void_sentinel_muru;
    newscript->RegisterSelf();
}
