/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: npc_hand_of_the_deceiver
SD%Complete:
SDComment: Deprecated
SDCategory: Sunwell Plateau
EndScriptData */

#include "precompiled.h"
#include "sunwell_plateau.h"

struct MANGOS_DLL_DECL npc_hand_of_the_deceiverAI : public ScriptedAI
{
    ScriptedInstance* m_pInstance;

    npc_hand_of_the_deceiverAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();

        Reset();
    }

    void Reset()
    {
    }

    void JustDied(Unit* killer)
    {
        m_pInstance->SetData(TYPE_HAND_OF_THE_DECEIVER, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->getVictim() || !m_creature->SelectHostileTarget())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_hand_of_the_deceiverAI(Creature* pCreature)
{
    return new npc_hand_of_the_deceiverAI(pCreature);
}

void AddSC_npc_hand_of_the_deceiverAI()
{
    Script* newscript;

    newscript = new Script;
    newscript->GetAI = &GetAI_npc_hand_of_the_deceiverAI;
    newscript->Name = "npc_hand_of_the_deceiver";
    newscript->RegisterSelf();
}
