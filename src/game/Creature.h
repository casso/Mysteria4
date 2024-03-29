/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
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

#ifndef MANGOSSERVER_CREATURE_H
#define MANGOSSERVER_CREATURE_H

#include "Common.h"
#include "Unit.h"
#include "UpdateMask.h"
#include "ItemPrototype.h"
#include "SharedDefines.h"
#include "LootMgr.h"
#include "DBCEnums.h"
#include "Database/DatabaseEnv.h"
#include "Cell.h"

#include <list>

struct SpellEntry;

class CreatureAI;
class Group;
class Quest;
class Player;
class WorldSession;

struct GameEventCreatureData;

enum CreatureFlagsExtra
{
    CREATURE_FLAG_EXTRA_INSTANCE_BIND   = 0x00000001,       // creature kill bind instance with killer and killer's group
    CREATURE_FLAG_EXTRA_CIVILIAN        = 0x00000002,       // not aggro (ignore faction/reputation hostility)
    CREATURE_FLAG_EXTRA_NO_PARRY        = 0x00000004,       // creature can't parry
    CREATURE_FLAG_EXTRA_NO_PARRY_HASTEN = 0x00000008,       // creature can't counter-attack at parry
    CREATURE_FLAG_EXTRA_NO_BLOCK        = 0x00000010,       // creature can't block
    CREATURE_FLAG_EXTRA_NO_CRUSH        = 0x00000020,       // creature can't do crush attacks
    CREATURE_FLAG_EXTRA_NO_XP_AT_KILL   = 0x00000040,       // creature kill not provide XP
    CREATURE_FLAG_EXTRA_INVISIBLE       = 0x00000080,       // creature is always invisible for player (mostly trigger creatures)
    CREATURE_FLAG_EXTRA_NOT_TAUNTABLE   = 0x00000100,       // creature is immune to taunt auras and effect attack me
    CREATURE_FLAG_EXTRA_AGGRO_ZONE      = 0x00000200,       // creature sets itself in combat with zone on aggro
};

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

#define MAX_KILL_CREDIT 2
#define MAX_CREATURE_MODEL 4

// from `creature_template` table
struct CreatureInfo
{
    uint32  Entry;
    uint32  HeroicEntry;
    uint32  KillCredit[MAX_KILL_CREDIT];
    uint32  ModelId[MAX_CREATURE_MODEL];
    char*   Name;
    char*   SubName;
    char*   IconName;
    uint32  GossipMenuId;
    uint32  minlevel;
    uint32  maxlevel;
    uint32  minhealth;
    uint32  maxhealth;
    uint32  minmana;
    uint32  maxmana;
    uint32  armor;
    uint32  faction_A;
    uint32  faction_H;
    uint32  npcflag;
    float   speed_walk;
    float   speed_run;
    float   scale;
    uint32  rank;
    float   mindmg;
    float   maxdmg;
    uint32  dmgschool;
    uint32  attackpower;
    float   dmg_multiplier;
    uint32  baseattacktime;
    uint32  rangeattacktime;
    uint32  unit_class;                                     // enum Classes. Note only 4 classes are known for creatures.
    uint32  unit_flags;                                     // enum UnitFlags mask values
    uint32  dynamicflags;
    uint32  family;                                         // enum CreatureFamily values (optional)
    uint32  trainer_type;
    uint32  trainer_spell;
    uint32  trainer_class;
    uint32  trainer_race;
    float   minrangedmg;
    float   maxrangedmg;
    uint32  rangedattackpower;
    uint32  type;                                           // enum CreatureType values
    uint32  type_flags;                                     // enum CreatureTypeFlags mask values
    uint32  lootid;
    uint32  pickpocketLootId;
    uint32  SkinLootId;
    int32   resistance1;
    int32   resistance2;
    int32   resistance3;
    int32   resistance4;
    int32   resistance5;
    int32   resistance6;
    uint32  spells[CREATURE_MAX_SPELLS];
    uint32  PetSpellDataId;
    uint32  mingold;
    uint32  maxgold;
    char const* AIName;
    uint32  MovementType;
    uint32  InhabitType;
    float   unk16;
    float   unk17;
    bool    RacialLeader;
    bool    RegenHealth;
    uint32  equipmentId;
    uint32  trainerId;
    uint32  vendorId;
    uint32  MechanicImmuneMask;
    uint32  flags_extra;
    uint32  ScriptID;

