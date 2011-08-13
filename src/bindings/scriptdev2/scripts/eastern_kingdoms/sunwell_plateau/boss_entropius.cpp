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
SDName: boss_entropius
SD%Complete:
SDComment:
SDCategory: Sunwell Plateau
EndScriptData */

#include "precompiled.h"
#include "sunwell_plateau.h"

struct MANGOS_DLL_DECL boss_entropiusAI : public ScriptedAI
{
    ScriptedInstance* m_pInstance;

    boss_entropiusAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();

        Reset();
    }

    void Reset()
    {
    }

    void Aggro(Unit* who)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_MURU, IN_PROGRESS);
    }

    void JustDied(Unit* killer)
    {
        m_pInstance->SetData(TYPE_MURU, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->getVictim() || !m_creature->SelectHostileTarget())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_entropius(Creature* pCreature)
{
    return new boss_entropiusAI(pCreature);
}

void AddSC_boss_entropius()
{
    Script* newscript;

    newscript = new Script;
    newscript->GetAI = &GetAI_boss_entropius;
    newscript->Name = "boss_entropius";
    newscript->RegisterSelf();
}
