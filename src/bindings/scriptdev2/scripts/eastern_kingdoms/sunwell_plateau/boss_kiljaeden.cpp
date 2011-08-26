/* Copyright 2006,2007 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
 
/* ScriptData
SDName: boss_kiljaden
SD%Complete: 90%
SDComment: 
SDAuthor: ScrappyDoo + Gregory
EndScriptData */
 
/*
Phase1 100%
Phase2 100%
Phase3 100%
Phase4 100%
Phase5 100%

DragonOrbs Event            = in progress
Kalecgos Event              = Implemented
Kalecgos And Anvena Event   = Implemented
Outro Event                 = Implemented
Shield Orb Event            = Implemented
Shadow Spikes               = Implemented
*/

#include "precompiled.h"
#include "sunwell_plateau.h"
 
enum Speeches
{
    // These are used throughout Sunwell and Magisters(?). Players can hear this while running through the instances.
    SAY_KJ_OFFCOMBAT1   = 12499, //in
    SAY_KJ_OFFCOMBAT2   = 12498, //in
    SAY_KJ_OFFCOMBAT3   = 12497, //in
    SAY_KJ_OFFCOMBAT4   = 12496, //in
    SAY_KJ_OFFCOMBAT5   = 12495, //in

    // Encounter speech and sounds
    SAY_KJ_EMERGE       = 12500, //in
    SAY_KJ_SLAY1        = 12501, //in
    SAY_KJ_SLAY2        = 12502, //in
    SAY_KJ_REFLECTION1  = 12504,
    SAY_KJ_REFLECTION2  = 12503,
    SAY_KJ_DARKNESS1    = 12505, //in
    SAY_KJ_DARKNESS2    = 12506, //in
    SAY_KJ_DARKNESS3    = 12507, //in
    SAY_KJ_PHASE3       = 12508, //in
    SAY_KJ_PHASE4       = 12509, //in
    SAY_KJ_PHASE5       = 12510, //in
    SAY_KJ_DEATH        = 12331, //in
    EMOTE_KJ_DARKNESS   = 0,

    /*** Kalecgos - Anveena speech at the beginning of Phase 5; Anveena's sacrifice ***/
    SAY_KALECGOS_AWAKEN     = 12445,
    SAY_ANVEENA_IMPRISONED  = 12511,
    SAY_KALECGOS_LETGO      = 12446, 
    SAY_ANVEENA_LOST        = 12512, 
    SAY_KALECGOS_FOCUS      = 12447,
    SAY_ANVEENA_KALEC       = 12513,
    SAY_KALECGOS_FATE       = 12448,
    SAY_ANVEENA_GOODBYE     = 12514,
    SAY_KALECGOS_GOODBYE    = 12449,
    SAY_KALECGOS_ENCOURAGE  = 12450, // 12439 breat

    /*** Kalecgos says throughout the fight ***/
    SAY_KALECGOS_JOIN       = 12438, //in
    SAY_KALEC_ORB_READY1    = 12440,
    SAY_KALEC_ORB_READY2    = 12441,
    SAY_KALEC_ORB_READY3    = 12442,
    SAY_KALEC_ORB_READY4    = 12443,

    /*** Outro Sounds***/
    SAY_VELEN_01 = 12515,
    SAY_VELEN_02 = 12516,
    SAY_VELEN_03 = 12517,
    SAY_VELEN_04 = 12518,
    SAY_VELEN_05 = 12519,
    SAY_VELEN_06 = 12520,
    SAY_VELEN_07 = 12521,
    SAY_VELEN_08 = 12522,
    SAY_VELEN_09 = 12523,
    SAY_LIADRIN_01 = 12526,
    SAY_LIADRIN_02 = 12525,
    SAY_LIADRIN_03 = 12524,
};

enum spells
{
    //Kilajden Spells
    AURA_SUNWELL_RADIANCE       = 45769, // NOT WORKING
    SPELL_REBIRTH               = 44200, // Emerge from the Sunwell Epick :D

    SPELL_SOULFLY               = 45442, // 100%
    SPELL_DARKNESS_OF_SOULS     = 46605, // 100% Visual Effect
    SPELL_DARKNESS_EXPLOSION    = 45657, // 100% Damage
    SPELL_DESTROY_DRAKES        = 46707, // ?
    SPELL_FIREBLOOM             = 45641, // NOT WORKING
    SPELL_FIREBLOOM_EFF         = 45642, // 100% Damage
    SPELL_FLAMEDARTS            = 45737, // 100%
    SPELL_LEGION_LIGHTING       = 45664, // 100%

    SPELL_ARMAGEDDON_EFFECT     = 24207, // 100% Visual
    SPELL_ARMAGEDDON_TRIGGER    = 45909, // Meteor spell trigger missile should cast Creature on himself
    SPELL_ARMAGEDDON_VISUAL     = 45911, // Does the hellfire visual to indicate where the meteor missle lands
    SPELL_ARMAGEDDON_VISUAL2    = 45914, // Does the light visual to indicate where the meteor missle lands

    //SPELL_SINISTER_REFLECTION   = 45892, // NOT WORKING -> WorkArround 99%
    SPELL_SACRIFICE_OF_ANVEENA  = 46474, // 100%

    SPELL_KNOCKBACK             = 0,     // ?

    //Hand of the Deceiver Spells
    SPELL_FELLFIRE_PORTAL       = 46875, // NOT WORKING -> WorkArround 99%
    SPELL_SHADOWBOLT_VOLLEY     = 45770, // 100%
    SPELL_SHADOWINFUSION        = 45772, // 100%
    SPELL_IMPFUSION             = 45779, // 100%
    SPELL_SHADOW_CHANNELING     = 46757, // Channeling animation out of combat

    //Orb Spells
    SPELL_ORB_BOLT              = 45680, // NOT WORKING
    // SPELL_ORB_BOLT              = 53086, // 100%

    //Reflections Spells
    // SPELL_PRIEST                = 47077, // 100%
    // SPELL_PALADIN               = 37369, // 100%
    // SPELL_PALADIN2              = 37369, // 100%
    // SPELL_WARLOCK               = 46190, // 100%
    // SPELL_WARLOCK2              = 47076, // 100%
    // SPELL_MAGE                  = 47074, // 100%
    // SPELL_ROGUE                 = 45897, // 100%
    // SPELL_WARRIOR               = 17207, // 100%
    // SPELL_DRUID                 = 47072, // 100%
    // SPELL_SHAMAN                = 47071, // 100%
    // SPELL_HUNTER                = 48098, // 100%

    //Orbs of DragonFligth
    SPELL_REVITALIZE            = 45027, // 100%
    SPELL_SHIELD_OF_BLUE        = 45848, // 100%
    AURA_BLUESHIELD             = 44867, // :D
    SPELL_HASTE                 = 45856, // NOT WORKING

    //Other Spells (used by players, etc)
    SPELL_VENGEANCE_OF_THE_BLUE_FLIGHT  = 45839, // Possess the blue dragon from the orb to help the raid.
};

enum creature_ids
{
    ID_ANVEENA      = 26046, // Embodiment of the Sunwell
    ID_KALECGOS     = 25319, // Helps the raid throughout the fight
    ID_VELEN        = 26246, // Outro
    ID_LIADRIN      = 26247, // outro

    IF_KILJAEDEN_CONTROLLER = 25608,
    ID_KILJADEN     = 25315,
    ID_DECIVER      = 25588,
    ID_SHIELDORB    = 25502,
    //ID_SINISTER     = 25708, // Sinister Reflection
    ID_ARMAGEDON    = 25735,
    ID_IMP          = 25598,
    ID_PORTAL       = 25603, //Imp Portal
    ID_DRAGON       = 25653,

    ID_THE_CORE_OF_ENTROPIUS  = 26262, // Used in the ending cinematic?
};
 