    // helpers
    SkillType GetRequiredLootSkill() const
    {
        if(type_flags & CREATURE_TYPEFLAGS_HERBLOOT)
            return SKILL_HERBALISM;
        else if(type_flags & CREATURE_TYPEFLAGS_MININGLOOT)
            return SKILL_MINING;
        else
            return SKILL_SKINNING;                          // normal case
    }

    bool isTameable() const
    {
        return type == CREATURE_TYPE_BEAST && family != 0 && (type_flags & CREATURE_TYPEFLAGS_TAMEABLE);
    }
};

struct EquipmentInfo
{
    uint32  entry;
    uint32  equipmodel[3];
    uint32  equipinfo[3];
    uint32  equipslot[3];
};

// from `creature` table
struct CreatureData
{
    uint32 id;                                              // entry in creature_template
    uint16 mapid;
    uint32 modelid_override;                                // overrides any model defined in creature_template
    int32 equipmentId;
    float posX;
    float posY;
    float posZ;
    float orientation;
    uint32 spawntimesecs;
    float spawndist;
    uint32 currentwaypoint;
    uint32 curhealth;
    uint32 curmana;
    bool  is_dead;
    uint8 movementType;
    uint8 spawnMask;
};

struct CreatureDataAddonAura
{
    uint32 spell_id;
    SpellEffectIndex effect_idx;
};

// from `creature_addon` table
struct CreatureDataAddon
{
    uint32 guidOrEntry;
    uint32 mount;
    uint32 bytes1;
    uint32 bytes2;
    uint32 emote;
    uint32 move_flags;
    CreatureDataAddonAura const* auras;                     // loaded as char* "spell1 eff1 spell2 eff2 ... "
};

struct CreatureModelInfo
{
    uint32 modelid;
    float bounding_radius;
    float combat_reach;
    uint8 gender;
    uint32 modelid_other_gender;                            // The oposite gender for this modelid (male/female)
    uint32 modelid_alternative;                             // An alternative model. Generally same gender(2)
};

struct CreatureModelRace
{
    uint32 modelid;                                         // Native model/base model the selection is for
    uint32 racemask;                                        // Races it applies to (and then a player source must exist for selection)
    uint32 creature_entry;                                  // Modelid from creature_template.entry will be selected
    uint32 modelid_racial;                                  // Explicit modelid. Used if creature_template entry is not defined
};

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some platform
#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

struct CreatureLocale
{
    std::vector<std::string> Name;
    std::vector<std::string> SubName;
};

struct GossipMenuItemsLocale
{
    std::vector<std::string> OptionText;
    std::vector<std::string> BoxText;
};

struct PointOfInterestLocale
{
    std::vector<std::string> IconName;
};

enum InhabitTypeValues
{
    INHABIT_GROUND = 1,
    INHABIT_WATER  = 2,
    INHABIT_AIR    = 4,
    INHABIT_ANYWHERE = INHABIT_GROUND | INHABIT_WATER | INHABIT_AIR
};

// Enums used by StringTextData::Type (CreatureEventAI)
enum ChatType
{
    CHAT_TYPE_SAY               = 0,
    CHAT_TYPE_YELL              = 1,
    CHAT_TYPE_TEXT_EMOTE        = 2,
    CHAT_TYPE_BOSS_EMOTE        = 3,
    CHAT_TYPE_WHISPER           = 4,
    CHAT_TYPE_BOSS_WHISPER      = 5,
    CHAT_TYPE_ZONE_YELL         = 6
};

