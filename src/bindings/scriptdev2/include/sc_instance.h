/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_INSTANCE_H
#define SC_INSTANCE_H

#include "InstanceData.h"
#include "Map.h"

enum EncounterState
{
    NOT_STARTED   = 0,
    IN_PROGRESS   = 1,
    FAIL          = 2,
    DONE          = 3,
    SPECIAL       = 4
};

#define OUT_SAVE_INST_DATA             debug_log("SD2: Saving Instance Data for Instance %s (Map %d, Instance Id %d)", instance->GetMapName(), instance->GetId(), instance->GetInstanceId())
#define OUT_SAVE_INST_DATA_COMPLETE    debug_log("SD2: Saving Instance Data for Instance %s (Map %d, Instance Id %d) completed.", instance->GetMapName(), instance->GetId(), instance->GetInstanceId())
#define OUT_LOAD_INST_DATA(a)          debug_log("SD2: Loading Instance Data for Instance %s (Map %d, Instance Id %d). Input is '%s'", instance->GetMapName(), instance->GetId(), instance->GetInstanceId(), a)
#define OUT_LOAD_INST_DATA_COMPLETE    debug_log("SD2: Instance Data Load for Instance %s (Map %d, Instance Id: %d) is complete.",instance->GetMapName(), instance->GetId(), instance->GetInstanceId())
#define OUT_LOAD_INST_DATA_FAIL        error_log("SD2: Unable to load Instance Data for Instance %s (Map %d, Instance Id: %d).",instance->GetMapName(), instance->GetId(), instance->GetInstanceId())

class MANGOS_DLL_DECL ScriptedInstance : public InstanceData
{
    public:

        ScriptedInstance(Map* pMap) : InstanceData(pMap) {}
        ~ScriptedInstance() {}

        // Change active state of doors or buttons
        void DoUseDoorOrButton(uint64 uiGuid, uint32 uiWithRestoreTime = 0, bool bUseAlternativeState = false);

        // Respawns a GO having negative spawntimesecs in gameobject-table
        void DoRespawnGameObject(uint64 uiGuid, uint32 uiTimeToDespawn = MINUTE);

        // Sends world state update to all players in instance
        void DoUpdateWorldState(uint32 uiStateId, uint32 uiStateData);

        // Get a Player from map
        Player* GetPlayerInMap(bool bOnlyAlive = false, bool bCanBeGamemaster = true);
};

/// A static const array of this structure must be handled to DialogueHelper
struct DialogueEntry
{
    int32 iTextEntry;                                       ///< To be said text entry
    uint32 uiSayerEntry;                                    ///< Entry of the mob who should say
    uint32 uiTimer;                                         ///< Time delay until next text of array is said (0 stops)
};

/// A static const array of this structure must be handled to DialogueHelper
struct DialogueEntryTwoSide
{
    int32 iTextEntry;                                       ///< To be said text entry (first side)
    uint32 uiSayerEntry;                                    ///< Entry of the mob who should say (first side)
    int32 iTextEntryAlt;                                    ///< To be said text entry (second side)
    uint32 uiSayerEntryAlt;                                 ///< Entry of the mob who should say (second side)
    uint32 uiTimer;                                         ///< Time delay until next text of array is said (0 stops)
};

/// Helper class handling a dialogue given as static const array of DialogueEntry or DialogueEntryTwoSide
class DialogueHelper
{
    public:
        // The array MUST be terminated by {0,0,0}
        DialogueHelper(DialogueEntry const* pDialogueArray);
        // The array MUST be terminated by {0,0,0,0,0}
        DialogueHelper(DialogueEntryTwoSide const* aDialogueTwoSide);

        /// Function that MUST be called
        void InitializeDialogueHelper(InstanceData* pInstance) { m_pInstance = pInstance; }
        /// Set if take first entries or second entries
        void SetDialogueSide(bool bIsFirstSide) { m_bIsFirstSide = bIsFirstSide; }

        void StartNextDialogueText(int32 iTextEntry);

        void DialogueUpdate(uint32 uiDiff);

    protected:
        virtual void JustDidDialogueStep(int32 iEntry) {}

    private:
        void DoNextDialogueStep();

        InstanceData* m_pInstance;

        DialogueEntry const* m_pDialogueArray;
        DialogueEntry const* m_pCurrentEntry;
        DialogueEntryTwoSide const* m_pDialogueTwoSideArray;
        DialogueEntryTwoSide const* m_pCurrentEntryTwoSide;

        uint32 m_uiTimer;
        bool m_bIsFirstSide;
};

#endif