float OrbSpawn [4][2] =
{
    {1853.300f,588.653f},
    {1698.900f,627.870f},
    {1781.502f,659.254f},
    {1853.300f,588.653f},
};

float DragonSpawnCoord[4][2] =
{
    (1668.711f, 643.723f),
    (1745.68f, 621.823f),
    (1704.14f, 583.591f),
    (1653.12f, 635.41f),
};

float DeceiverPos[12] =
{
    1705.20f, 607.20f, 28.5500f, 1.990370f, // vpravo
    1684.70f, 614.42f, 28.0580f, 0.698392f, // v strede
    1682.95f, 637.75f, 27.9231f, 5.717090f, // vlavo
};

#define GAMEOBJECT_ORB_OF_THE_BLUE_DRAGONFLIGHT 188415

#define GOSSIP_ITEM_1 "Shield of the Blue Dragon Flight"
#define GOSSIP_ITEM_2 "Revitalize"
#define GOSSIP_ITEM_3 "Haste"
#define GOSSIP_ITEM_4 "Blink"
#define GOSSIP_ITEM_5 "Fight"

// --- Kiljaden --- 
struct MANGOS_DLL_DECL boss_kiljadenAI : public Scripted_NoMovementAI
{
    boss_kiljadenAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        Reset();
    }
 
    ScriptedInstance* pInstance;
    
    bool m_rebirth_casted;
    bool after_aggro_yell;

    uint64 m_uiKalecgosGUID;
    uint64 m_uiAnveenaGUID;

    //BlueShield WorkArround
    uint32 m_uiCancelShieldTimer;

    uint64 m_uiDragonGUID[4];

    uint32 m_uiOrbTimer;
    bool m_bBoolOrb;

    uint32 m_uiKalecgosTimer;
    bool m_bIsKalecgosSpawned;

    //SpeachTimers
    uint32 m_uiKJOrdersTimer;

    //Anvena and Kalecgos Timer
    uint32 m_uiKalecgosAnvenaCount;
    bool m_bIsAnvena;
    uint32 m_uiKalecgosAnvenaTimer;

    //FireBloom Targets Table
    uint64 m_uiFireBloomTarget[5];

    //Phase2
    uint32 m_uiSoulFlyTimer;
    uint32 m_uiLegionLightingTimer;
    uint32 m_uiFireBloomCheck;
    uint32 m_uiFireBloomTimer;
    uint32 m_uiFireBloomCount;
    uint32 m_uiShieldOrbTimer;

    //Phase3
    bool m_bPhase3;
    uint32 m_uiFlameDartTimer;
    uint32 m_uiDarknessOfSoulsTimer;
    uint32 m_uiDarknessExplosionTimer;

    //Phase4
    bool m_bPhase4;
    bool m_bDarknessOfSoulsCasting;
    uint32 m_uiAramageddonTimer;
    
    //Phase5
    bool m_bPhase5;

    //Sinister Reflections
    //uint64 m_uiSinisterGUID[12][2];
    //uint32 m_uiSinnisterCastTimer;
 
    void Reset()
    {
        m_rebirth_casted = false;
        after_aggro_yell = false;

        //BlueShield WorkArround
        m_uiCancelShieldTimer = 300000;

        //creatures
        m_uiKalecgosGUID = 0;
        m_uiAnveenaGUID = 0;

        //SpeachTimers
        m_uiKJOrdersTimer = 10000;

        m_bIsAnvena = false;

        //Kalecgos Event
        m_uiKalecgosTimer = 25000;
        m_bIsKalecgosSpawned = false;

        m_bBoolOrb = true;

        //Phase2
        m_uiSoulFlyTimer        = 1000;
        m_uiLegionLightingTimer = 15000;
        m_uiFireBloomCheck      = 2000;
        m_uiFireBloomTimer      = 30000;
        m_uiFireBloomCount      = 10;
        m_uiShieldOrbTimer      = 25000;

        m_bDarknessOfSoulsCasting  = false;
        m_uiDarknessExplosionTimer = 120000;

        m_bPhase3 = false;
        m_bPhase4 = false;
        m_bPhase5 = false;

        m_creature->SetVisibility(VISIBILITY_OFF);
        m_creature->setFaction(35);

        if(!m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

        if(!m_creature->HasAura(AURA_SUNWELL_RADIANCE))
            m_creature->CastSpell(m_creature, AURA_SUNWELL_RADIANCE, true);

        if(m_creature->HasAura(SPELL_SACRIFICE_OF_ANVEENA))
            m_creature->RemoveAurasDueToSpell(SPELL_SACRIFICE_OF_ANVEENA,0);

        if(pInstance)
        {
            pInstance->SetData(DATA_DECIVER, NOT_STARTED); 
            pInstance->SetData(DATA_KILJAEDEN_EVENT, NOT_STARTED);
        }

        if(Creature* kalecc = GetClosestCreatureWithEntry(m_creature,ID_KALECGOS,100.0f))
        {
            kalecc->SetVisibility(VISIBILITY_OFF);
            kalecc->DealDamage(kalecc,kalecc->GetHealth(),NULL,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,NULL,false);
                kalecc->ForcedDespawn();
        }
        else
        {
            error_log("KJ::Reset : Kalecgos not found.");
        }

        for(unsigned int i = 0 ; i < 3 ; i++)
            m_creature->SummonCreature(ID_DECIVER, DeceiverPos[i*4+0], DeceiverPos[i*4+1], DeceiverPos[i*4+2], DeceiverPos[i*4+3], TEMPSUMMON_CORPSE_DESPAWN, 10000);

        for(uint8 i=0; i<4; ++i)
            m_uiDragonGUID[i] = 0;

        m_creature->GetMotionMaster()->MovePoint(0,1698.67f,628.028f,28.3f, false);
    }
    
    void Aggro(Unit *who) 
    {
        m_creature->SetInCombatWithZone();
        if(!after_aggro_yell)
        {
            DoPlaySoundToSet(m_creature, SAY_KJ_EMERGE);
            m_creature->MonsterYell("The expendible have perished... So be it! Now I shall succeed where Sargeras could not! I will bleed this wretched world and secure my place as the true master of the Burning Legion. The end has come! Let the unraveling of this world commence!",LANG_UNIVERSAL,NULL);

            after_aggro_yell = true;
        }
        m_creature->SetVisibility(VISIBILITY_ON);
        //m_creature->CastSpell(m_creature, SPELL_REBIRTH, false);
    }

    void KilledUnit(Unit *Victim) 
    {
        switch (rand()%2)
        {  
        case 0: DoPlaySoundToSet(m_creature, SAY_KJ_SLAY1); m_creature->MonsterYell("Another step towards destruction!",LANG_UNIVERSAL,NULL); break;
        case 1: DoPlaySoundToSet(m_creature, SAY_KJ_SLAY2); m_creature->MonsterYell("Anukh-Kyrie!",LANG_UNIVERSAL,NULL); break;
        }
    }

    /*
    void Sinister(Player* victim,uint8 i,uint8 k)
    {
        uint32 spell;
        switch(victim->getClass())
        {
            case CLASS_PRIEST:  spell = SPELL_PRIEST; break;
            case CLASS_PALADIN: spell = SPELL_PALADIN; break;
            case CLASS_WARLOCK: spell = SPELL_WARLOCK; break;
            case CLASS_MAGE:    spell = SPELL_MAGE; break;
            case CLASS_ROGUE:   spell = SPELL_ROGUE; break;
            case CLASS_WARRIOR: spell = SPELL_WARRIOR; break;
            case CLASS_DRUID:   spell = SPELL_DRUID; break;
            case CLASS_SHAMAN:  spell = SPELL_SHAMAN; break;
            case CLASS_HUNTER:  spell = SPELL_HUNTER; break;
        }

        
        for(i=i; i<k; ++i)
        {
            Creature* Sinister = m_creature->SummonCreature(ID_SINISTER, victim->GetPositionX()+rand()%3, victim->GetPositionY()+rand()%3, m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
            if(Sinister)
            {
                Sinister->SetDisplayId(victim->GetDisplayId());
                Sinister->AI()->AttackStart(Sinister->getVictim());
                m_uiSinisterGUID[i][0] = Sinister->GetGUID();
                m_uiSinisterGUID[i][1] = spell;
            }
        }
    }
    */

    void JustDied(Unit* Killer) 
    {
        DoPlaySoundToSet(m_creature, SAY_KJ_DEATH);
        m_creature->MonsterYell("Nooooooooooooo!",LANG_UNIVERSAL,NULL);

        if(pInstance)
            pInstance->SetData(DATA_KILJAEDEN_EVENT, DONE);
    }

    void DamageDeal(Unit* pDoneTo, uint32& uiDamage) 
    {
        if(pDoneTo->HasAura(AURA_BLUESHIELD))
        {
            uiDamage = uiDamage * 0.05;
        } else {
            if(Creature* dracek = GetClosestCreatureWithEntry(pDoneTo,25653,150.0f))
            {
                if(dracek->HasAura(45848))
                    uiDamage = uiDamage * 0.05;
            }
            else
            {
                error_log("KJ::DealDamage : Dracek not found.");
            }
        }
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if(pDoneBy->HasAura(AURA_BLUESHIELD))
        {
            uiDamage = uiDamage * 0.05;
        } else {
            if(Creature* dracek = GetClosestCreatureWithEntry(pDoneBy,25653,150.0f))
            {
                if(dracek->HasAura(45848))
                    uiDamage = uiDamage * 0.05;
            }
            else
            {
                error_log("KJ::DamageTaken : Dracek not found.");
            }
        }
    }

    void SpellHit(Unit* pCaster, const SpellEntry* spell)
    {
        if(spell->Id == 20271)
        {
            if(m_creature->FindCurrentSpellBySpellId(46680))
            {
                return;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Sounds OOC, Kiljaeden Orders
        if(!m_creature->getVictim())
        {
            if(m_uiKJOrdersTimer < diff)
            {
                switch (rand()%5)
                {
                    case 0: DoPlaySoundToSet(m_creature, SAY_KJ_OFFCOMBAT1); break;
                    case 1: DoPlaySoundToSet(m_creature, SAY_KJ_OFFCOMBAT2); break;
                    case 2: DoPlaySoundToSet(m_creature, SAY_KJ_OFFCOMBAT3); break;
                    case 3: DoPlaySoundToSet(m_creature, SAY_KJ_OFFCOMBAT4); break;
                    case 4: DoPlaySoundToSet(m_creature, SAY_KJ_OFFCOMBAT5); break;
                }
                m_uiKJOrdersTimer = 30000;
            }else m_uiKJOrdersTimer -= diff;
        }
        else if(!m_rebirth_casted)
        {
            m_creature->CastSpell(m_creature, SPELL_REBIRTH, false);
            m_rebirth_casted = true;
        }

        //Rebirth After Phase1
        if(pInstance && pInstance->GetData(DATA_DECIVER) == SPECIAL)
        {
            m_creature->setFaction(14);
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            pInstance->SetData(DATA_KILJAEDEN_EVENT, IN_PROGRESS);
            pInstance->SetData(DATA_DECIVER, NOT_STARTED);
            Player* pTarget = pInstance->instance->GetPlayers().begin()->getSource();
            if(pTarget)
                Aggro(pTarget);
        }

        if(!m_creature->SelectHostileTarget() || !m_creature->getVictim())
               return;

        //FireBloom Damage WorkArround
        if (m_uiFireBloomCheck < diff)
        {
            if(m_uiFireBloomCount < 10)
                for(uint8 i=0; i<5; ++i)
                {
                    if(Unit* FireTarget = pInstance->instance->GetCreature(m_uiFireBloomTarget[i]))
                        FireTarget->CastSpell(FireTarget, SPELL_FIREBLOOM_EFF, true);
                }
            ++m_uiFireBloomCount;
            m_uiFireBloomCheck = 2000;
        }else m_uiFireBloomCheck -= diff;

        // After Each Phase Dragons Are Spawned
        if((m_uiOrbTimer < diff) && !m_bBoolOrb)
        {
            Creature* Kalec = GetClosestCreatureWithEntry(m_creature,ID_KALECGOS,100.0f);
            if(Kalec)
            {
                switch (rand()%4)
                {
                    case 0: DoPlaySoundToSet(m_creature, SAY_KALEC_ORB_READY1); Kalec->MonsterSay("I will channel my power into the orbs, be ready!",LANG_UNIVERSAL,NULL); break;
                    case 1: DoPlaySoundToSet(m_creature, SAY_KALEC_ORB_READY2); Kalec->MonsterSay("I have empowered another orb! Use it quickly!",LANG_UNIVERSAL,NULL); break;
                    case 2: DoPlaySoundToSet(m_creature, SAY_KALEC_ORB_READY3); Kalec->MonsterSay("Another orb is ready! Make haste!",LANG_UNIVERSAL,NULL); break;
                    case 3: DoPlaySoundToSet(m_creature, SAY_KALEC_ORB_READY4); Kalec->MonsterSay("I have channeled all I can! The power is in your hands!",LANG_UNIVERSAL,NULL); break;
                }
            }
            uint8 m_uiMaxDragons = 1;
            if(m_bPhase5)
                m_uiMaxDragons = 4;
            for(uint8 i=0; i<m_uiMaxDragons; ++i)
            {
                Creature* Dragon = m_creature->SummonCreature(ID_DRAGON, m_creature->GetPositionX()+urand(20,35), m_creature->GetPositionY()+urand(20,35), m_creature->GetPositionZ()+1, 0, TEMPSUMMON_TIMED_DESPAWN, 120000);
                m_uiDragonGUID[i] = Dragon->GetGUID();
            }
            m_bBoolOrb = true;
        }else m_uiOrbTimer -= diff;

        for(uint8 i=0; i<4; ++i)
        {
            if(Unit* Dragon = pInstance->instance->GetCreature(m_uiDragonGUID[i]))
                if(Dragon && Dragon->HasAura(SPELL_SHIELD_OF_BLUE))
                {
                    m_uiCancelShieldTimer = 5000;
                    std::list<HostileReference *> t_list = m_creature->getThreatManager().getThreatList();
                    for(std::list<HostileReference *>::iterator itr = t_list.begin(); itr!= t_list.end(); ++itr)
                    {
                        Unit *TargetedPlayer = pInstance->instance->GetCreature((*itr)->getUnitGuid());  
                        if (TargetedPlayer && TargetedPlayer->GetTypeId() == TYPEID_PLAYER && TargetedPlayer->IsWithinDistInMap(Dragon, 10) && !TargetedPlayer->HasAura(AURA_BLUESHIELD))
                            TargetedPlayer->CastSpell(TargetedPlayer,AURA_BLUESHIELD,true);
                    }
                }
        }

        if(m_uiCancelShieldTimer < diff)
        {
            std::list<HostileReference *> t_list = m_creature->getThreatManager().getThreatList();
            for(std::list<HostileReference *>::iterator itr = t_list.begin(); itr!= t_list.end(); ++itr)
            {
                Unit *ShieldedPlayer1 = pInstance->instance->GetCreature((*itr)->getUnitGuid());
                if (ShieldedPlayer1 && ShieldedPlayer1->GetTypeId() == TYPEID_PLAYER && ShieldedPlayer1->HasAura(AURA_BLUESHIELD))
                {
                    ShieldedPlayer1->RemoveAurasDueToSpell(AURA_BLUESHIELD);
                } 
            }
            m_uiCancelShieldTimer = 300000;
        }else m_uiCancelShieldTimer -= diff;

        //Kalecgos and Anvena Event
        if((m_uiKalecgosAnvenaTimer < diff) && m_bIsAnvena)
        {
            Creature* Kalec = GetClosestCreatureWithEntry(m_creature,ID_KALECGOS,100.0f);
            Creature* Anveena = GetClosestCreatureWithEntry(m_creature,ID_ANVEENA,100.0f);
            if(Kalec && Anveena)
            {
            switch(m_uiKalecgosAnvenaCount)
            {
                case 0:
                DoPlaySoundToSet(m_creature, SAY_KALECGOS_AWAKEN); Kalec->MonsterSay("Anveena, you must awaken, this world needs you!",LANG_UNIVERSAL,NULL); m_uiKalecgosAnvenaTimer = 5000; break;
                case 1:
                DoPlaySoundToSet(m_creature, SAY_ANVEENA_IMPRISONED); Anveena->MonsterSay("I serve only the master now.",LANG_UNIVERSAL,NULL); m_uiKalecgosAnvenaTimer = 3000; break;
                case 2:
                DoPlaySoundToSet(m_creature, SAY_KALECGOS_LETGO); Kalec->MonsterSay("You must let go! You must become what you were always meant to be! The time is now, Anveena!",LANG_UNIVERSAL,NULL); m_uiKalecgosAnvenaTimer = 6000; break;
                case 5:
                DoPlaySoundToSet(m_creature, SAY_ANVEENA_LOST); Anveena->MonsterSay("But I'm... lost. I cannot find my way back.",LANG_UNIVERSAL,NULL); m_uiKalecgosAnvenaTimer = 4000; break;
                case 4:
                DoPlaySoundToSet(m_creature, SAY_KALECGOS_FOCUS); Kalec->MonsterSay("Anveena, I love you! Focus on my voice, come back for me now! Only you can cleanse the Sunwell!",LANG_UNIVERSAL,NULL); m_uiKalecgosAnvenaTimer = 8000; break;
                case 3:
                DoPlaySoundToSet(m_creature, SAY_ANVEENA_KALEC); Anveena->MonsterSay("Kalec...? Kalec?",LANG_UNIVERSAL,NULL); m_uiKalecgosAnvenaTimer = 5000; break;
                case 6:
                DoPlaySoundToSet(m_creature, SAY_KALECGOS_FATE); Kalec->MonsterSay("Yes, Anveena! Let fate embrace you now!",LANG_UNIVERSAL,NULL); m_uiKalecgosAnvenaTimer = 5000; break;
                case 7:
                DoPlaySoundToSet(m_creature, SAY_ANVEENA_GOODBYE);
                Anveena->MonsterSay("The nightmare is over, the spell is broken! Goodbye, Kalec, my love!",LANG_UNIVERSAL,NULL);
                m_creature->CastSpell(m_creature, SPELL_SACRIFICE_OF_ANVEENA, false);
                if(Unit* Anveena = pInstance->instance->GetCreature(m_uiAnveenaGUID))
                    if(Anveena && Anveena->isAlive())
                        Anveena->SetVisibility(VISIBILITY_OFF);
                m_uiKalecgosAnvenaTimer = 5000;
                break;
                case 9:
                DoPlaySoundToSet(m_creature, SAY_KALECGOS_GOODBYE); m_creature->MonsterSay("Goodbye, Anveena, my love. Few will remember your name, yet this day you change the course of destiny. What was once corrupt is now pure. Heroes, do not let her sacrifice be in vain.",LANG_UNIVERSAL,NULL); m_bIsAnvena = false; break;
                case 8:
                DoPlaySoundToSet(m_creature, SAY_KALECGOS_ENCOURAGE); m_creature->MonsterSay("Strike now, heroes, while he is weakened! Vanquish the Deceiver!",LANG_UNIVERSAL,NULL); m_uiKalecgosAnvenaTimer = 14000; break;         
            }
            }
            ++m_uiKalecgosAnvenaCount;
        }m_uiKalecgosAnvenaTimer -= diff;

        //Kalecgos Event
        if((m_uiKalecgosTimer < diff) && !m_bIsKalecgosSpawned)
        {
            DoPlaySoundToSet(m_creature, SAY_KALECGOS_JOIN);
            if(Creature* cKalecgos = m_creature->SummonCreature(ID_KALECGOS, m_creature->GetPositionX()-25, m_creature->GetPositionY()-25, m_creature->GetPositionZ(), 0.686f, TEMPSUMMON_TIMED_DESPAWN, 600000))
            {
                cKalecgos->MonsterSay("You are not alone. The Blue Dragonflight shall help you vanquish the Deceiver.",LANG_UNIVERSAL,NULL);
                m_uiKalecgosGUID = cKalecgos->GetGUID();
                cKalecgos->setFaction(35);
                cKalecgos->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            //Kalecgos need to start shhooting arcane bolt into Kiljaeden
            //Dragon->AI()->AttackStart(m_creature);
            }
            else
            {
                error_log("KJ::UpdateAI : Kalecgos not found.");
            }

            m_bIsKalecgosSpawned = true;
        }m_uiKalecgosTimer -= diff;

        //Shield At Start each phases
        if(m_uiShieldOrbTimer < diff && !m_bPhase5)
        {
            uint8 l=1;
            if(m_bPhase3)
                l=2;
            if(m_bPhase4)
                l=3;
            for(uint8 k=0; k<l; ++k)
            {
                Creature* ShieldOrb = m_creature->SummonCreature(ID_SHIELDORB, m_creature->GetPositionX()+urand(1,15), m_creature->GetPositionY()+urand(1,15), m_creature->GetPositionZ()+10, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000);
                if(ShieldOrb)
                    ShieldOrb->AI()->AttackStart(m_creature->getVictim()); 
            }
            
            m_uiShieldOrbTimer = 50000; 
        }else m_uiShieldOrbTimer -= diff;
        
        /*
        //Sinister Reflects Attack
        if(m_uiSinnisterCastTimer < diff)
        {
            uint8 m_uiSinisterCount = 0;
            if(m_bPhase3)
                m_uiSinisterCount = 4;
            if(m_bPhase4)
                m_uiSinisterCount = 8;
            if(m_bPhase5)
                m_uiSinisterCount = 12;
            for(uint8 i=0; i<m_uiSinisterCount; ++i)
            {
                if(Unit* Sinister = pInstance->instance->GetCreature(m_uiSinisterGUID[i][0]))
                {
                    if(!Sinister->isDead())
                    {
                        if(Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0))
                            Sinister->CastSpell(target, m_uiSinisterGUID[i][1], true);
                    }
                }
            }
            m_uiSinnisterCastTimer = 8000;
        }else m_uiSinnisterCastTimer -= diff;
        */

        //Phase4
        if((m_uiAramageddonTimer < diff) && m_bPhase4)
        {
            uint8 h=3;
            if(m_bPhase5)
                h=5; // TODO pocet armagedonov
            if(!m_bPhase5 && m_bDarknessOfSoulsCasting)
                h=0;
            
            for(uint8 i=0; i<h; ++i)
            {
                if(Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0))
                    Creature* Armagedon = m_creature->SummonCreature(ID_ARMAGEDON, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 15000);
            }
            m_uiAramageddonTimer = 14000;
        }else m_uiAramageddonTimer -= diff;

        if(m_uiDarknessExplosionTimer < diff && m_bDarknessOfSoulsCasting)
        {
            m_creature->CastSpell(m_creature->getVictim(), SPELL_DARKNESS_EXPLOSION, true);
            m_bDarknessOfSoulsCasting = false;
            m_uiDarknessExplosionTimer = 600000;
            m_uiDarknessOfSoulsTimer = 45000;
            m_uiFireBloomTimer = 25000;
            if(m_bPhase5)
                m_uiDarknessOfSoulsTimer = 25000;
        }else m_uiDarknessExplosionTimer -= diff;

        if(m_bDarknessOfSoulsCasting)
            return;

        if(m_uiDarknessOfSoulsTimer < diff && m_bPhase3)
        {
            switch (rand()%3)
            {
                case 0: DoPlaySoundToSet(m_creature, SAY_KJ_DARKNESS1); m_creature->MonsterYell("Chaos!",LANG_UNIVERSAL,NULL); break;
                case 1: DoPlaySoundToSet(m_creature, SAY_KJ_DARKNESS2); m_creature->MonsterYell("Destruction!",LANG_UNIVERSAL,NULL); break;
                case 2: DoPlaySoundToSet(m_creature, SAY_KJ_DARKNESS3); m_creature->MonsterYell("Oblivion!",LANG_UNIVERSAL,NULL); break;
            }
            m_creature->CastSpell(m_creature, SPELL_DARKNESS_OF_SOULS, false);
            m_bDarknessOfSoulsCasting = true;
            m_uiDarknessExplosionTimer = 8500;
        }else m_uiDarknessOfSoulsTimer -= diff;

        //Phases
        if(((m_creature->GetHealth()*100 / m_creature->GetMaxHealth()) <= 85) && !m_bPhase3)
        {      
            DoPlaySoundToSet(m_creature, SAY_KJ_PHASE3);
            m_creature->MonsterYell("I will not be denied! This world shall fall!",LANG_UNIVERSAL,NULL);
            
            /*
            if(Unit* victim = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0)) 
                if (victim && (victim->GetTypeId() == TYPEID_PLAYER))
                    Sinister(((Player*)victim),0,4);
            */

            //Start Timerow  Dochodzacych w 3 Fazie
            //m_uiSinnisterCastTimer     = 10000;
            m_uiFlameDartTimer         = 40000;
            m_uiDarknessOfSoulsTimer   = 45000;
            m_bDarknessOfSoulsCasting  = false;
            m_bPhase3 = true;
            //DragonsTimer
            m_uiOrbTimer = 35000;
            m_bBoolOrb = false;
        }

        if(((m_creature->GetHealth()*100 / m_creature->GetMaxHealth()) <= 55) && !m_bPhase4)
        {
            DoPlaySoundToSet(m_creature, SAY_KJ_PHASE4);
            m_creature->MonsterYell("Do not harbor false hope. You cannot win!",LANG_UNIVERSAL,NULL);
            
            /*
            if(Unit* victim = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0)) 
                if (victim && (victim->GetTypeId() == TYPEID_PLAYER))
                    Sinister(((Player*)victim),4,8);
            */

            //Start Timerow  Dochodzacych w 4 Fazie
            m_uiAramageddonTimer       = 2000;  //100% ok
            m_bPhase4 = true;
            m_uiDarknessOfSoulsTimer   = 45000;
            //DragonsTimer
            m_uiOrbTimer = 35000;
            m_bBoolOrb = false;
        }

        if(((m_creature->GetHealth()*100 / m_creature->GetMaxHealth()) <= 25) && !m_bPhase5)
        {
            DoPlaySoundToSet(m_creature, SAY_KJ_PHASE5);
            m_creature->MonsterSay("Aggghh! The powers of the Sunwell... turn... against me! What have you done? What have you done???",LANG_UNIVERSAL,NULL);
            
            /*
            if(Unit* victim = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0)) 
                if (victim && (victim->GetTypeId() == TYPEID_PLAYER))
                    Sinister(((Player*)victim),8,12);
            */

            //Start Timerow  Dochodzacych w 5 Fazie
            m_uiDarknessOfSoulsTimer   = 45000;
            m_bPhase5 = true;
            //DragonsTimer
            m_uiOrbTimer = 35000;
            m_bBoolOrb = false;
            //Kalecgos and Anvena Event
            m_uiKalecgosAnvenaTimer = 20000;
            m_bIsAnvena = true;
            m_uiKalecgosAnvenaCount = 0;

            if(Creature* cAnveena = GetClosestCreatureWithEntry(m_creature,ID_ANVEENA,100.0f))
            {
                m_uiAnveenaGUID = cAnveena->GetGUID();
            } else {
                if(Creature* cAnveena = m_creature->SummonCreature(ID_ANVEENA, m_creature->GetPositionX()+urand(20,30), m_creature->GetPositionY()+urand(20,30), m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 120000))
                {
                    m_uiAnveenaGUID = cAnveena->GetGUID();
                    //cAnveena->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    cAnveena->setFaction(35);
                }
            }
        }

        //Phase2
        if(m_uiLegionLightingTimer < diff)
        {
            DoCast(m_creature->getVictim(), SPELL_LEGION_LIGHTING);
            m_uiLegionLightingTimer = 11000;
        }else m_uiLegionLightingTimer -= diff;

        if(m_uiFireBloomTimer < diff)
        {
            for(uint8 i=0; i<5; ++i)
            {
                Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0);
                m_uiFireBloomTarget[i] = target->GetGUID();
                m_uiFireBloomCount = 0;
                //DoCast(target, SPELL_FIREBLOOM, true);
            }
            m_uiFireBloomTimer = 25000;
        }else m_uiFireBloomTimer -= diff;

        if(m_uiSoulFlyTimer < diff)
        {
            DoCast(m_creature->getVictim(), SPELL_SOULFLY);
            m_uiSoulFlyTimer = 7000;
        }else m_uiSoulFlyTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

struct MANGOS_DLL_DECL mob_deceiverAI : public ScriptedAI
{
    mob_deceiverAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        Reset();
    }
 
    ScriptedInstance* pInstance;

    uint32 m_uiBoltTimer;
    uint32 m_uiPortalTimer;
    uint32 m_uiImpTimer;
    uint8 m_uiImpCount;
    float m_fxx;
    float m_fyy;
 
    void Reset()
    {
        m_uiPortalTimer = 25000;
        m_uiBoltTimer = 10000;
        m_uiImpTimer = 60000;
        m_uiImpCount = 0;
        m_fxx = 0;
        m_fyy = 0;

        if(pInstance)
            pInstance->SetData(DATA_DECIVER, NOT_STARTED);
    }
    
    void Aggro(Unit *who) 
    {
        if (pInstance)
            pInstance->SetData(DATA_KILJAEDEN_EVENT, IN_PROGRESS);

        if(!m_creature->getVictim())
            m_creature->AI()->AttackStart(who);
    }

    void JustDied(Unit* Killer) 
    {
        // Odpocitavanie
        if(pInstance && pInstance->GetData(DATA_DECIVER) == NOT_STARTED)
            pInstance->SetData(DATA_DECIVER, IN_PROGRESS);
        else 
            if(pInstance && pInstance->GetData(DATA_DECIVER) == IN_PROGRESS)
                pInstance->SetData(DATA_DECIVER, DONE);
            else
                if(pInstance && pInstance->GetData(DATA_DECIVER) == DONE)
                {
                    pInstance->SetData(DATA_DECIVER, SPECIAL);

                    // Vypnutie sunwell efektu u controllera
                    if (Creature* pController = pInstance->instance->GetCreature(pInstance->GetData64(DATA_KILJAEDEN_CONTROLLER)))
                        pController->RemoveAurasDueToSpell(46410);
                }
    }

    void KilledUnit(Unit *Victim) {}
     
    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
        {
            // Channelovanie
            if (!m_creature->IsNonMeleeSpellCasted(false) && !m_creature->IsInEvadeMode())
                if (Creature* pController = pInstance->instance->GetCreature(pInstance->GetData64(DATA_KILJAEDEN_CONTROLLER)))
                        m_creature->CastSpell(pController, SPELL_SHADOW_CHANNELING, false);

            return;
        }

        if(m_uiBoltTimer < diff)
        {
            DoCast(m_creature->getVictim(), SPELL_SHADOWBOLT_VOLLEY);
            m_uiBoltTimer = 10000;
        }else m_uiBoltTimer -= diff;

        if((m_creature->GetHealth()*100 / m_creature->GetMaxHealth()) <= 20)
            if(!m_creature->HasAura(SPELL_SHADOWINFUSION))
                m_creature->CastSpell(m_creature, SPELL_SHADOWINFUSION, true);

        if(m_uiPortalTimer < diff)
        {
            m_fxx = m_creature->GetPositionX();
            m_fyy = m_creature->GetPositionY();

            Creature* Portal = m_creature->SummonCreature(ID_PORTAL, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 20500);
            m_uiPortalTimer = 30000;
            m_uiImpTimer = 500;
            m_uiImpCount = 0;
        }else m_uiPortalTimer -= diff;

        if(m_uiImpTimer < diff)
        {
            if(m_uiImpCount < 4)
            {
                Creature* Imp = m_creature->SummonCreature(ID_IMP, m_fxx, m_fyy, m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 60000);
                if(Imp)
                    if(Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0))
                        Imp->AI()->AttackStart(target);  
            }
            m_uiImpTimer = 5000;
            ++m_uiImpCount;
        }else m_uiImpTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

//Shield Orb AI
struct MANGOS_DLL_DECL mob_orbAI : public Scripted_NoMovementAI
{
    mob_orbAI(Creature *c) : Scripted_NoMovementAI(c)
    {
        Reset();
    }
 
    uint32 m_uiSpellTimer;
 
    void Reset()
    {
        m_uiSpellTimer = 1000;
        m_creature->setFaction(14);
        m_creature->SetVisibility(VISIBILITY_ON);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void DamageDeal(Unit* pDoneTo, uint32& uiDamage) 
    {
        if(pDoneTo->HasAura(AURA_BLUESHIELD))
        {
            uiDamage = uiDamage * 0.05;
        } else {
            if(Creature* dracek = GetClosestCreatureWithEntry(pDoneTo,25653,150.0f))
            {
                if(dracek->HasAura(45848))
                    uiDamage = uiDamage * 0.05;
            }
            else
            {
                error_log("KJ mob_orb::DamageDeal : Dracek not found.");
            }
        }
    }

    void DamageTaken(Unit* pDoneBy, uint32& uiDamage) 
    {
        if(pDoneBy->HasAura(AURA_BLUESHIELD))
        {
            uiDamage = uiDamage * 0.05;
        } else {
            if(Creature* dracek = GetClosestCreatureWithEntry(pDoneBy,25653,150.0f))
            {
                if(dracek->HasAura(45848))
                    uiDamage = uiDamage * 0.05;
            }
            else
            {
                error_log("KJ mob_orb::DamageTaken : Dracek not found.");
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
                return;

        if(m_uiSpellTimer < diff)
        {
            for(uint8 i=0; i<3; ++i)
            {
                if(Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM,0))
                    m_creature->CastSpell(target, SPELL_ORB_BOLT, true);
            }
            m_uiSpellTimer = 3000;
        }else m_uiSpellTimer -= diff;
    }
};


//AI for Armageddon target
struct MANGOS_DLL_DECL mob_armagedonAI : public Scripted_NoMovementAI
{
    mob_armagedonAI(Creature* c) : Scripted_NoMovementAI(c) {}

    uint8 spell;
    uint32 uiTimer;

    void Reset()
    {
        spell = 0;
        uiTimer = 0;
    }

    void UpdateAI(const uint32 diff)
    {
        if (uiTimer <= diff)
        {
            switch(spell)
            {
                case 0:
                    DoCast(m_creature, SPELL_ARMAGEDDON_VISUAL, true);
                    ++spell;
                    break;
                case 1:
                    DoCast(m_creature, SPELL_ARMAGEDDON_VISUAL2, true);
                    uiTimer = 9000;
                    ++spell;
                    break;
                case 2:
                    DoCast(m_creature, SPELL_ARMAGEDDON_TRIGGER, true);
                    ++spell;
                    uiTimer = 5000;
                    break;
                case 3:
                    m_creature->DealDamage(m_creature,m_creature->GetHealth(),0,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,0,false);
                    m_creature->RemoveCorpse();
                    break;
            }
        } else uiTimer -=diff;
    }
};

struct MANGOS_DLL_DECL mob_killimpAI : public ScriptedAI
{
    mob_killimpAI(Creature *c) : ScriptedAI(c) 
    {
        Reset(); 
    }

    bool m_bIsReached;
    uint32 m_uiVisibilityTimer;

    void Reset() 
    {
        m_bIsReached = false;
    }

    void JustDied(Unit* Killer) 
    {
        if(!m_bIsReached)
        {
            m_creature->CastSpell(m_creature->getVictim(), SPELL_IMPFUSION, true);
            m_bIsReached = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        //Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
                return;

        if( m_creature->isAttackReady() && !m_creature->IsNonMeleeSpellCasted(false))
        {
            if(m_bIsReached)
                if(m_uiVisibilityTimer < diff)
                {
                    m_creature->SetVisibility(VISIBILITY_OFF);
                    m_uiVisibilityTimer = 60000;
                }else m_uiVisibilityTimer -= diff;

            //If we are within range melee the target
            if(!m_bIsReached)
                if(m_creature->IsWithinDistInMap(m_creature->getVictim(), 3.0f))
                {
                    m_creature->CastSpell(m_creature->getVictim(), SPELL_IMPFUSION, true);
                    m_uiVisibilityTimer = 2000;
                    m_bIsReached = true;
                }
        }
    }
};          

// Kiljaeden Controller Event AI
struct MANGOS_DLL_DECL mob_kiljaeden_controllerAI : public Scripted_NoMovementAI
{
    mob_kiljaeden_controllerAI(Creature *c) : Scripted_NoMovementAI(c) 
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        postevent = false;
        Reset(); 
    }
    ScriptedInstance* pInstance; 

    uint32 m_uiProphetsTimer;
    uint32 m_uiProphetsCount;
    bool m_bIsOutroEnd;
    bool m_bIsProhetSpawned;
    bool m_bIsOnce;
    bool postevent;

    void Reset() 
    {
        if(postevent) return;
        m_bIsProhetSpawned = true;
        m_bIsOutroEnd = true;
        m_creature->setFaction(14);
        m_uiProphetsCount = 10;
        m_bIsOnce = true;

        if(pInstance)
            pInstance->SetData(DATA_DECIVER, NOT_STARTED);

        m_creature->CastSpell(m_creature,46410,false);
    }

    void Aggro(Unit *who)
    { 
        m_creature->SetInCombatWithZone();

        /*for(uint8 i=0; i<3; ++i)
        {
            Creature* Deveiver = m_creature->SummonCreature(ID_DECIVER, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 10000);
                if(Deveiver)
                    if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
                        Deveiver->AI()->AttackStart(target);
        }*/
    }

    void UpdateAI(const uint32 diff)
    {
        // if(pInstance && pInstance->GetData(DATA_KILJAEDEN_EVENT) == IN_PROGRESS)
        // {
        //     m_creature->RemoveAurasDueToSpell(46410);
        //     m_creature->SetVisibility(VISIBILITY_OFF);
        // }

        //Return since we have no target
        //if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
                //return;

        if(pInstance && pInstance->GetData(DATA_KILJAEDEN_EVENT) == DONE && m_bIsOnce)
        {
            m_bIsProhetSpawned = false;
            m_uiProphetsCount = 0;
            m_bIsOutroEnd = false;
            m_uiProphetsTimer = 20000;
            m_bIsOnce = false;
            postevent = true;
        }

        if(!m_bIsProhetSpawned)
        {
            m_creature->SummonCreature(99925,0,0,0,0,TEMPSUMMON_DEAD_DESPAWN,0);
            m_creature->DealDamage(m_creature,m_creature->GetHealth(),0,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,0,false);
        }

        /*if((m_uiProphetsTimer < diff) && !m_bIsOutroEnd)
        {
            switch(m_uiProphetsCount)
            {
                case 0:
                DoPlaySoundToSet(m_creature, SAY_VELEN_01); m_uiProphetsTimer = 25000; break; //1
                case 1:
                DoPlaySoundToSet(m_creature, SAY_VELEN_02); m_uiProphetsTimer = 15000; break; //2
                case 2:
                DoPlaySoundToSet(m_creature, SAY_VELEN_03); m_uiProphetsTimer = 10000; break;  //2
                case 3:
                DoPlaySoundToSet(m_creature, SAY_VELEN_04); m_uiProphetsTimer = 26000; break; //3
                case 9:
                DoPlaySoundToSet(m_creature, SAY_LIADRIN_01); m_uiProphetsTimer = 20000; break; //9
                case 5:
                DoPlaySoundToSet(m_creature, SAY_VELEN_05); m_uiProphetsTimer = 16000;break; //5
                case 6:
                DoPlaySoundToSet(m_creature, SAY_LIADRIN_02); m_uiProphetsTimer = 3000; break; //6
                case 7:
                DoPlaySoundToSet(m_creature, SAY_VELEN_06); m_uiProphetsTimer = 17000; break; //7
                case 8:
                DoPlaySoundToSet(m_creature, SAY_VELEN_07); m_uiProphetsTimer = 16000; break; //8
                case 4:
                DoPlaySoundToSet(m_creature, SAY_LIADRIN_03); m_uiProphetsTimer = 11000; break; //4
                case 10:
                DoPlaySoundToSet(m_creature, SAY_VELEN_08); m_uiProphetsTimer = 7000; break; //10
                case 11:
                DoPlaySoundToSet(m_creature, SAY_VELEN_09); break; //11
                m_bIsOutroEnd = true; 
                m_creature->setFaction(35); 
                m_creature->DealDamage(m_creature,m_creature->GetHealth(),0,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,0,false);
            }
            ++m_uiProphetsCount;
        }m_uiProphetsTimer -= diff;*/
    }
};

// Kiljaeden post-event AI
struct MANGOS_DLL_DECL mob_kiljaeden_posteventAI : public Scripted_NoMovementAI
{
    mob_kiljaeden_posteventAI(Creature *c) : Scripted_NoMovementAI(c) 
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        Reset(); 
    }
    ScriptedInstance* pInstance; 

    uint32 m_uiProphetsTimer;
    uint32 m_uiProphetsCount;
    bool m_bIsOutroEnd;
    bool m_bIsProhetSpawned;
    bool m_bIsOnce;

    void Reset() 
    {
        m_bIsProhetSpawned = true;
        m_bIsOutroEnd = true;
        m_creature->setFaction(35);
        m_uiProphetsCount = 10;
        m_bIsOnce = true;

        if(pInstance)
            pInstance->SetData(DATA_DECIVER, NOT_STARTED);
    }

    void UpdateAI(const uint32 diff)
    {
        if(pInstance && pInstance->GetData(DATA_KILJAEDEN_EVENT) == DONE && m_bIsOnce)
        {
            m_bIsProhetSpawned = false;
            m_uiProphetsCount = 0;
            m_bIsOutroEnd = false;
            m_uiProphetsTimer = 20000;
            m_bIsOnce = false;
        }

        if(!m_bIsProhetSpawned)
        {
            m_creature->setFaction(35);
            m_creature->CombatStop(true);
            Creature* Velen = m_creature->SummonCreature(ID_VELEN, 1725.682007f,638.759277f,28.050213f,3.608646f, TEMPSUMMON_TIMED_DESPAWN, 360000);
            if(Velen)
            {
            Velen->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            Velen->setFaction(35);
            }
            Creature* Liadrin = m_creature->SummonCreature(ID_LIADRIN, 1727.728760f,631.062500f,28.050213f,3.250504f, TEMPSUMMON_TIMED_DESPAWN, 360000);
            if(Liadrin)
            {
            Liadrin->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            Liadrin->setFaction(35);
            }
            m_bIsProhetSpawned = true;
        }

        if((m_uiProphetsTimer < diff) && !m_bIsOutroEnd)
        {
            Creature* Velen = GetClosestCreatureWithEntry(m_creature,ID_VELEN,100.0f);
            Creature* Liadrin = GetClosestCreatureWithEntry(m_creature,ID_LIADRIN,100.0f);

            if(Velen && Liadrin)
            {
            switch(m_uiProphetsCount)
            {
                case 0:
                DoPlaySoundToSet(m_creature, SAY_VELEN_01); Velen->MonsterSay("Mortal Heroes, your victory here today was foretold, long ago. My brothers anquished cry of defeat will echo across the universe, bringing renewed hope to all those who still stand against The Burning Crusade.",LANG_UNIVERSAL,NULL); m_uiProphetsTimer = 25000; break; //1
                case 1:
                DoPlaySoundToSet(m_creature, SAY_VELEN_02); Velen->MonsterSay("As the Legion's final defeat draws ever-nearer, stand proud in the knowledge that you have saved worlds without number of the flame.",LANG_UNIVERSAL,NULL); m_uiProphetsTimer = 15000; break; //2
                case 2:
                DoPlaySoundToSet(m_creature, SAY_VELEN_03); Velen->MonsterSay("Just as this day marks an ending, so too does it herald a new beginnning...",LANG_UNIVERSAL,NULL); m_uiProphetsTimer = 10000; break;  //2
                case 3:
                DoPlaySoundToSet(m_creature, SAY_VELEN_04); Velen->MonsterSay("The creature Entropius, whom you were forced to destroy, was once a noble naruu, M'uru. In life, M'uru channeled vast energies of LIGHT and HOPE. For a time, a misguided few sought to steal those energies...",LANG_UNIVERSAL,NULL); m_uiProphetsTimer = 26000; break; //3
                case 9:
                DoPlaySoundToSet(m_creature, SAY_LIADRIN_01); Liadrin->MonsterSay("Blessed ancestors! I feel it... so much love... so much grace... there are... no words... impossible to describe...",LANG_UNIVERSAL,NULL); m_uiProphetsTimer = 20000; break; //9
                case 5:
                DoPlaySoundToSet(m_creature, SAY_VELEN_05); Velen->MonsterSay("Then fortunate it is, that I have reclaimed the noble naaru's spark spark from where it fell! Where faith dwells, hope is never lost, young blood elf.",LANG_UNIVERSAL,NULL); m_uiProphetsTimer = 16000;break; //5
                case 6:
                DoPlaySoundToSet(m_creature, SAY_LIADRIN_02); Liadrin->MonsterSay("Can it be?",LANG_UNIVERSAL,NULL); m_uiProphetsTimer = 3000; break; //6
                case 7:
                DoPlaySoundToSet(m_creature, SAY_VELEN_06); Velen->MonsterSay("Gaze now, mortals - upon the HEART OF M'URU! Unblemished. Bathed by the ligth of Creation - just as it was at the Dawn.",LANG_UNIVERSAL,NULL); m_uiProphetsTimer = 17000; break; //7
                case 8:
                DoPlaySoundToSet(m_creature, SAY_VELEN_07); /*m_creature->CastSpell(m_creature,46410,false);*/ m_creature->CastSpell(m_creature,46822,false); Velen->MonsterSay("In time, the light and hope held within - will rebirth more than this mere fount of power... Mayhap, they will rebirth the soul of a nation.",LANG_UNIVERSAL,NULL); m_uiProphetsTimer = 16000; break; //8
                case 4:
                DoPlaySoundToSet(m_creature, SAY_LIADRIN_03); Liadrin->MonsterSay("Our arrogance was unpardonable. We damned one of the most noble beings of all. We may never atone for this sin.",LANG_UNIVERSAL,NULL); m_uiProphetsTimer = 11000; break; //4
                case 10:
                DoPlaySoundToSet(m_creature, SAY_VELEN_08); Velen->MonsterSay("Salvation, young one. It waits for us all.",LANG_UNIVERSAL,NULL); m_uiProphetsTimer = 7000; break; //10
                case 11:
                DoPlaySoundToSet(m_creature, SAY_VELEN_09); Velen->MonsterSay("Farewell...",LANG_UNIVERSAL,NULL);
                m_bIsOutroEnd = true; 
                //m_creature->setFaction(35); 
                //m_creature->DealDamage(m_creature,m_creature->GetHealth(),0,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,0,false); 
                Velen->SetVisibility(VISIBILITY_OFF);
                Liadrin->SetVisibility(VISIBILITY_OFF);
                break; //11
            }
            }
            ++m_uiProphetsCount;
        }m_uiProphetsTimer -= diff;
    }
};

//Dragon Gossip Menu
//This function is called when the player opens the gossip menubool
bool GossipHello_dragon(Player* pPlayer, Creature* pCreature)
{
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
    //pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
    //pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
    //pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
    pPlayer->PlayerTalkClass->SendGossipMenu(907, pCreature->GetGUID());
    return true;
}

struct MANGOS_DLL_DECL dragon_timersAI: public ScriptedAI
{
    dragon_timersAI(Creature* c): ScriptedAI(c)
    {
        Reset();
    }

    uint32 bubbletimer;

    void Reset()
    {
        bubbletimer = 0;
    }

    void Aggro(Unit* pWho)
    {
        return;
    }

    void AttackStart(Unit* pWho)
    {
        return;
    }

    bool WillCast()
    {
        if(bubbletimer) return false;

        bubbletimer = 20000;
        return true;
    }

    void UpdateAI(const uint32 diff)
    {
        if(bubbletimer <= diff)
        {
            bubbletimer = 0;
        } else bubbletimer -= diff;

        if(bubbletimer > 20000)
            bubbletimer = 0;

        if(!m_creature->getVictim() || !m_creature->SelectHostileTarget())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_dragon_timersAI(Creature* pCreature)
{
    return new dragon_timersAI(pCreature);
}

bool GossipSelect_dragon(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiSender == GOSSIP_SENDER_MAIN)
    {
        switch (uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                if( ((dragon_timersAI*)pCreature->AI())->WillCast() )
                    pCreature->CastSpell(pPlayer, SPELL_SHIELD_OF_BLUE, false);
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            case GOSSIP_ACTION_INFO_DEF+2:
                pPlayer->CastSpell(pPlayer, SPELL_REVITALIZE, true);
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            case GOSSIP_ACTION_INFO_DEF+3:
                pPlayer->CastSpell(pPlayer, SPELL_HASTE, true);
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            case GOSSIP_ACTION_INFO_DEF+4:
                pPlayer->TeleportTo(pPlayer->GetMapId(), pPlayer->GetPositionX()+10, pPlayer->GetPositionY()+10, pPlayer->GetPositionZ(), pPlayer->GetOrientation());
                pPlayer->CLOSE_GOSSIP_MENU();
                return true;
            case GOSSIP_ACTION_INFO_DEF+5:
                pPlayer->CLOSE_GOSSIP_MENU();
        }
    }
    return true;
}

CreatureAI* GetAI_mob_killimp(Creature *_Creature)
{
    return new mob_killimpAI(_Creature);
}

CreatureAI* GetAI_mob_deceiver(Creature *_Creature)
{
    return new mob_deceiverAI(_Creature);
}

CreatureAI* GetAI_mob_armagedon(Creature *_Creature)
{
    return new mob_armagedonAI(_Creature);
}

CreatureAI* GetAI_boss_kiljaden(Creature *_Creature)
{
    return new boss_kiljadenAI(_Creature);
}

CreatureAI* GetAI_mob_orb(Creature *_Creature)
{
    return new mob_orbAI(_Creature);
}

CreatureAI* GetAI_mob_kiljaeden_controller(Creature *_Creature)
{
    return new mob_kiljaeden_controllerAI(_Creature);
}

CreatureAI* GetAI_mob_kiljaeden_postevent(Creature *_Creature)
{
    return new mob_kiljaeden_posteventAI(_Creature);
}

struct MANGOS_DLL_DECL mob_anveenaAI: public ScriptedAI
{
    mob_anveenaAI(Creature* c): ScriptedAI(c)
    {
        Reset();
    }

    uint8 event_phase;

    void Reset()
    {
        event_phase = 1;
        m_creature->GetMotionMaster()->MovePoint(0,1695.93f,628.289f,68.5714f);
        m_creature->CastSpell(m_creature,46367,false);
    }

    void UpdateAI(const uint32 diff)
    {
        if(event_phase == 1)
        {
            //outstring_log("Nemam auru");
            if(!m_creature->HasAura(46367))
            {
                m_creature->CastSpell(m_creature,46367,true);
                //outstring_log("Nahazuju to");
            } else ++event_phase;
        }
    }
};

CreatureAI* GetAI_mob_anveena(Creature *_Creature)
{
    return new mob_anveenaAI(_Creature);
}

struct MANGOS_DLL_DECL mob_kalecgos_kjAI: public ScriptedAI
{
    mob_kalecgos_kjAI(Creature* c): ScriptedAI(c)
    {
        Reset();
    }

    uint8 event_phase;
    Creature* kiljaeden;

    void Reset()
    {
        event_phase = 1;
        kiljaeden = NULL;
    }

    void UpdateAI(const uint32 diff)
    {
        if(m_creature->GetVisibility() == VISIBILITY_ON)
            m_creature->SetVisibility(VISIBILITY_OFF);

        if(event_phase == 1)
        {
            if(!kiljaeden)
            {
                kiljaeden = GetClosestCreatureWithEntry(m_creature,25315,100.0f);
            }
            if(kiljaeden && kiljaeden->isDead())
            {
                ++event_phase;
            }
            if(!kiljaeden)
            {
                m_creature->SetVisibility(VISIBILITY_OFF);
                m_creature->DealDamage(m_creature, m_creature->GetHealth(),0,DIRECT_DAMAGE,SPELL_SCHOOL_MASK_NORMAL,0,false);
            }
        }
    }
};

CreatureAI* GetAI_mob_kalecgos_kj(Creature *_Creature)
{
    return new mob_kalecgos_kjAI(_Creature);
}

void AddSC_boss_kiljaeden()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="mob_kiljaeden_controller";
    newscript->GetAI = &GetAI_mob_kiljaeden_controller;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_kiljaeden_postevent";
    newscript->GetAI = &GetAI_mob_kiljaeden_postevent;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_kiljaden";
    newscript->GetAI = &GetAI_boss_kiljaden;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_killimp";
    newscript->GetAI = &GetAI_mob_killimp;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_orb";
    newscript->GetAI = &GetAI_mob_orb;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_armagedon";
    newscript->GetAI = &GetAI_mob_armagedon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_deceiver";
    newscript->GetAI = &GetAI_mob_deceiver;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_anveena";
    newscript->GetAI = &GetAI_mob_anveena;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_kalecgos_kj";
    newscript->GetAI = &GetAI_mob_kalecgos_kj;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_dragon_kj";
    newscript->GetAI = GetAI_dragon_timersAI;
    newscript->pGossipHello = &GossipHello_dragon;
    newscript->pGossipSelect = &GossipSelect_dragon;
    newscript->RegisterSelf();
}