// Selection method used by SelectAttackingTarget
enum AttackingTarget
{
    ATTACKING_TARGET_RANDOM = 0,                            //Just selects a random target
    ATTACKING_TARGET_TOPAGGRO,                              //Selects targes from top aggro to bottom
    ATTACKING_TARGET_BOTTOMAGGRO,                           //Selects targets from bottom aggro to top
    /* not implemented
    ATTACKING_TARGET_RANDOM_PLAYER,                         //Just selects a random target (player only)
    ATTACKING_TARGET_TOPAGGRO_PLAYER,                       //Selects targes from top aggro to bottom (player only)
    ATTACKING_TARGET_BOTTOMAGGRO_PLAYER,                    //Selects targets from bottom aggro to top (player only)
    */
};

// Vendors
struct VendorItem
{
    VendorItem(uint32 _item, uint32 _maxcount, uint32 _incrtime, uint32 _ExtendedCost)
        : item(_item), maxcount(_maxcount), incrtime(_incrtime), ExtendedCost(_ExtendedCost) {}

    uint32 item;
    uint32 maxcount;                                        // 0 for infinity item amount
    uint32 incrtime;                                        // time for restore items amount if maxcount != 0
    uint32 ExtendedCost;                                    // index in ItemExtendedCost.dbc
};
typedef std::vector<VendorItem*> VendorItemList;

struct VendorItemData
{
    VendorItemList m_items;

    VendorItem* GetItem(uint32 slot) const
    {
        if(slot>=m_items.size()) return NULL;
        return m_items[slot];
    }
    bool Empty() const { return m_items.empty(); }
    uint8 GetItemCount() const { return m_items.size(); }
    void AddItem( uint32 item, uint32 maxcount, uint32 ptime, uint32 ExtendedCost)
    {
        m_items.push_back(new VendorItem(item, maxcount, ptime, ExtendedCost));
    }
    bool RemoveItem( uint32 item_id );
    VendorItem const* FindItem(uint32 item_id) const;
    size_t FindItemSlot(uint32 item_id) const;

    void Clear()
    {
        for (VendorItemList::const_iterator itr = m_items.begin(); itr != m_items.end(); ++itr)
            delete (*itr);
        m_items.clear();
    }
};

struct VendorItemCount
{
    explicit VendorItemCount(uint32 _item, uint32 _count)
        : itemId(_item), count(_count), lastIncrementTime(time(NULL)) {}

    uint32 itemId;
    uint32 count;
    time_t lastIncrementTime;
};

typedef std::list<VendorItemCount> VendorItemCounts;

struct TrainerSpell
{
    TrainerSpell() : spell(0), spellCost(0), reqSkill(0), reqSkillValue(0), reqLevel(0) {}

    TrainerSpell(uint32 _spell, uint32 _spellCost, uint32 _reqSkill, uint32 _reqSkillValue, uint32 _reqLevel)
        : spell(_spell), spellCost(_spellCost), reqSkill(_reqSkill), reqSkillValue(_reqSkillValue), reqLevel(_reqLevel)
    {}

    uint32 spell;
    uint32 spellCost;
    uint32 reqSkill;
    uint32 reqSkillValue;
    uint32 reqLevel;
};

typedef UNORDERED_MAP<uint32 /*spellid*/, TrainerSpell> TrainerSpellMap;

struct TrainerSpellData
{
    TrainerSpellData() : trainerType(0) {}

    TrainerSpellMap spellList;
    uint32 trainerType;                                     // trainer type based at trainer spells, can be different from creature_template value.
                                                            // req. for correct show non-prof. trainers like weaponmaster, allowed values 0 and 2.
    TrainerSpell const* Find(uint32 spell_id) const;
    void Clear() { spellList.clear(); }
};

typedef std::map<uint32,time_t> CreatureSpellCooldowns;

// max different by z coordinate for creature aggro reaction
#define CREATURE_Z_ATTACK_RANGE 3

#define MAX_VENDOR_ITEMS 255                                // Limitation in item count field size in SMSG_LIST_INVENTORY

enum CreatureSubtype
{
    CREATURE_SUBTYPE_GENERIC,                               // new Creature
    CREATURE_SUBTYPE_PET,                                   // new Pet
    CREATURE_SUBTYPE_TOTEM,                                 // new Totem
    CREATURE_SUBTYPE_TEMPORARY_SUMMON,                      // new TemporarySummon
};

class MANGOS_DLL_SPEC Creature : public Unit
{
    CreatureAI *i_AI;

    public:

        explicit Creature(CreatureSubtype subtype = CREATURE_SUBTYPE_GENERIC);
        virtual ~Creature();

        void AddToWorld();
        void RemoveFromWorld();

        bool Create(uint32 guidlow, Map *map, uint32 Entry, Team team = TEAM_NONE, const CreatureData *data = NULL, GameEventCreatureData const* eventData = NULL);
        bool LoadCreatureAddon(bool reload = false);
        void SelectLevel(const CreatureInfo *cinfo, float percentHealth = 100.0f, float percentMana = 100.0f);
        void LoadEquipment(uint32 equip_entry, bool force=false);

        bool HasStaticDBSpawnData() const;                  // listed in `creature` table and have fixed in DB guid

        char const* GetSubName() const { return GetCreatureInfo()->SubName; }

        void Update(uint32 update_diff, uint32 time);                           // overwrite Unit::Update
        void GetRespawnCoord(float &x, float &y, float &z, float* ori = NULL, float* dist =NULL) const;
        uint32 GetEquipmentId() const { return m_equipmentId; }

        CreatureSubtype GetSubtype() const { return m_subtype; }
        bool IsPet() const { return m_subtype == CREATURE_SUBTYPE_PET; }
        bool IsTotem() const { return m_subtype == CREATURE_SUBTYPE_TOTEM; }
        bool IsTemporarySummon() const { return m_subtype == CREATURE_SUBTYPE_TEMPORARY_SUMMON; }

        bool IsCorpse() const { return getDeathState() ==  CORPSE; }
        bool IsDespawned() const { return getDeathState() ==  DEAD; }
        void SetCorpseDelay(uint32 delay) { m_corpseDelay = delay; }
        bool IsRacialLeader() const { return GetCreatureInfo()->RacialLeader; }
        bool IsCivilian() const { return GetCreatureInfo()->flags_extra & CREATURE_FLAG_EXTRA_CIVILIAN; }
        bool CanWalk() const { return GetCreatureInfo()->InhabitType & INHABIT_GROUND; }
        bool CanSwim() const { return GetCreatureInfo()->InhabitType & INHABIT_WATER; }
        bool CanFly()  const { return GetCreatureInfo()->InhabitType & INHABIT_AIR; }

        bool IsTrainerOf(Player* player, bool msg) const;
        bool CanInteractWithBattleMaster(Player* player, bool msg) const;
        bool CanTrainAndResetTalentsOf(Player* pPlayer) const;

        bool IsOutOfThreatArea(Unit* pVictim) const;
        void FillGuidsListFromThreatList(std::vector<ObjectGuid>& guids, uint32 maxamount = 0);

        bool IsImmuneToSpell(SpellEntry const* spellInfo);
                                                            // redefine Unit::IsImmuneToSpell
        bool IsImmuneToSpellEffect(SpellEntry const* spellInfo, SpellEffectIndex index) const;
                                                            // redefine Unit::IsImmuneToSpellEffect
        bool IsElite() const
        {
            if(IsPet())
                return false;

            uint32 rank = GetCreatureInfo()->rank;
            return rank != CREATURE_ELITE_NORMAL && rank != CREATURE_ELITE_RARE;
        }

        bool IsWorldBoss() const
        {
            if(IsPet())
                return false;

            return GetCreatureInfo()->rank == CREATURE_ELITE_WORLDBOSS;
        }

        uint32 GetLevelForTarget(Unit const* target) const; // overwrite Unit::GetLevelForTarget for boss level support

        bool IsInEvadeMode() const;

        bool AIM_Initialize();

        CreatureAI* AI() { return i_AI; }

        void AddSplineFlag(SplineFlags f)
        {
            bool need_walk_sync = (f & SPLINEFLAG_WALKMODE) != (m_splineFlags & SPLINEFLAG_WALKMODE);
            m_splineFlags = SplineFlags(m_splineFlags | f);
            if (need_walk_sync)
                UpdateWalkMode(this, false);
        }
        void RemoveSplineFlag(SplineFlags f)
        {
            bool need_walk_sync = (f & SPLINEFLAG_WALKMODE) != (m_splineFlags & SPLINEFLAG_WALKMODE);
            m_splineFlags = SplineFlags(m_splineFlags & ~f);
            if (need_walk_sync)
                UpdateWalkMode(this, false);
        }
        bool HasSplineFlag(SplineFlags f) const { return m_splineFlags & f; }
        SplineFlags GetSplineFlags() const { return m_splineFlags; }
        void SetSplineFlags(SplineFlags f)
        {
            bool need_walk_sync = (f & SPLINEFLAG_WALKMODE) != (m_splineFlags & SPLINEFLAG_WALKMODE);
            m_splineFlags = f;                              // need set before
            if (need_walk_sync)
                UpdateWalkMode(this, false);
        }

        void SendMonsterMoveWithSpeedToCurrentDestination(Player* player = NULL);

        uint32 GetShieldBlockValue() const                  // dunno mob block value
        {
            return (getLevel() / 2 + uint32(GetStat(STAT_STRENGTH) / 20));
        }

        SpellSchoolMask GetMeleeDamageSchoolMask() const { return m_meleeDamageSchoolMask; }
        void SetMeleeDamageSchool(SpellSchools school) { m_meleeDamageSchoolMask = SpellSchoolMask(1 << school); }

        void _AddCreatureSpellCooldown(uint32 spell_id, time_t end_time);
        void _AddCreatureCategoryCooldown(uint32 category, time_t apply_time);
        void AddCreatureSpellCooldown(uint32 spellid);
        bool HasSpellCooldown(uint32 spell_id) const;
        bool HasCategoryCooldown(uint32 spell_id) const;

        bool HasSpell(uint32 spellID) const;

        bool UpdateEntry(uint32 entry, Team team = ALLIANCE, const CreatureData* data = NULL, GameEventCreatureData const* eventData = NULL, bool preserveHPAndPower = true);

        void ApplyGameEventSpells(GameEventCreatureData const* eventData, bool activated);
        bool UpdateStats(Stats stat);
        bool UpdateAllStats();
        void UpdateResistances(uint32 school);
        void UpdateArmor();
        void UpdateMaxHealth();
        void UpdateMaxPower(Powers power);
        void UpdateAttackPowerAndDamage(bool ranged = false);
        void UpdateDamagePhysical(WeaponAttackType attType);
        uint32 GetCurrentEquipmentId() { return m_equipmentId; }
        float GetSpellDamageMod(int32 Rank);

        VendorItemData const* GetVendorItems() const;
        VendorItemData const* GetVendorTemplateItems() const;
        uint32 GetVendorItemCurrentCount(VendorItem const* vItem);
        uint32 UpdateVendorItemCurrentCount(VendorItem const* vItem, uint32 used_count);

        TrainerSpellData const* GetTrainerTemplateSpells() const;
        TrainerSpellData const* GetTrainerSpells() const;

        CreatureInfo const *GetCreatureInfo() const { return m_creatureInfo; }
        CreatureDataAddon const* GetCreatureAddon() const;

        static uint32 ChooseDisplayId(const CreatureInfo *cinfo, const CreatureData *data = NULL, GameEventCreatureData const* eventData = NULL);

        std::string GetAIName() const;
        std::string GetScriptName() const;
        uint32 GetScriptId() const;

        // overwrite WorldObject function for proper name localization
        const char* GetNameForLocaleIdx(int32 locale_idx) const;

        void SetDeathState(DeathState s);                   // overwrite virtual Unit::SetDeathState
        bool FallGround();

        bool LoadFromDB(uint32 guid, Map *map);
        void SaveToDB();
                                                            // overwrited in Pet
        virtual void SaveToDB(uint32 mapid, uint8 spawnMask);
        virtual void DeleteFromDB();                        // overwrited in Pet

        Loot loot;
        bool lootForPickPocketed;
        bool lootForBody;
        bool lootForSkin;

        void PrepareBodyLootState();
        ObjectGuid GetLootRecipientGuid() const { return m_lootRecipientGuid; }
        uint32 GetLootGroupRecipientId() const { return m_lootGroupRecipientId; }
        Player* GetLootRecipient() const;                   // use group cases as prefered
        Group* GetGroupLootRecipient() const;
        bool HasLootRecipient() const { return m_lootGroupRecipientId || !m_lootRecipientGuid.IsEmpty(); }
        bool IsGroupLootRecipient() const { return m_lootGroupRecipientId; }
        void SetLootRecipient(Unit* unit);
        void AllLootRemovedFromCorpse();
        Player* GetOriginalLootRecipient() const;           // ignore group changes/etc, not for looting

        SpellEntry const *ReachWithSpellAttack(Unit *pVictim);
        SpellEntry const *ReachWithSpellCure(Unit *pVictim);

        uint32 m_spells[CREATURE_MAX_SPELLS];
        CreatureSpellCooldowns m_CreatureSpellCooldowns;
        CreatureSpellCooldowns m_CreatureCategoryCooldowns;

        float GetAttackDistance(Unit const* pl) const;

        void SendAIReaction(AiReaction reactionType);

        void DoFleeToGetAssistance();
        void CallForHelp(float fRadius);
        void CallAssistance();
        void SetNoCallAssistance(bool val) { m_AlreadyCallAssistance = val; }
        void SetNoSearchAssistance(bool val) { m_AlreadySearchedAssistance = val; }
        bool HasSearchedAssistance() { return m_AlreadySearchedAssistance; }
        bool CanAssistTo(const Unit* u, const Unit* enemy, bool checkfaction = true) const;
        bool CanInitiateAttack();

        MovementGeneratorType GetDefaultMovementType() const { return m_defaultMovementType; }
        void SetDefaultMovementType(MovementGeneratorType mgt) { m_defaultMovementType = mgt; }

        // for use only in LoadHelper, Map::Add Map::CreatureCellRelocation
        Cell const& GetCurrentCell() const { return m_currentCell; }
        void SetCurrentCell(Cell const& cell) { m_currentCell = cell; }

        bool IsVisibleInGridForPlayer(Player* pl) const;

        void RemoveCorpse();
        bool IsDeadByDefault() const { return m_isDeadByDefault; };

        void ForcedDespawn(uint32 timeMSToDespawn = 0);

        time_t const& GetRespawnTime() const { return m_respawnTime; }
        time_t GetRespawnTimeEx() const;
        void SetRespawnTime(uint32 respawn) { m_respawnTime = respawn ? time(NULL) + respawn : 0; }
        void Respawn();
        void SaveRespawnTime();

        uint32 GetRespawnDelay() const { return m_respawnDelay; }
        void SetRespawnDelay(uint32 delay) { m_respawnDelay = delay; }

        float GetRespawnRadius() const { return m_respawnradius; }
        void SetRespawnRadius(float dist) { m_respawnradius = dist; }

        // Functions spawn/remove creature with DB guid in all loaded map copies (if point grid loaded in map)
        static void AddToRemoveListInMaps(uint32 db_guid, CreatureData const* data);
        static void SpawnInMaps(uint32 db_guid, CreatureData const* data);

        void StartGroupLoot(Group* group, uint32 timer);

        void SendZoneUnderAttackMessage(Player* attacker);

        void SetInCombatWithZone();

        Unit* SelectAttackingTarget(AttackingTarget target, uint32 position) const;

        bool HasQuest(uint32 quest_id) const;
        bool HasInvolvedQuest(uint32 quest_id)  const;

        GridReference<Creature> &GetGridRef() { return m_gridRef; }
        bool IsRegeneratingHealth() { return m_regenHealth; }
        virtual uint8 GetPetAutoSpellSize() const { return CREATURE_MAX_SPELLS; }
        virtual uint32 GetPetAutoSpellOnPos(uint8 pos) const
        {
            if (pos >= CREATURE_MAX_SPELLS || m_charmInfo->GetCharmSpell(pos)->GetType() != ACT_ENABLED)
                return 0;
            else
                return m_charmInfo->GetCharmSpell(pos)->GetAction();
        }

        void SetCombatStartPosition(float x, float y, float z) { CombatStartX = x; CombatStartY = y; CombatStartZ = z; }
        void GetCombatStartPosition(float &x, float &y, float &z) { x = CombatStartX; y = CombatStartY; z = CombatStartZ; }

        void SetSummonPoint(float fX, float fY, float fZ, float fOrient) { m_summonXpoint = fX; m_summonYpoint = fY; m_summonZpoint = fZ; m_summonOrientation = fOrient; }
        void GetSummonPoint(float &fX, float &fY, float &fZ, float &fOrient) const { fX = m_summonXpoint; fY = m_summonYpoint; fZ = m_summonZpoint; fOrient = m_summonOrientation; }

        void SetDeadByDefault (bool death_state) { m_isDeadByDefault = death_state; }

        void SetActiveObjectState(bool on);

        void SetNeedNotify() { m_needNotify = true; }
        void SetLevitate(bool enable);

    protected:
        bool CreateFromProto(uint32 guidlow,uint32 Entry, Team team, const CreatureData *data = NULL, GameEventCreatureData const* eventData =NULL);
        bool InitEntry(uint32 entry, const CreatureData* data = NULL, GameEventCreatureData const* eventData = NULL);
        void RelocationNotify();

        uint32 m_groupLootTimer;                            // (msecs)timer used for group loot
        uint32 m_groupLootId;                               // used to find group which is looting corpse
        void StopGroupLoot();

        // vendor items
        VendorItemCounts m_vendorItemCounts;

        void _RealtimeSetCreatureInfo();

        static float _GetHealthMod(int32 Rank);
        static float _GetDamageMod(int32 Rank);

        uint32 m_lootMoney;
        ObjectGuid m_lootRecipientGuid;                     // player who will have rights for looting if m_lootGroupRecipient==0 or group disbanded
        uint32 m_lootGroupRecipientId;                      // group who will have rights for looting if set and exist

        /// Timers
        uint32 m_corpseDecayTimer;                          // (msecs)timer for death or corpse disappearance
        time_t m_respawnTime;                               // (secs) time of next respawn
        uint32 m_respawnDelay;                              // (secs) delay between corpse disappearance and respawning
        uint32 m_corpseDelay;                               // (secs) delay between death and corpse disappearance
        float m_respawnradius;

        CreatureSubtype m_subtype;                          // set in Creatures subclasses for fast it detect without dynamic_cast use
        void RegenerateMana();
        void RegenerateHealth();
        MovementGeneratorType m_defaultMovementType;
        Cell m_currentCell;                                 // store current cell where creature listed
        uint32 m_equipmentId;

        bool m_AlreadyCallAssistance;
        bool m_AlreadySearchedAssistance;
        bool m_regenHealth;
        bool m_AI_locked;
        bool m_isDeadByDefault;
        bool m_needNotify;

        SpellSchoolMask m_meleeDamageSchoolMask;
        uint32 m_originalEntry;

        float CombatStartX;
        float CombatStartY;
        float CombatStartZ;

        float m_summonXpoint;
        float m_summonYpoint;
        float m_summonZpoint;
        float m_summonOrientation;

    private:
        GridReference<Creature> m_gridRef;
        CreatureInfo const* m_creatureInfo;                 // in heroic mode can different from sObjectMgr::GetCreatureTemplate(GetEntry())
        SplineFlags m_splineFlags;
};

class AssistDelayEvent : public BasicEvent
{
    public:
        AssistDelayEvent(ObjectGuid victim, Unit& owner, std::list<Creature*> const& assistants);

        bool Execute(uint64 e_time, uint32 p_time);
    private:
        AssistDelayEvent();

        ObjectGuid              m_victimGuid;
        std::vector<ObjectGuid> m_assistantGuids;
        Unit&                   m_owner;
};

class ForcedDespawnDelayEvent : public BasicEvent
{
    public:
        ForcedDespawnDelayEvent(Creature& owner) : BasicEvent(), m_owner(owner) { }
        bool Execute(uint64 e_time, uint32 p_time);

    private:
        Creature& m_owner;
};

#endif
