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

#include "SpellMgr.h"
#include "ObjectMgr.h"
#include "SpellAuraDefines.h"
#include "ProgressBar.h"
#include "DBCStores.h"
#include "World.h"
#include "Chat.h"
#include "Spell.h"
#include "BattleGroundMgr.h"
#include "Language.h"

SpellMgr::SpellMgr()
{
}

SpellMgr::~SpellMgr()
{
}

SpellMgr& SpellMgr::Instance()
{
    static SpellMgr spellMgr;
    return spellMgr;
}

int32 GetSpellDuration(SpellEntry const *spellInfo)
{
    if(!spellInfo)
        return 0;
    SpellDurationEntry const *du = sSpellDurationStore.LookupEntry(spellInfo->DurationIndex);
    if(!du)
        return 0;
    return (du->Duration[0] == -1) ? -1 : abs(du->Duration[0]);
}

int32 GetSpellMaxDuration(SpellEntry const *spellInfo)
{
    if(!spellInfo)
        return 0;
    SpellDurationEntry const *du = sSpellDurationStore.LookupEntry(spellInfo->DurationIndex);
    if(!du)
        return 0;
    return (du->Duration[2] == -1) ? -1 : abs(du->Duration[2]);
}

uint32 GetSpellCastTime(SpellEntry const* spellInfo, Spell const* spell)
{
    if (spell)
    {
        // some triggered spells have data only usable for client
        if (spell->IsTriggeredSpellWithRedundentData())
            return 0;

        // spell targeted to non-trading trade slot item instant at trade success apply
        if (spell->GetCaster()->GetTypeId()==TYPEID_PLAYER)
            if (TradeData* my_trade = ((Player*)(spell->GetCaster()))->GetTradeData())
                if (Item* nonTrade = my_trade->GetTraderData()->GetItem(TRADE_SLOT_NONTRADED))
                    if (nonTrade == spell->m_targets.getItemTarget())
                        return 0;
    }

    SpellCastTimesEntry const *spellCastTimeEntry = sSpellCastTimesStore.LookupEntry(spellInfo->CastingTimeIndex);

    // not all spells have cast time index and this is all is pasiive abilities
    if (!spellCastTimeEntry)
        return 0;

    int32 castTime = spellCastTimeEntry->CastTime;

    if (spell)
    {
        if (Player* modOwner = spell->GetCaster()->GetSpellModOwner())
            modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_CASTING_TIME, castTime, spell);

        if (!(spellInfo->Attributes & (SPELL_ATTR_IS_ABILITY|SPELL_ATTR_TRADESPELL)))
            castTime = int32(castTime * spell->GetCaster()->GetFloatValue(UNIT_MOD_CAST_SPEED));
        else
        {
            if (spell->IsRangedSpell() && !spell->IsAutoRepeat())
                castTime = int32(castTime * spell->GetCaster()->m_modAttackSpeedPct[RANGED_ATTACK]);
        }
    }

    if (spellInfo->Attributes & SPELL_ATTR_RANGED && (!spell || !spell->IsAutoRepeat()))
        castTime += 500;

    return (castTime > 0) ? uint32(castTime) : 0;
}

uint16 GetSpellAuraMaxTicks(SpellEntry const* spellInfo)
{
    int32 DotDuration = GetSpellDuration(spellInfo);
    if(DotDuration == 0)
        return 1;

    // 200% limit
    if(DotDuration > 30000)
        DotDuration = 30000;

    for (int j = 0; j < MAX_EFFECT_INDEX; ++j)
    {
        if (spellInfo->Effect[j] == SPELL_EFFECT_APPLY_AURA && (
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_PERIODIC_DAMAGE ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_PERIODIC_HEAL ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_PERIODIC_LEECH) )
        {
            if (spellInfo->EffectAmplitude[j] != 0)
                return DotDuration / spellInfo->EffectAmplitude[j];
            break;
        }
    }

    return 6;
}

WeaponAttackType GetWeaponAttackType(SpellEntry const *spellInfo)
{
    if(!spellInfo)
        return BASE_ATTACK;

    switch (spellInfo->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
            if (spellInfo->AttributesEx3 & SPELL_ATTR_EX3_REQ_OFFHAND)
                return OFF_ATTACK;
            else
                return BASE_ATTACK;
            break;
        case SPELL_DAMAGE_CLASS_RANGED:
            return RANGED_ATTACK;
            break;
        default:
                                                            // Wands
            if (spellInfo->AttributesEx2 & SPELL_ATTR_EX2_AUTOREPEAT_FLAG)
                return RANGED_ATTACK;
            else
                return BASE_ATTACK;
            break;
    }
}

bool IsPassiveSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if (!spellInfo)
        return false;
    return IsPassiveSpell(spellInfo);
}

bool IsPassiveSpell(SpellEntry const *spellInfo)
{
    return (spellInfo->Attributes & SPELL_ATTR_PASSIVE) != 0;
}

bool IsNoStackAuraDueToAura(uint32 spellId_1, SpellEffectIndex effIndex_1, uint32 spellId_2, SpellEffectIndex effIndex_2)
{
    SpellEntry const *spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    if (spellInfo_1->Effect[effIndex_1] != spellInfo_2->Effect[effIndex_2] ||
        spellInfo_1->EffectItemType[effIndex_1] != spellInfo_2->EffectItemType[effIndex_2] ||
        spellInfo_1->EffectMiscValue[effIndex_1] != spellInfo_2->EffectMiscValue[effIndex_2] ||
        spellInfo_1->EffectApplyAuraName[effIndex_1] != spellInfo_2->EffectApplyAuraName[effIndex_2])
        return false;

    return true;
}

int32 CompareAuraRanks(uint32 spellId_1, SpellEffectIndex effIndex_1, uint32 spellId_2, SpellEffectIndex effIndex_2)
{
    SpellEntry const*spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry const*spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return 0;
    if (spellId_1 == spellId_2) return 0;

    int32 diff = spellInfo_1->EffectBasePoints[effIndex_1] - spellInfo_2->EffectBasePoints[effIndex_2];
    if (spellInfo_1->CalculateSimpleValue(effIndex_1) < 0 && spellInfo_2->CalculateSimpleValue(effIndex_2) < 0)
        return -diff;
    else return diff;
}

SpellSpecific GetSpellSpecific(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return SPELL_NORMAL;

    switch(spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            // Aspect of the Beast
            if (spellInfo->Id == 13161)
                return SPELL_ASPECT;

            // Food / Drinks (mostly)
            if (spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED)
            {
                bool food = false;
                bool drink = false;
                for(int i = 0; i < MAX_EFFECT_INDEX; ++i)
                {
                    switch(spellInfo->EffectApplyAuraName[i])
                    {
                        // Food
                        case SPELL_AURA_MOD_REGEN:
                        case SPELL_AURA_OBS_MOD_HEALTH:
                            food = true;
                            break;
                        // Drink
                        case SPELL_AURA_MOD_POWER_REGEN:
                        case SPELL_AURA_OBS_MOD_MANA:
                            drink = true;
                            break;
                        default:
                            break;
                    }
                }

                if (food && drink)
                    return SPELL_FOOD_AND_DRINK;
                else if (food)
                    return SPELL_FOOD;
                else if (drink)
                    return SPELL_DRINK;
            }
            else
            {
                // Well Fed buffs (must be exclusive with Food / Drink replenishment effects, or else Well Fed will cause them to be removed)
                // SpellIcon 2560 is Spell 46687, does not have this flag
                if ((spellInfo->AttributesEx2 & SPELL_ATTR_EX2_FOOD_BUFF) || spellInfo->SpellIconID == 2560)
                    return SPELL_WELL_FED;
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            // family flags 18(Molten), 25(Frost/Ice), 28(Mage)
            if (spellInfo->SpellFamilyFlags & UI64LIT(0x12040000))
                return SPELL_MAGE_ARMOR;

            if ((spellInfo->SpellFamilyFlags & UI64LIT(0x1000000)) && spellInfo->EffectApplyAuraName[EFFECT_INDEX_0] == SPELL_AURA_MOD_CONFUSE)
                return SPELL_MAGE_POLYMORPH;

            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            if (spellInfo->SpellFamilyFlags & UI64LIT(0x00008000010000))
                return SPELL_POSITIVE_SHOUT;

            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // only warlock curses have this
            if (spellInfo->Dispel == DISPEL_CURSE)
                return SPELL_CURSE;

            // family flag 37 (only part spells have family name)
            if (spellInfo->SpellFamilyFlags & UI64LIT(0x2000000000))
                return SPELL_WARLOCK_ARMOR;

            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            // "Well Fed" buff from Blessed Sunfruit, Blessed Sunfruit Juice, Alterac Spring Water
            if ((spellInfo->Attributes & SPELL_ATTR_CASTABLE_WHILE_SITTING) &&
                (spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_AUTOATTACK) &&
                (spellInfo->SpellIconID == 52 || spellInfo->SpellIconID == 79))
                return SPELL_WELL_FED;
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // only hunter stings have this
            if (spellInfo->Dispel == DISPEL_POISON)
                return SPELL_STING;

            // only hunter aspects have this (one have generic family)
            if (spellInfo->SpellFamilyFlags & UI64LIT(0x0044000000380000))
                return SPELL_ASPECT;

            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            if (IsSealSpell(spellInfo))
                return SPELL_SEAL;

            if (spellInfo->SpellFamilyFlags & UI64LIT(0x10000100))
                return SPELL_BLESSING;

            if ((spellInfo->SpellFamilyFlags & UI64LIT(0x00000820180400)) && (spellInfo->AttributesEx3 & 0x200))
                return SPELL_JUDGEMENT;

            for (int i = 0; i < 3; ++i)
            {
                // only paladin auras have this
                if (spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
                    return SPELL_AURA;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            if (IsElementalShield(spellInfo))
                return SPELL_ELEMENTAL_SHIELD;

            break;
        }

        case SPELLFAMILY_POTION:
            return sSpellMgr.GetSpellElixirSpecific(spellInfo->Id);
    }

    // only warlock armor/skin have this (in additional to family cases)
    if( spellInfo->SpellVisual == 130 && spellInfo->SpellIconID == 89)
    {
        return SPELL_WARLOCK_ARMOR;
    }

    // Tracking spells (exclude Well Fed, some other always allowed cases)
    if ((IsSpellHaveAura(spellInfo, SPELL_AURA_TRACK_CREATURES) ||
        IsSpellHaveAura(spellInfo, SPELL_AURA_TRACK_RESOURCES)  ||
        IsSpellHaveAura(spellInfo, SPELL_AURA_TRACK_STEALTHED)) &&
        ((spellInfo->AttributesEx & SPELL_ATTR_EX_UNK17) || (spellInfo->AttributesEx6 & SPELL_ATTR_EX6_UNK10)))
        return SPELL_TRACKER;

    // elixirs can have different families, but potion most ofc.
    if (SpellSpecific sp = sSpellMgr.GetSpellElixirSpecific(spellInfo->Id))
        return sp;

    return SPELL_NORMAL;
}


// target not allow have more one spell specific from same caster
bool IsSingleFromSpellSpecificPerTargetPerCaster(SpellSpecific spellSpec1,SpellSpecific spellSpec2)
{
    switch(spellSpec1)
    {
        case SPELL_BLESSING:
        case SPELL_AURA:
        case SPELL_STING:
        case SPELL_CURSE:
        case SPELL_ASPECT:
        case SPELL_POSITIVE_SHOUT:
        case SPELL_JUDGEMENT:
            return spellSpec1==spellSpec2;
        default:
            return false;
    }
}

// target not allow have more one ranks from spell from spell specific per target
bool IsSingleFromSpellSpecificSpellRanksPerTarget(SpellSpecific spellSpec1,SpellSpecific spellSpec2)
{
    switch(spellSpec1)
    {
        case SPELL_BLESSING:
        case SPELL_AURA:
        case SPELL_CURSE:
        case SPELL_ASPECT:
            return spellSpec1==spellSpec2;
        default:
            return false;
    }
}

// target not allow have more one spell specific per target from any caster
bool IsSingleFromSpellSpecificPerTarget(SpellSpecific spellSpec1,SpellSpecific spellSpec2)
{
    switch(spellSpec1)
    {
        case SPELL_SEAL:
        case SPELL_TRACKER:
        case SPELL_WARLOCK_ARMOR:
        case SPELL_MAGE_ARMOR:
        case SPELL_ELEMENTAL_SHIELD:
        case SPELL_MAGE_POLYMORPH:
        case SPELL_WELL_FED:
            return spellSpec1==spellSpec2;
        case SPELL_BATTLE_ELIXIR:
            return spellSpec2==SPELL_BATTLE_ELIXIR
                || spellSpec2==SPELL_FLASK_ELIXIR;
        case SPELL_GUARDIAN_ELIXIR:
            return spellSpec2==SPELL_GUARDIAN_ELIXIR
                || spellSpec2==SPELL_FLASK_ELIXIR;
        case SPELL_FLASK_ELIXIR:
            return spellSpec2==SPELL_BATTLE_ELIXIR
                || spellSpec2==SPELL_GUARDIAN_ELIXIR
                || spellSpec2==SPELL_FLASK_ELIXIR;
        case SPELL_FOOD:
            return spellSpec2==SPELL_FOOD
                || spellSpec2==SPELL_FOOD_AND_DRINK;
        case SPELL_DRINK:
            return spellSpec2==SPELL_DRINK
                || spellSpec2==SPELL_FOOD_AND_DRINK;
        case SPELL_FOOD_AND_DRINK:
            return spellSpec2==SPELL_FOOD
                || spellSpec2==SPELL_DRINK
                || spellSpec2==SPELL_FOOD_AND_DRINK;
        default:
            return false;
    }
}

bool IsPositiveTarget(uint32 targetA, uint32 targetB)
{
    switch(targetA)
    {
        // non-positive targets
        case TARGET_CHAIN_DAMAGE:
        case TARGET_ALL_ENEMY_IN_AREA:
        case TARGET_ALL_ENEMY_IN_AREA_INSTANT:
        case TARGET_IN_FRONT_OF_CASTER:
        case TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
        case TARGET_CURRENT_ENEMY_COORDINATES:
        case TARGET_SINGLE_ENEMY:
            return false;
        // positive or dependent
        case TARGET_CASTER_COORDINATES:
            return (targetB == TARGET_ALL_PARTY || targetB == TARGET_ALL_FRIENDLY_UNITS_AROUND_CASTER);
        default:
            break;
    }
    if (targetB)
        return IsPositiveTarget(targetB, 0);
    return true;
}

bool IsExplicitPositiveTarget(uint32 targetA)
{
    // positive targets that in target selection code expect target in m_targers, so not that auto-select target by spell data by m_caster and etc
    switch(targetA)
    {
        case TARGET_SINGLE_FRIEND:
        case TARGET_SINGLE_PARTY:
        case TARGET_CHAIN_HEAL:
        case TARGET_SINGLE_FRIEND_2:
        case TARGET_AREAEFFECT_PARTY_AND_CLASS:
            return true;
        default:
            break;
    }
    return false;
}

bool IsExplicitNegativeTarget(uint32 targetA)
{
    // non-positive targets that in target selection code expect target in m_targers, so not that auto-select target by spell data by m_caster and etc
    switch(targetA)
    {
        case TARGET_CHAIN_DAMAGE:
        case TARGET_CURRENT_ENEMY_COORDINATES:
        case TARGET_SINGLE_ENEMY:
            return true;
        default:
            break;
    }
    return false;
}

bool IsPositiveEffect(uint32 spellId, SpellEffectIndex effIndex)
{
    SpellEntry const *spellproto = sSpellStore.LookupEntry(spellId);
    if (!spellproto) return false;

    switch(spellproto->Effect[effIndex])
    {
        case SPELL_EFFECT_DUMMY:
            // some explicitly required dummy effect sets
            switch(spellId)
            {
                case 28441:                                 // AB Effect 000
                    return false;
                default:
                    break;
            }
            break;
        // always positive effects (check before target checks that provided non-positive result in some case for positive effects)
        case SPELL_EFFECT_HEAL:
        case SPELL_EFFECT_LEARN_SPELL:
        case SPELL_EFFECT_SKILL_STEP:
        case SPELL_EFFECT_HEAL_PCT:
        case SPELL_EFFECT_ENERGIZE_PCT:
            return true;

            // non-positive aura use
        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
        {
            switch(spellproto->EffectApplyAuraName[effIndex])
            {
                case SPELL_AURA_DUMMY:
                {
                    // dummy aura can be positive or negative dependent from casted spell
                    switch(spellproto->Id)
                    {
                        case 13139:                         // net-o-matic special effect
                        case 23445:                         // evil twin
                        case 35679:                         // Protectorate Demolitionist
                        case 38637:                         // Nether Exhaustion (red)
                        case 38638:                         // Nether Exhaustion (green)
                        case 38639:                         // Nether Exhaustion (blue)
                        case 44689:                         // Relay Race Accept Hidden Debuff - DND
                            return false;
                        // some spells have unclear target modes for selection, so just make effect positive
                        case 27184:
                        case 27190:
                        case 27191:
                        case 27201:
                        case 27202:
                        case 27203:
                            return true;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_MOD_DAMAGE_DONE:            // dependent from base point sign (negative -> negative)
                case SPELL_AURA_MOD_STAT:
                case SPELL_AURA_MOD_SKILL:
                case SPELL_AURA_MOD_DODGE_PERCENT:
                case SPELL_AURA_MOD_HEALING_PCT:
                case SPELL_AURA_MOD_HEALING_DONE:
                    if(spellproto->CalculateSimpleValue(effIndex) < 0)
                        return false;
                    break;
                case SPELL_AURA_MOD_DAMAGE_TAKEN:           // dependent from bas point sign (positive -> negative)
                    // Amplify Magic, Dampen Magic are positive buffs
                    if(spellproto->Attributes == 65536 && spellproto->AttributesEx == 131072 && spellproto->SpellFamilyFlags == 8192)
                        return true;
                    if(spellproto->CalculateSimpleValue(effIndex) > 0)
                        return false;
                    break;
                case SPELL_AURA_MOD_SPELL_CRIT_CHANCE:
                case SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT:
                case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE:
                    if(spellproto->CalculateSimpleValue(effIndex) > 0)
                        return true;                        // some expected positive spells have SPELL_ATTR_EX_NEGATIVE or unclear target modes
                    break;
                case SPELL_AURA_ADD_TARGET_TRIGGER:
                    return true;
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                    if (spellId != spellproto->EffectTriggerSpell[effIndex])
                    {
                        uint32 spellTriggeredId = spellproto->EffectTriggerSpell[effIndex];
                        SpellEntry const *spellTriggeredProto = sSpellStore.LookupEntry(spellTriggeredId);

                        if (spellTriggeredProto)
                        {
                            // non-positive targets of main spell return early
                            for(int i = 0; i < MAX_EFFECT_INDEX; ++i)
                            {
                                // if non-positive trigger cast targeted to positive target this main cast is non-positive
                                // this will place this spell auras as debuffs
                                if (IsPositiveTarget(spellTriggeredProto->EffectImplicitTargetA[effIndex],spellTriggeredProto->EffectImplicitTargetB[effIndex]) &&
                                    !IsPositiveEffect(spellTriggeredId,SpellEffectIndex(i)))
                                    return false;
                            }
                        }
                    }
                    break;
                case SPELL_AURA_PROC_TRIGGER_SPELL:
                    // many positive auras have negative triggered spells at damage for example and this not make it negative (it can be canceled for example)
                    break;
                case SPELL_AURA_MOD_STUN:                   //have positive and negative spells, we can't sort its correctly at this moment.
                    if (effIndex == EFFECT_INDEX_0 && spellproto->Effect[EFFECT_INDEX_1] == 0 && spellproto->Effect[EFFECT_INDEX_2] == 0)
                        return false;                       // but all single stun aura spells is negative

                    // Petrification
                    if(spellproto->Id == 17624)
                        return false;
                    break;
                case SPELL_AURA_MOD_PACIFY_SILENCE:
                    if(spellproto->Id == 24740)             // Wisp Costume
                        return true;
                    return false;
                case SPELL_AURA_MOD_ROOT:
                case SPELL_AURA_MOD_SILENCE:
                case SPELL_AURA_GHOST:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_MOD_STALKED:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                    return false;
                case SPELL_AURA_PERIODIC_DAMAGE:            // used in positive spells also.
                    // part of negative spell if casted at self (prevent cancel)
                    if (spellproto->EffectImplicitTargetA[effIndex] == TARGET_SELF ||
                        spellproto->EffectImplicitTargetA[effIndex] == TARGET_SELF2)
                        return false;
                    break;
                case SPELL_AURA_MOD_DECREASE_SPEED:         // used in positive spells also
                    // part of positive spell if casted at self
                    if ((spellproto->EffectImplicitTargetA[effIndex] == TARGET_SELF ||
                        spellproto->EffectImplicitTargetA[effIndex] == TARGET_SELF2) &&
                        spellproto->SpellFamilyName == SPELLFAMILY_GENERIC)
                        return false;
                    // but not this if this first effect (don't found better check)
                    if (spellproto->Attributes & 0x4000000 && effIndex == EFFECT_INDEX_0)
                        return false;
                    break;
                case SPELL_AURA_TRANSFORM:
                    // some spells negative
                    switch(spellproto->Id)
                    {
                        case 36897:                         // Transporter Malfunction (race mutation to horde)
                        case 36899:                         // Transporter Malfunction (race mutation to alliance)
                            return false;
                    }
                    break;
                case SPELL_AURA_MOD_SCALE:
                    // some spells negative
                    switch(spellproto->Id)
                    {
                        case 802:                           // Mutate Bug, wrongly negative by target modes
                            return true;
                        case 36900:                         // Soul Split: Evil!
                        case 36901:                         // Soul Split: Good
                        case 36893:                         // Transporter Malfunction (decrease size case)
                        case 36895:                         // Transporter Malfunction (increase size case)
                            return false;
                    }
                    break;
                case SPELL_AURA_MECHANIC_IMMUNITY:
                {
                    // non-positive immunities
                    switch(spellproto->EffectMiscValue[effIndex])
                    {
                        case MECHANIC_BANDAGE:
                        case MECHANIC_SHIELD:
                        case MECHANIC_MOUNT:
                        case MECHANIC_INVULNERABILITY:
                            return false;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_ADD_FLAT_MODIFIER:          // mods
                case SPELL_AURA_ADD_PCT_MODIFIER:
                {
                    // non-positive mods
                    switch(spellproto->EffectMiscValue[effIndex])
                    {
                        case SPELLMOD_COST:                 // dependent from bas point sign (negative -> positive)
                            if(spellproto->CalculateSimpleValue(effIndex) > 0)
                                return false;
                            break;
                        default:
                            break;
                    }
                }   break;
                case SPELL_AURA_FORCE_REACTION:
                    if(spellproto->Id==42792)               // Recently Dropped Flag (prevent cancel)
                        return false;
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    // non-positive targets
    if(!IsPositiveTarget(spellproto->EffectImplicitTargetA[effIndex],spellproto->EffectImplicitTargetB[effIndex]))
        return false;

    // AttributesEx check
    if(spellproto->AttributesEx & SPELL_ATTR_EX_NEGATIVE)
        return false;

    // ok, positive
    return true;
}

bool IsPositiveSpell(uint32 spellId)
{
    SpellEntry const *spellproto = sSpellStore.LookupEntry(spellId);
    if (!spellproto)
        return false;

    // spells with atleast one negative effect are considered negative
    // some self-applied spells have negative effects but in self casting case negative check ignored.
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (!IsPositiveEffect(spellId, SpellEffectIndex(i)))
            return false;
    return true;
}

bool IsSingleTargetSpell(SpellEntry const *spellInfo)
{
    // all other single target spells have if it has AttributesEx5
    if ( spellInfo->AttributesEx5 & SPELL_ATTR_EX5_SINGLE_TARGET_SPELL )
        return true;

    // TODO - need found Judgements rule
    switch(GetSpellSpecific(spellInfo->Id))
    {
        case SPELL_JUDGEMENT:
            return true;
        default:
            break;
    }

    // single target triggered spell.
    // Not real client side single target spell, but it' not triggered until prev. aura expired.
    // This is allow store it in single target spells list for caster for spell proc checking
    if(spellInfo->Id==38324)                                // Regeneration (triggered by 38299 (HoTs on Heals))
        return true;

    return false;
}

bool IsSingleTargetSpells(SpellEntry const *spellInfo1, SpellEntry const *spellInfo2)
{
    // TODO - need better check
    // Equal icon and spellfamily
    if( spellInfo1->SpellFamilyName == spellInfo2->SpellFamilyName &&
        spellInfo1->SpellIconID == spellInfo2->SpellIconID )
        return true;

    // TODO - need found Judgements rule
    SpellSpecific spec1 = GetSpellSpecific(spellInfo1->Id);
    // spell with single target specific types
    switch(spec1)
    {
        case SPELL_JUDGEMENT:
        case SPELL_MAGE_POLYMORPH:
            if(GetSpellSpecific(spellInfo2->Id) == spec1)
                return true;
            break;
        default:
            break;
    }

    return false;
}

SpellCastResult GetErrorAtShapeshiftedCast (SpellEntry const *spellInfo, uint32 form)
{
    // talents that learn spells can have stance requirements that need ignore
    // (this requirement only for client-side stance show in talent description)
    if( GetTalentSpellCost(spellInfo->Id) > 0 &&
        (spellInfo->Effect[EFFECT_INDEX_0] == SPELL_EFFECT_LEARN_SPELL || spellInfo->Effect[EFFECT_INDEX_1] == SPELL_EFFECT_LEARN_SPELL || spellInfo->Effect[EFFECT_INDEX_2] == SPELL_EFFECT_LEARN_SPELL) )
        return SPELL_CAST_OK;

    uint32 stanceMask = (form ? 1 << (form - 1) : 0);

    if (stanceMask & spellInfo->StancesNot)                 // can explicitly not be casted in this stance
        return SPELL_FAILED_NOT_SHAPESHIFT;

    if (stanceMask & spellInfo->Stances)                    // can explicitly be casted in this stance
        return SPELL_CAST_OK;

    bool actAsShifted = false;
    if (form > 0)
    {
        SpellShapeshiftFormEntry const *shapeInfo = sSpellShapeshiftFormStore.LookupEntry(form);
        if (!shapeInfo)
        {
            sLog.outError("GetErrorAtShapeshiftedCast: unknown shapeshift %u", form);
            return SPELL_CAST_OK;
        }
        actAsShifted = !(shapeInfo->flags1 & 1);            // shapeshift acts as normal form for spells
    }

    if(actAsShifted)
    {
        if (spellInfo->Attributes & SPELL_ATTR_NOT_SHAPESHIFT) // not while shapeshifted
            return SPELL_FAILED_NOT_SHAPESHIFT;
        else if (spellInfo->Stances != 0)                   // needs other shapeshift
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }
    else
    {
        // needs shapeshift
        if(!(spellInfo->AttributesEx2 & SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT) && spellInfo->Stances != 0)
            return SPELL_FAILED_ONLY_SHAPESHIFT;
    }

    return SPELL_CAST_OK;
}

void SpellMgr::LoadSpellTargetPositions()
{
    mSpellTargetPositions.clear();                                // need for reload case

    uint32 count = 0;

    //                                                0   1           2                  3                  4                  5
    QueryResult *result = WorldDatabase.Query("SELECT id, target_map, target_position_x, target_position_y, target_position_z, target_orientation FROM spell_target_position");
    if (!result)
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell target coordinates", count );
        return;
    }

    barGoLink bar((int)result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint32 Spell_ID = fields[0].GetUInt32();

        SpellTargetPosition st;

        st.target_mapId       = fields[1].GetUInt32();
        st.target_X           = fields[2].GetFloat();
        st.target_Y           = fields[3].GetFloat();
        st.target_Z           = fields[4].GetFloat();
        st.target_Orientation = fields[5].GetFloat();

        MapEntry const* mapEntry = sMapStore.LookupEntry(st.target_mapId);
        if (!mapEntry)
        {
            sLog.outErrorDb("Spell (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.",Spell_ID,st.target_mapId);
            continue;
        }

        if (st.target_X==0 && st.target_Y==0 && st.target_Z==0)
        {
            sLog.outErrorDb("Spell (ID:%u) target coordinates not provided.",Spell_ID);
            continue;
        }

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(Spell_ID);
        if (!spellInfo)
        {
            sLog.outErrorDb("Spell (ID:%u) listed in `spell_target_position` does not exist.",Spell_ID);
            continue;
        }

        bool found = false;
        for(int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (spellInfo->EffectImplicitTargetA[i]==TARGET_TABLE_X_Y_Z_COORDINATES || spellInfo->EffectImplicitTargetB[i]==TARGET_TABLE_X_Y_Z_COORDINATES)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            sLog.outErrorDb("Spell (Id: %u) listed in `spell_target_position` does not have target TARGET_TABLE_X_Y_Z_COORDINATES (17).",Spell_ID);
            continue;
        }

        mSpellTargetPositions[Spell_ID] = st;
        ++count;

    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell teleport coordinates", count );
}

void SpellMgr::LoadSpellAffects()
{
    mSpellAffectMap.clear();                                // need for reload case

    uint32 count = 0;

    //                                                0      1         2
    QueryResult *result = WorldDatabase.Query("SELECT entry, effectId, SpellFamilyMask FROM spell_affect");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell affect definitions", count );
        return;
    }

    barGoLink bar( (int)result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint32 entry = fields[0].GetUInt32();
        uint8 effectId = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(entry);

        if (!spellInfo)
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` does not exist", entry);
            continue;
        }

        if (effectId >= MAX_EFFECT_INDEX)
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` have invalid effect index (%u)", entry,effectId);
            continue;
        }

        if (spellInfo->Effect[effectId] != SPELL_EFFECT_APPLY_AURA || (
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_FLAT_MODIFIER &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_PCT_MODIFIER  &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_TARGET_TRIGGER &&
            spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_IGNORE_COMBAT_RESULT))
        {
            sLog.outErrorDb("Spell %u listed in `spell_affect` have not SPELL_AURA_ADD_FLAT_MODIFIER (%u) or SPELL_AURA_ADD_PCT_MODIFIER (%u) or SPELL_AURA_ADD_TARGET_TRIGGER (%u) for effect index (%u)", entry,SPELL_AURA_ADD_FLAT_MODIFIER,SPELL_AURA_ADD_PCT_MODIFIER,SPELL_AURA_ADD_TARGET_TRIGGER,effectId);
            continue;
        }

        uint64 spellAffectMask = fields[2].GetUInt64();

        // Spell.dbc have own data for low part of SpellFamilyMask
        if (spellInfo->EffectItemType[effectId])
        {
            if (spellInfo->EffectItemType[effectId] == spellAffectMask)
            {
                sLog.outErrorDb("Spell %u listed in `spell_affect` have redundant (same with EffectItemType%d) data for effect index (%u) and not needed, skipped.", entry,effectId+1,effectId);
                continue;
            }

            // 24429 have wrong data in EffectItemType and overwrites by DB, possible bug in client
            if (spellInfo->Id!=24429 && spellInfo->EffectItemType[effectId] != spellAffectMask)
            {
                sLog.outErrorDb("Spell %u listed in `spell_affect` have different low part from EffectItemType%d for effect index (%u) and not needed, skipped.", entry,effectId+1,effectId);
                continue;
            }
        }

        mSpellAffectMap.insert(SpellAffectMap::value_type((entry<<8) + effectId,spellAffectMask));

        ++count;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell affect definitions", count );

    for (uint32 id = 0; id < sSpellStore.GetNumRows(); ++id)
    {
        SpellEntry const* spellInfo = sSpellStore.LookupEntry(id);
        if (!spellInfo)
            continue;

        for (int effectId = 0; effectId < MAX_EFFECT_INDEX; ++effectId)
        {
            if (spellInfo->Effect[effectId] != SPELL_EFFECT_APPLY_AURA || (
                spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_FLAT_MODIFIER &&
                spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_PCT_MODIFIER  &&
                spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_ADD_TARGET_TRIGGER &&
                spellInfo->EffectApplyAuraName[effectId] != SPELL_AURA_IGNORE_COMBAT_RESULT))
                continue;

            if (spellInfo->EffectItemType[effectId] != 0)
                continue;

            if (mSpellAffectMap.find((id<<8) + effectId) !=  mSpellAffectMap.end())
                continue;

            sLog.outErrorDb("Spell %u (%s) misses spell_affect for effect %u",id,spellInfo->SpellName[sWorld.GetDefaultDbcLocale()], effectId);
        }
    }
}

template <typename EntryType, typename WorkerType, typename StorageType>
struct SpellRankHelper
{
    SpellRankHelper(SpellMgr &_mgr, StorageType &_storage): mgr(_mgr), worker(_storage), customRank(0) {}
    void RecordRank(EntryType &entry, uint32 spell_id)
    {
        const SpellEntry *spell = sSpellStore.LookupEntry(spell_id);
        if (!spell)
        {
            sLog.outErrorDb("Spell %u listed in `%s` does not exist", spell_id, worker.TableName());
            return;
        }

        uint32 first_id = mgr.GetFirstSpellInChain(spell_id);

        // most spell ranks expected same data
        if(first_id)
        {
            firstRankSpells.insert(first_id);

            if (first_id != spell_id)
            {
                if (!worker.IsValidCustomRank(entry, spell_id, first_id))
                    return;
                // for later check that first rank also added
                else
                {
                    firstRankSpellsWithCustomRanks.insert(first_id);
                    ++customRank;
                }
            }
        }

        worker.AddEntry(entry, spell);
    }
    void FillHigherRanks()
    {
        // check that first rank added for custom ranks
        for (std::set<uint32>::const_iterator itr = firstRankSpellsWithCustomRanks.begin(); itr != firstRankSpellsWithCustomRanks.end(); ++itr)
            if (!worker.HasEntry(*itr))
                sLog.outErrorDb("Spell %u must be listed in `%s` as first rank for listed custom ranks of spell but not found!", *itr, worker.TableName());

        // fill absent non first ranks data base at first rank data
        for (std::set<uint32>::const_iterator itr = firstRankSpells.begin(); itr != firstRankSpells.end(); ++itr)
        {
            if (worker.SetStateToEntry(*itr))
                mgr.doForHighRanks(*itr, worker);
        }
    }
    std::set<uint32> firstRankSpells;
    std::set<uint32> firstRankSpellsWithCustomRanks;

    SpellMgr &mgr;
    WorkerType worker;
    uint32 customRank;
};

struct DoSpellProcEvent
{
    DoSpellProcEvent(SpellProcEventMap& _spe_map) : spe_map(_spe_map), customProc(0), count(0) {}
    void operator() (uint32 spell_id)
    {
        SpellProcEventEntry const& spe = state->second;
        // add ranks only for not filled data (some ranks have ppm data different for ranks for example)
        SpellProcEventMap::const_iterator spellItr = spe_map.find(spell_id);
        if (spellItr == spe_map.end())
            spe_map[spell_id] = spe;
        // if custom rank data added then it must be same except ppm
        else
        {
            SpellProcEventEntry const& r_spe = spellItr->second;
            if (spe.schoolMask != r_spe.schoolMask)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different schoolMask from first rank in chain", spell_id);

            if (spe.spellFamilyName != r_spe.spellFamilyName)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different spellFamilyName from first rank in chain", spell_id);

            if (spe.spellFamilyMask != r_spe.spellFamilyMask)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different spellFamilyMask from first rank in chain", spell_id);

            if (spe.procFlags != r_spe.procFlags)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different procFlags from first rank in chain", spell_id);

            if (spe.procEx != r_spe.procEx)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different procEx from first rank in chain", spell_id);

            // only ppm allowed has been different from first rank

            if (spe.customChance != r_spe.customChance)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different customChance from first rank in chain", spell_id);

            if (spe.cooldown != r_spe.cooldown)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` as custom rank have different cooldown from first rank in chain", spell_id);
        }
    }

    const char* TableName() { return "spell_proc_event"; }
    bool IsValidCustomRank(SpellProcEventEntry const &spe, uint32 entry, uint32 first_id)
    {
        // let have independent data in table for spells with ppm rates (exist rank dependent ppm rate spells)
        if (!spe.ppmRate)
        {
            sLog.outErrorDb("Spell %u listed in `spell_proc_event` is not first rank (%u) in chain", entry, first_id);
            // prevent loading since it won't have an effect anyway
            return false;
        }
        return true;
    }
    void AddEntry(SpellProcEventEntry const &spe, SpellEntry const *spell)
    {
        spe_map[spell->Id] = spe;

        bool isCustom = false;

        if (spe.procFlags == 0)
        {
            if (spell->procFlags==0)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` probally not triggered spell (no proc flags)", spell->Id);
        }
        else
        {
            if (spell->procFlags==spe.procFlags)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` has exactly same proc flags as in spell.dbc, field value redundant", spell->Id);
            else
                isCustom = true;
        }

        if (spe.customChance == 0)
        {
            /* enable for re-check cases, 0 chance ok for some cases because in some cases it set by another spell/talent spellmod)
            if (spell->procChance==0 && !spe.ppmRate)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` probally not triggered spell (no chance or ppm)", spell->Id);
            */
        }
        else
        {
            if (spell->procChance==spe.customChance)
                sLog.outErrorDb("Spell %u listed in `spell_proc_event` has exactly same custom chance as in spell.dbc, field value redundant", spell->Id);
            else
                isCustom = true;
        }

        // totally redundant record
        if (!spe.schoolMask && !spe.spellFamilyMask && !spe.procFlags &&
            !spe.procEx && !spe.ppmRate && !spe.customChance && !spe.cooldown)
        {
            sLog.outErrorDb("Spell %u listed in `spell_proc_event` not have any useful data", spell->Id);
        }

        if (isCustom)
            ++customProc;
        else
            ++count;
    }

    bool HasEntry(uint32 spellId) { return spe_map.count(spellId) > 0; }
    bool SetStateToEntry(uint32 spellId) { return (state = spe_map.find(spellId)) != spe_map.end(); }
    SpellProcEventMap& spe_map;
    SpellProcEventMap::const_iterator state;

    uint32 customProc;
    uint32 count;
};

void SpellMgr::LoadSpellProcEvents()
{
    mSpellProcEventMap.clear();                             // need for reload case

    //                                                0      1           2                3                4          5       6        7             8
    QueryResult *result = WorldDatabase.Query("SELECT entry, SchoolMask, SpellFamilyName, SpellFamilyMask, procFlags, procEx, ppmRate, CustomChance, Cooldown FROM spell_proc_event");
    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();
        sLog.outString();
        sLog.outString( ">> No spell proc event conditions loaded");
        return;
    }

    SpellRankHelper<SpellProcEventEntry, DoSpellProcEvent, SpellProcEventMap> rankHelper(*this, mSpellProcEventMap);

    barGoLink bar( (int)result->GetRowCount() );
    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint32 entry = fields[0].GetUInt32();

        SpellProcEventEntry spe;

        spe.schoolMask      = fields[1].GetUInt32();
        spe.spellFamilyName = fields[2].GetUInt32();
        spe.spellFamilyMask = fields[3].GetUInt64();
        spe.procFlags       = fields[4].GetUInt32();
        spe.procEx          = fields[5].GetUInt32();
        spe.ppmRate         = fields[6].GetFloat();
        spe.customChance    = fields[7].GetFloat();
        spe.cooldown        = fields[8].GetUInt32();

        rankHelper.RecordRank(spe, entry);

    } while (result->NextRow());

    rankHelper.FillHigherRanks();

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u extra spell proc event conditions +%u custom proc (inc. +%u custom ranks)",  rankHelper.worker.count, rankHelper.worker.customProc, rankHelper.customRank);
}

struct DoSpellProcItemEnchant
{
    DoSpellProcItemEnchant(SpellProcItemEnchantMap& _procMap, float _ppm) : procMap(_procMap), ppm(_ppm) {}
    void operator() (uint32 spell_id) { procMap[spell_id] = ppm; }

    SpellProcItemEnchantMap& procMap;
    float ppm;
};

void SpellMgr::LoadSpellProcItemEnchant()
{
    mSpellProcItemEnchantMap.clear();                       // need for reload case

    uint32 count = 0;

    //                                                0      1
    QueryResult *result = WorldDatabase.Query("SELECT entry, ppmRate FROM spell_proc_item_enchant");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u proc item enchant definitions", count );
        return;
    }

    barGoLink bar( (int)result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint32 entry = fields[0].GetUInt32();
        float ppmRate = fields[1].GetFloat();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(entry);

        if (!spellInfo)
        {
            sLog.outErrorDb("Spell %u listed in `spell_proc_item_enchant` does not exist", entry);
            continue;
        }

        uint32 first_id = GetFirstSpellInChain(entry);

        if ( first_id != entry )
        {
            sLog.outErrorDb("Spell %u listed in `spell_proc_item_enchant` is not first rank (%u) in chain", entry, first_id);
            // prevent loading since it won't have an effect anyway
            continue;
        }

        mSpellProcItemEnchantMap[entry] = ppmRate;

        // also add to high ranks
        DoSpellProcItemEnchant worker(mSpellProcItemEnchantMap, ppmRate);
        doForHighRanks(entry,worker);

        ++count;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u proc item enchant definitions", count );
}

struct DoSpellBonuses
{
    DoSpellBonuses(SpellBonusMap& _spellBonusMap, SpellBonusEntry const& _spellBonus) : spellBonusMap(_spellBonusMap), spellBonus(_spellBonus) {}
    void operator() (uint32 spell_id) { spellBonusMap[spell_id] = spellBonus; }

    SpellBonusMap& spellBonusMap;
    SpellBonusEntry const& spellBonus;
};

void SpellMgr::LoadSpellBonuses()
{
    mSpellBonusMap.clear();                             // need for reload case
    uint32 count = 0;
    //                                                0      1             2          3
    QueryResult *result = WorldDatabase.Query("SELECT entry, direct_bonus, dot_bonus, ap_bonus FROM spell_bonus_data");
    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();
        sLog.outString();
        sLog.outString( ">> Loaded %u spell bonus data", count);
        return;
    }

    barGoLink bar( (int)result->GetRowCount() );
    do
    {
        Field *fields = result->Fetch();
        bar.step();
        uint32 entry = fields[0].GetUInt32();

        SpellEntry const* spell = sSpellStore.LookupEntry(entry);
        if (!spell)
        {
            sLog.outErrorDb("Spell %u listed in `spell_bonus_data` does not exist", entry);
            continue;
        }

        uint32 first_id = GetFirstSpellInChain(entry);

        if ( first_id != entry )
        {
            sLog.outErrorDb("Spell %u listed in `spell_bonus_data` is not first rank (%u) in chain", entry, first_id);
            // prevent loading since it won't have an effect anyway
            continue;
        }

        SpellBonusEntry sbe;

        sbe.direct_damage = fields[1].GetFloat();
        sbe.dot_damage    = fields[2].GetFloat();
        sbe.ap_bonus      = fields[3].GetFloat();

        mSpellBonusMap[entry] = sbe;

        // also add to high ranks
        DoSpellBonuses worker(mSpellBonusMap, sbe);
        doForHighRanks(entry, worker);

        ++count;

    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u extra spell bonus data",  count);
}

bool SpellMgr::IsSpellProcEventCanTriggeredBy(SpellProcEventEntry const * spellProcEvent, uint32 EventProcFlag, SpellEntry const * procSpell, uint32 procFlags, uint32 procExtra, bool active)
{
    // No extra req need
    uint32 procEvent_procEx = PROC_EX_NONE;

    // check prockFlags for condition
    if((procFlags & EventProcFlag) == 0)
        return false;

    // Always trigger for this
    if (EventProcFlag & (PROC_FLAG_KILLED | PROC_FLAG_KILL | PROC_FLAG_ON_TRAP_ACTIVATION))
        return true;

    if (spellProcEvent)     // Exist event data
    {
        // Store extra req
        procEvent_procEx = spellProcEvent->procEx;

        // For melee triggers
        if (procSpell == NULL)
        {
            // Check (if set) for school (melee attack have Normal school)
            if(spellProcEvent->schoolMask && (spellProcEvent->schoolMask & SPELL_SCHOOL_MASK_NORMAL) == 0)
                return false;
        }
        else // For spells need check school/spell family/family mask
        {
            // Check (if set) for school
            if(spellProcEvent->schoolMask && (spellProcEvent->schoolMask & procSpell->SchoolMask) == 0)
                return false;

            // Check (if set) for spellFamilyName
            if(spellProcEvent->spellFamilyName && (spellProcEvent->spellFamilyName != procSpell->SpellFamilyName))
                return false;

            // spellFamilyName is Ok need check for spellFamilyMask if present
            if(spellProcEvent->spellFamilyMask)
            {
                if ((spellProcEvent->spellFamilyMask & procSpell->SpellFamilyFlags) == 0)
                    return false;
                active = true; // Spell added manualy -> so its active spell
            }
        }
    }
    // Check for extra req (if none) and hit/crit
    if (procEvent_procEx == PROC_EX_NONE)
    {
        // No extra req, so can trigger only for active (damage/healing present) and hit/crit
        if((procExtra & (PROC_EX_NORMAL_HIT|PROC_EX_CRITICAL_HIT)) && active)
            return true;
    }
    else // Passive spells hits here only if resist/reflect/immune/evade
    {
        // Exist req for PROC_EX_EX_TRIGGER_ALWAYS
        if (procEvent_procEx & PROC_EX_EX_TRIGGER_ALWAYS)
            return true;
        // Passive spells can`t trigger if need hit (exclude cases when procExtra include non-active flags)
        if ((procEvent_procEx & PROC_EX_NORMAL_HIT & procExtra) && !active)
            return false;
        // Check Extra Requirement like (hit/crit/miss/resist/parry/dodge/block/immune/reflect/absorb and other)
        if (procEvent_procEx & procExtra)
            return true;
    }
    return false;
}

void SpellMgr::LoadSpellElixirs()
{
    mSpellElixirs.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                0      1
    QueryResult *result = WorldDatabase.Query("SELECT entry, mask FROM spell_elixir");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell elixir definitions", count );
        return;
    }

    barGoLink bar( (int)result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint32 entry = fields[0].GetUInt32();
        uint8 mask = fields[1].GetUInt8();

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(entry);

        if (!spellInfo)
        {
            sLog.outErrorDb("Spell %u listed in `spell_elixir` does not exist", entry);
            continue;
        }

        mSpellElixirs[entry] = mask;

        ++count;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell elixir definitions", count );
}

struct DoSpellThreat
{
    DoSpellThreat(SpellThreatMap& _threatMap) : threatMap(_threatMap), count(0) {}
    void operator() (uint32 spell_id)
    {
        SpellThreatEntry const &ste = state->second;
        // add ranks only for not filled data (spells adding flat threat are usually different for ranks)
        SpellThreatMap::const_iterator spellItr = threatMap.find(spell_id);
        if (spellItr == threatMap.end())
            threatMap[spell_id] = ste;

        // just assert that entry is not redundant
        else
        {
            SpellThreatEntry const& r_ste = spellItr->second;
            if (ste.threat == r_ste.threat && ste.multiplier == r_ste.multiplier && ste.ap_bonus == r_ste.ap_bonus)
                sLog.outErrorDb("Spell %u listed in `spell_threat` as custom rank has same data as Rank 1, so redundant", spell_id);
        }
    }
    const char* TableName() { return "spell_threat"; }
    bool IsValidCustomRank(SpellThreatEntry const &ste, uint32 entry, uint32 first_id)
    {
        if (!ste.threat)
        {
            sLog.outErrorDb("Spell %u listed in `spell_threat` is not first rank (%u) in chain and has no threat", entry, first_id);
            // prevent loading unexpected data
            return false;
        }
        return true;
    }
    void AddEntry(SpellThreatEntry const &ste, SpellEntry const *spell)
    {
        threatMap[spell->Id] = ste;

        // flat threat bonus and attack power bonus currently only work properly when all
        // effects have same targets, otherwise, we'd need to seperate it by effect index
        if (ste.threat || ste.ap_bonus != 0.f)
        {
            const uint32 *targetA = spell->EffectImplicitTargetA;
            const uint32 *targetB = spell->EffectImplicitTargetB;
            if ((targetA[EFFECT_INDEX_1] && targetA[EFFECT_INDEX_1] != targetA[EFFECT_INDEX_0]) ||
                (targetA[EFFECT_INDEX_2] && targetA[EFFECT_INDEX_2] != targetA[EFFECT_INDEX_0]))
                sLog.outErrorDb("Spell %u listed in `spell_threat` has effects with different targets, threat may be assigned incorrectly", spell->Id);
        }
        ++count;
    }
    bool HasEntry(uint32 spellId) { return threatMap.count(spellId) > 0; }
    bool SetStateToEntry(uint32 spellId) { return (state = threatMap.find(spellId)) != threatMap.end(); }

    SpellThreatMap& threatMap;
    SpellThreatMap::const_iterator state;
    uint32 count;
};

void SpellMgr::LoadSpellThreats()
{
    mSpellThreatMap.clear();                                // need for reload case

    //                                                0      1       2           3
    QueryResult *result = WorldDatabase.Query("SELECT entry, Threat, multiplier, ap_bonus FROM spell_threat");
    if( !result )
    {
        barGoLink bar( 1 );
        bar.step();
        sLog.outString();
        sLog.outString( ">> No spell threat entries loaded.");
        return;
    }

    SpellRankHelper<SpellThreatEntry, DoSpellThreat, SpellThreatMap> rankHelper(*this, mSpellThreatMap);

    barGoLink bar( (int)result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint32 entry = fields[0].GetUInt32();

        SpellThreatEntry ste;
        ste.threat = fields[1].GetUInt16();
        ste.multiplier = fields[2].GetFloat();
        ste.ap_bonus = fields[3].GetFloat();

        rankHelper.RecordRank(ste, entry);

    } while( result->NextRow() );

    rankHelper.FillHigherRanks();

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell threat entries", rankHelper.worker.count );
}

bool SpellMgr::IsRankSpellDueToSpell(SpellEntry const *spellInfo_1,uint32 spellId_2) const
{
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);
    if(!spellInfo_1 || !spellInfo_2) return false;
    if(spellInfo_1->Id == spellId_2) return false;

    return GetFirstSpellInChain(spellInfo_1->Id)==GetFirstSpellInChain(spellId_2);
}

bool SpellMgr::canStackSpellRanksInSpellBook(SpellEntry const *spellInfo) const
{
    if (IsPassiveSpell(spellInfo))                          // ranked passive spell
        return false;
    if (spellInfo->SpellFamilyName == SPELLFAMILY_DRUID && spellInfo->SpellFamilyFlags & UI64LIT(0x0000000000000400))
        return true;
    if (spellInfo->powerType != POWER_MANA && spellInfo->powerType != POWER_HEALTH)
        return false;
    if (IsProfessionOrRidingSpell(spellInfo->Id))
        return false;

    if (IsSkillBonusSpell(spellInfo->Id))
        return false;

    // All stance spells. if any better way, change it.
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        switch(spellInfo->SpellFamilyName)
        {
            case SPELLFAMILY_PALADIN:
                // Paladin aura Spell
                if (spellInfo->Effect[i]==SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
                    return false;
                break;
            case SPELLFAMILY_DRUID:
                // Druid form Spell
                if (spellInfo->Effect[i]==SPELL_EFFECT_APPLY_AURA &&
                    spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_SHAPESHIFT)
                    return false;
                break;
            case SPELLFAMILY_ROGUE:
                // Rogue Stealth
                if (spellInfo->Effect[i]==SPELL_EFFECT_APPLY_AURA &&
                    spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_SHAPESHIFT)
                    return false;
                break;
        }
    }
    return true;
}

bool SpellMgr::IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2) const
{
    // Jedna instancia spellu 2x >> False
    if(spellId_1 == spellId_2)
        return false;

    SpellEntry const *spellInfo_1 = sSpellStore.LookupEntry(spellId_1);
    SpellEntry const *spellInfo_2 = sSpellStore.LookupEntry(spellId_2);

    // Chyba ziskania informacii o spelle >> false
    if(!spellInfo_1 || !spellInfo_2)
        return false;

    //I think we don't check this correctly because i need a exception for spell:
    //72,11327,18461...(called from 1856,1857...) Call Aura 16,31, after trigger another spell who call aura 77 and 77 remove 16 and 31, this should not happen.
    if(spellInfo_2->SpellFamilyFlags == 2048)
        return false;

    // Resurrection sickness
    if((spellInfo_1->Id == SPELL_ID_PASSIVE_RESURRECTION_SICKNESS) != (spellInfo_2->Id==SPELL_ID_PASSIVE_RESURRECTION_SICKNESS))
        return false;

    // Allow stack passive and not passive spells
    if ((spellInfo_1->Attributes & SPELL_ATTR_PASSIVE)!=(spellInfo_2->Attributes & SPELL_ATTR_PASSIVE))
        return false;

    // Specific spell family spells
    switch(spellInfo_1->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
            switch(spellInfo_2->SpellFamilyName)
            {
                case SPELLFAMILY_GENERIC:                   // same family case
                {
                    // Thunderfury
                    if ((spellInfo_1->Id == 21992 && spellInfo_2->Id == 27648) ||
                        (spellInfo_2->Id == 21992 && spellInfo_1->Id == 27648))
                        return false;

                    // Lightning Speed (Mongoose) and Fury of the Crashing Waves (Tsunami Talisman)
                    if ((spellInfo_1->Id == 28093 && spellInfo_2->Id == 42084) ||
                        (spellInfo_2->Id == 28093 && spellInfo_1->Id == 42084))
                        return false;

                    // Soulstone Resurrection and Twisting Nether (resurrector)
                    if (spellInfo_1->SpellIconID == 92 && spellInfo_2->SpellIconID == 92 && (
                        (spellInfo_1->SpellVisual == 99 && spellInfo_2->SpellVisual == 0) ||
                        (spellInfo_2->SpellVisual == 99 && spellInfo_1->SpellVisual == 0)))
                        return false;

                    // Heart of the Wild, Agility and various Idol Triggers
                    if (spellInfo_1->SpellIconID == 240 && spellInfo_2->SpellIconID == 240)
                        return false;

                    // Personalized Weather (thunder effect should overwrite rainy aura)
                    if (spellInfo_1->SpellIconID == 2606 && spellInfo_2->SpellIconID == 2606)
                        return false;

                    // Brood Affliction: Bronze
                    if ((spellInfo_1->Id == 23170 && spellInfo_2->Id == 23171) ||
                        (spellInfo_2->Id == 23170 && spellInfo_1->Id == 23171))
                        return false;

                    // Male Shadowy Disguise
                    if ((spellInfo_1->Id == 32756 && spellInfo_2->Id == 38080) ||
                        (spellInfo_2->Id == 32756 && spellInfo_1->Id == 38080))
                         return false;

                    // Female Shadowy Disguise
                    if ((spellInfo_1->Id == 32756 && spellInfo_2->Id == 38081) ||
                        (spellInfo_2->Id == 32756 && spellInfo_1->Id == 38081))
                         return false;

                    // Regular and Night Elf Ghost
                    if ((spellInfo_1->Id == 8326 && spellInfo_2->Id == 20584) ||
                        (spellInfo_2->Id == 8326 && spellInfo_1->Id == 20584))
                         return false;

                    break;
                }
                case SPELLFAMILY_MAGE:
                    // Arcane Intellect and Insight
                    if (spellInfo_2->SpellIconID == 125 && spellInfo_1->Id == 18820)
                        return false;
                    break;
                case SPELLFAMILY_WARRIOR:
                {
                    // Scroll of Protection and Defensive Stance (multi-family check)
                    if (spellInfo_1->SpellIconID == 276 && spellInfo_1->SpellVisual == 196 && spellInfo_2->Id == 71)
                        return false;

                    // Improved Hamstring -> Hamstring (multi-family check)
                    if ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x2)) && spellInfo_1->Id == 23694)
                        return false;

                    break;
                }
                case SPELLFAMILY_DRUID:
                {
                    // Scroll of Stamina and Leader of the Pack (multi-family check)
                    if (spellInfo_1->SpellIconID == 312 && spellInfo_1->SpellVisual == 216 && spellInfo_2->Id == 24932)
                        return false;

                    // Dragonmaw Illusion (multi-family check)
                    if (spellId_1 == 40216 && spellId_2 == 42016)
                        return false;

                    break;
                }
                case SPELLFAMILY_ROGUE:
                {
                    // Garrote-Silence -> Garrote (multi-family check)
                    if (spellInfo_1->SpellIconID == 498 && spellInfo_1->SpellVisual == 0 && spellInfo_2->SpellIconID == 498)
                        return false;

                    break;
                }
                case SPELLFAMILY_HUNTER:
                {
                    // Concussive Shot and Imp. Concussive Shot (multi-family check)
                    if (spellInfo_1->Id == 19410 && spellInfo_2->Id == 5116)
                        return false;

                    // Improved Wing Clip -> Wing Clip (multi-family check)
                    if ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x40)) && spellInfo_1->Id == 19229)
                        return false;
                    break;
                }
                case SPELLFAMILY_PALADIN:
                {
                    // Unstable Currents and other -> *Sanctity Aura (multi-family check)
                    if (spellInfo_2->SpellIconID==502 && spellInfo_1->SpellIconID==502 && spellInfo_1->SpellVisual==969)
                        return false;

                    // *Band of Eternal Champion and Seal of Command(multi-family check)
                    if (spellId_1 == 35081 && spellInfo_2->SpellIconID==561 && spellInfo_2->SpellVisual==7992)
                        return false;

                    break;
                }
            }
            // Dragonmaw Illusion, Blood Elf Illusion, Human Illusion, Illidari Agent Illusion, Scarlet Crusade Disguise
            if(spellInfo_1->SpellIconID == 1691 && spellInfo_2->SpellIconID == 1691)
                return false;
            break;
        case SPELLFAMILY_MAGE:
            if( spellInfo_2->SpellFamilyName == SPELLFAMILY_MAGE )
            {
                // Blizzard & Chilled (and some other stacked with blizzard spells
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x80)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x100000))) ||
                    ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x80)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x100000))))
                    return false;

                // Blink & Improved Blink
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x0000000000010000)) && (spellInfo_2->SpellVisual == 72 && spellInfo_2->SpellIconID == 1499)) ||
                    ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x0000000000010000)) && (spellInfo_1->SpellVisual == 72 && spellInfo_1->SpellIconID == 1499)))
                    return false;

                // Fireball & Pyroblast (Dots)
                if( (spellInfo_1->SpellFamilyFlags & UI64LIT(0x1)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x400000)) ||
                    (spellInfo_2->SpellFamilyFlags & UI64LIT(0x1)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x400000)) )
                    return false;
            }
            // Detect Invisibility and Mana Shield (multi-family check)
            if (spellInfo_2->Id == 132 && spellInfo_1->SpellIconID == 209 && spellInfo_1->SpellVisual == 968)
                return false;

            // Combustion and Fire Protection Aura (multi-family check)
            if (spellInfo_1->Id == 11129 && spellInfo_2->SpellIconID == 33 && spellInfo_2->SpellVisual == 321)
                return false;

            // Arcane Intellect and Insight
            if (spellInfo_1->SpellIconID == 125 && spellInfo_2->Id == 18820)
                return false;

            break;
        case SPELLFAMILY_WARLOCK:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_WARLOCK)
            {
                // Siphon Life and Drain Life
                if ((spellInfo_1->SpellIconID == 152 && spellInfo_2->SpellIconID == 546) ||
                    (spellInfo_2->SpellIconID == 152 && spellInfo_1->SpellIconID == 546))
                    return false;

                //Corruption & Seed of corruption
                if ((spellInfo_1->SpellIconID == 313 && spellInfo_2->SpellIconID == 1932) ||
                    (spellInfo_2->SpellIconID == 313 && spellInfo_1->SpellIconID == 1932))
                    if(spellInfo_1->SpellVisual != 0 && spellInfo_2->SpellVisual != 0)
                        return true;                        // can't be stacked

                // Corruption and Unstable Affliction
                if ((spellInfo_1->SpellIconID == 313 && spellInfo_2->SpellIconID == 2039) ||
                    (spellInfo_2->SpellIconID == 313 && spellInfo_1->SpellIconID == 2039))
                    return false;

                // (Corruption or Unstable Affliction) and (Curse of Agony or Curse of Doom)
                if (((spellInfo_1->SpellIconID == 313 || spellInfo_1->SpellIconID == 2039) && (spellInfo_2->SpellIconID == 544  || spellInfo_2->SpellIconID == 91)) ||
                    ((spellInfo_2->SpellIconID == 313 || spellInfo_2->SpellIconID == 2039) && (spellInfo_1->SpellIconID == 544  || spellInfo_1->SpellIconID == 91)))
                    return false;

                // Fear a Death coil >> no Stack
                if((spellInfo_1->Attributes == 0x10000 && spellInfo_1->SpellFamilyFlags == UI64LIT(0x80000) && spellInfo_2->Attributes == 0x40010000 && spellInfo_2->SpellFamilyFlags == UI64LIT(0x40000000000)) ||
                   (spellInfo_2->Attributes == 0x10000 && spellInfo_2->SpellFamilyFlags == UI64LIT(0x80000) && spellInfo_1->Attributes == 0x40010000 && spellInfo_1->SpellFamilyFlags == UI64LIT(0x40000000000)))
                    return true;
            }
            // Detect Invisibility and Mana Shield (multi-family check)
            if (spellInfo_1->Id == 132 && spellInfo_2->SpellIconID == 209 && spellInfo_2->SpellVisual == 968)
                return false;
            break;
        case SPELLFAMILY_WARRIOR:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_WARRIOR)
            {
                // Rend and Deep Wound
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x20)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x1000000000))) ||
                    ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x20)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x1000000000))))
                    return false;

                // Battle Shout and Rampage
                if ((spellInfo_1->SpellIconID == 456 && spellInfo_2->SpellIconID == 2006) ||
                    (spellInfo_2->SpellIconID == 456 && spellInfo_1->SpellIconID == 2006))
                    return false;
            }

            // Hamstring -> Improved Hamstring (multi-family check)
            if ((spellInfo_1->SpellFamilyFlags & UI64LIT(0x2)) && spellInfo_2->Id == 23694)
                return false;

            // Defensive Stance and Scroll of Protection (multi-family check)
            if (spellInfo_1->Id == 71 && spellInfo_2->SpellIconID == 276 && spellInfo_2->SpellVisual == 196)
                return false;

            // Bloodlust and Bloodthirst (multi-family check)
            if (spellInfo_2->Id == 2825 && spellInfo_1->SpellIconID == 38 && spellInfo_1->SpellVisual == 0)
                return false;

            break;
        case SPELLFAMILY_PRIEST:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_PRIEST)
            {
                //Devouring Plague and Shadow Vulnerability
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x2000000)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x800000000))) ||
                    ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x2000000)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x800000000))))
                    return false;

                //StarShards and Shadow Word: Pain
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x200000)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x8000))) ||
                    ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x200000)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x8000))))
                    return false;
            }
            break;
        case SPELLFAMILY_DRUID:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_DRUID)
            {
                //Omen of Clarity and Blood Frenzy
                if (((spellInfo_1->SpellFamilyFlags == UI64LIT(0x0) && spellInfo_1->SpellIconID == 108) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x20000000000000))) ||
                    ((spellInfo_2->SpellFamilyFlags == UI64LIT(0x0) && spellInfo_2->SpellIconID == 108) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x20000000000000))))
                    return false;

                //  Tree of Life (Shapeshift) and 34123 Tree of Life (Passive)
                if ((spellId_1 == 33891 && spellId_2 == 34123) ||
                    (spellId_2 == 33891 && spellId_1 == 34123))
                    return false;

                // Wrath of Elune and Nature's Grace
                if ((spellInfo_1->Id == 16886 && spellInfo_2->Id == 46833) ||
                    (spellInfo_2->Id == 16886 && spellInfo_1->Id == 46833))
                    return false;

                // Bear Rage (Feral T4 (2)) and Omen of Clarity
                if ((spellInfo_1->Id == 16864 && spellInfo_2->Id == 37306) ||
                    (spellInfo_2->Id == 16864 && spellInfo_1->Id == 37306))
                    return false;

                // Cat Energy (Feral T4 (2)) and Omen of Clarity
                if ((spellInfo_1->Id == 16864 && spellInfo_2->Id == 37311) ||
                    (spellInfo_2->Id == 16864 && spellInfo_1->Id == 37311))
                    return false;
            }

            // Leader of the Pack and Scroll of Stamina (multi-family check)
            if (spellInfo_1->Id == 24932 && spellInfo_2->SpellIconID == 312 && spellInfo_2->SpellVisual == 216)
                return false;

            // Dragonmaw Illusion (multi-family check)
            if (spellId_1 == 42016 && spellId_2 == 40216 )
                return false;

            break;
        case SPELLFAMILY_ROGUE:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_ROGUE)
            {
                // Master of Subtlety
                if ((spellId_1 == 31665 && spellId_2 == 31666) ||
                    (spellId_1 == 31666 && spellId_2 == 31665))
                    return false;
            }

            // Garrote -> Garrote-Silence (multi-family check)
            if (spellInfo_1->SpellIconID == 498 && spellInfo_2->SpellIconID == 498 && spellInfo_2->SpellVisual == 0)
                return false;
            break;
        case SPELLFAMILY_HUNTER:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_HUNTER)
            {
                // Rapid Fire & Quick Shots
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x20)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x20000000000))) ||
                    ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x20)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x20000000000))) )
                    return false;

                // Serpent Sting & (Immolation/Explosive Trap Effect)
                if (((spellInfo_1->SpellFamilyFlags & UI64LIT(0x4)) && (spellInfo_2->SpellFamilyFlags & UI64LIT(0x00000004000))) ||
                    ((spellInfo_2->SpellFamilyFlags & UI64LIT(0x4)) && (spellInfo_1->SpellFamilyFlags & UI64LIT(0x00000004000))))
                    return false;

                // Bestial Wrath
                if (spellInfo_1->SpellIconID == 1680 && spellInfo_2->SpellIconID == 1680)
                    return false;
            }

            // Wing Clip -> Improved Wing Clip (multi-family check)
            if ((spellInfo_1->SpellFamilyFlags & UI64LIT(0x40)) && spellInfo_2->Id == 19229)
                return false;

            // Concussive Shot and Imp. Concussive Shot (multi-family check)
            if (spellInfo_2->Id == 19410 && spellInfo_1->Id == 5116)
                return false;
            break;
        case SPELLFAMILY_PALADIN:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_PALADIN)
            {
                // Paladin Seals
                if (IsSealSpell(spellInfo_1) && IsSealSpell(spellInfo_2))
                    return true;
                // Concentration Aura and Improved Concentration Aura and Aura Mastery
                if ((spellInfo_1->SpellIconID == 1487) && (spellInfo_2->SpellIconID == 1487))
                    return false;
            }

            // Combustion and Fire Protection Aura (multi-family check)
            if (spellInfo_2->Id == 11129 && spellInfo_1->SpellIconID == 33 && spellInfo_1->SpellVisual == 321)
                return false;

            // *Sanctity Aura -> Unstable Currents and other (multi-family check)
            if (spellInfo_1->SpellIconID==502 && spellInfo_2->SpellFamilyName == SPELLFAMILY_GENERIC && spellInfo_2->SpellIconID==502 && spellInfo_2->SpellVisual==969)
                return false;

            // *Seal of Command and Band of Eternal Champion (multi-family check)
            if (spellInfo_1->SpellIconID==561 && spellInfo_1->SpellVisual==7992 && spellId_2 == 35081)
                return false;
            break;
        case SPELLFAMILY_SHAMAN:
            if (spellInfo_2->SpellFamilyName == SPELLFAMILY_SHAMAN)
            {
                // Windfury weapon
                if (spellInfo_1->SpellIconID==220 && spellInfo_2->SpellIconID==220 &&
                    spellInfo_1->SpellFamilyFlags != spellInfo_2->SpellFamilyFlags)
                    return false;
            }
            // Bloodlust and Bloodthirst (multi-family check)
            if (spellInfo_1->Id == 2825 && spellInfo_2->SpellIconID == 38 && spellInfo_2->SpellVisual == 0)
                return false;
            break;
        default:
            break;
    }

    // more generic checks
    if (spellInfo_1->SpellIconID == spellInfo_2->SpellIconID &&
        spellInfo_1->SpellIconID != 0 && spellInfo_2->SpellIconID != 0)
    {
        bool isModifier = false;
        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if (spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER ||
                spellInfo_1->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER  ||
                spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_FLAT_MODIFIER ||
                spellInfo_2->EffectApplyAuraName[i] == SPELL_AURA_ADD_PCT_MODIFIER )
                isModifier = true;
        }

        if (!isModifier)
            return true;
    }

    if (IsRankSpellDueToSpell(spellInfo_1, spellId_2))
        return true;

    if (spellInfo_1->SpellFamilyName == 0 || spellInfo_2->SpellFamilyName == 0)
        return false;

    if (spellInfo_1->SpellFamilyName != spellInfo_2->SpellFamilyName)
        return false;

    bool dummy_only = true;
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (spellInfo_1->Effect[i] != spellInfo_2->Effect[i] ||
            spellInfo_1->EffectItemType[i] != spellInfo_2->EffectItemType[i] ||
            spellInfo_1->EffectMiscValue[i] != spellInfo_2->EffectMiscValue[i] ||
            spellInfo_1->EffectApplyAuraName[i] != spellInfo_2->EffectApplyAuraName[i])
            return false;

        // ignore dummy only spells
        if (spellInfo_1->Effect[i] && spellInfo_1->Effect[i] != SPELL_EFFECT_DUMMY && spellInfo_1->EffectApplyAuraName[i] != SPELL_AURA_DUMMY)
            dummy_only = false;
    }
    if (dummy_only)
        return false;

    return true;
}

bool SpellMgr::IsProfessionOrRidingSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return false;

    if(spellInfo->Effect[1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[1];

    return IsProfessionOrRidingSkill(skill);
}

bool SpellMgr::IsProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return false;

    if (spellInfo->Effect[EFFECT_INDEX_1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[EFFECT_INDEX_1];

    return IsProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionSpell(uint32 spellId)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spellId);
    if(!spellInfo)
        return false;

    if (spellInfo->Effect[EFFECT_INDEX_1] != SPELL_EFFECT_SKILL)
        return false;

    uint32 skill = spellInfo->EffectMiscValue[EFFECT_INDEX_1];

    return IsPrimaryProfessionSkill(skill);
}

bool SpellMgr::IsPrimaryProfessionFirstRankSpell(uint32 spellId) const
{
    return IsPrimaryProfessionSpell(spellId) && GetSpellRank(spellId)==1;
}

bool SpellMgr::IsSkillBonusSpell(uint32 spellId) const
{
    SkillLineAbilityMapBounds bounds = GetSkillLineAbilityMapBounds(spellId);

    for(SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        SkillLineAbilityEntry const *pAbility = _spell_idx->second;
        if (!pAbility || pAbility->learnOnGetSkill != ABILITY_LEARNED_ON_GET_PROFESSION_SKILL)
            continue;

        if (pAbility->req_skill_value > 0)
            return true;
    }

    return false;
}

SpellEntry const* SpellMgr::SelectAuraRankForLevel(SpellEntry const* spellInfo, uint32 level) const
{
    // fast case
    if (level + 10 >= spellInfo->spellLevel)
        return spellInfo;

    // ignore selection for passive spells
    if (IsPassiveSpell(spellInfo))
        return spellInfo;

    bool needRankSelection = false;
    for(int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        // for simple aura in check apply to any non caster based targets, in rank search mode to any explicit targets
        if (((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA && 
            (IsExplicitPositiveTarget(spellInfo->EffectImplicitTargetA[i]) ||
            IsAreaEffectPossitiveTarget(Targets(spellInfo->EffectImplicitTargetA[i])))) ||
            spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AREA_AURA_PARTY) &&
            IsPositiveEffect(spellInfo->Id, SpellEffectIndex(i)))
        {
            needRankSelection = true;
            break;
        }
    }

    // not required (rank check more slow so check it here)
    if (!needRankSelection || GetSpellRank(spellInfo->Id) == 0)
        return spellInfo;

    for(uint32 nextSpellId = spellInfo->Id; nextSpellId != 0; nextSpellId = GetPrevSpellInChain(nextSpellId))
    {
        SpellEntry const *nextSpellInfo = sSpellStore.LookupEntry(nextSpellId);
        if (!nextSpellInfo)
            break;

        // if found appropriate level
        if (level + 10 >= spellInfo->spellLevel)
            return nextSpellInfo;

        // one rank less then
    }

    // not found
    return NULL;
}

typedef UNORDERED_MAP<uint32,uint32> AbilitySpellPrevMap;

static void LoadSpellChains_AbilityHelper(SpellChainMap& chainMap, AbilitySpellPrevMap const& prevRanks, uint32 spell_id, uint32 prev_id, uint32 deep = 30)
{
    // spell already listed in chains store
    SpellChainMap::const_iterator chain_itr = chainMap.find(spell_id);
    if (chain_itr != chainMap.end())
    {
        MANGOS_ASSERT(chain_itr->second.prev == prev_id && "LoadSpellChains_AbilityHelper: Conflicting data in talents or spell abilities dbc");
        return;
    }

    // prev rank listed in main chain table (can fill correct data directly)
    SpellChainMap::const_iterator prev_chain_itr = chainMap.find(prev_id);
    if (prev_chain_itr != chainMap.end())
    {
        SpellChainNode node;
        node.prev  = prev_id;
        node.first = prev_chain_itr->second.first;
        node.rank  = prev_chain_itr->second.rank+1;
        node.req   = 0;
        chainMap[spell_id] = node;
        return;
    }

    // prev spell not listed in prev ranks store, so it first rank
    AbilitySpellPrevMap::const_iterator prev_itr = prevRanks.find(prev_id);
    if (prev_itr == prevRanks.end())
    {
        SpellChainNode prev_node;
        prev_node.prev  = 0;
        prev_node.first = prev_id;
        prev_node.rank  = 1;
        prev_node.req   = 0;
        chainMap[prev_id] = prev_node;

        SpellChainNode node;
        node.prev  = prev_id;
        node.first = prev_id;
        node.rank  = 2;
        node.req   = 0;
        chainMap[spell_id] = node;
        return;
    }

    if (deep == 0)
    {
        MANGOS_ASSERT(false && "LoadSpellChains_AbilityHelper: Infinity cycle in spell ability data");
        return;
    }

    // prev rank listed, so process it first
    LoadSpellChains_AbilityHelper(chainMap, prevRanks, prev_id, prev_itr->second, deep-1);

    // prev rank must be listed now
    prev_chain_itr = chainMap.find(prev_id);
    if (prev_chain_itr == chainMap.end())
        return;

    SpellChainNode node;
    node.prev  = prev_id;
    node.first = prev_chain_itr->second.first;
    node.rank  = prev_chain_itr->second.rank+1;
    node.req   = 0;
    chainMap[spell_id] = node;
}

void SpellMgr::LoadSpellChains()
{
    mSpellChains.clear();                                   // need for reload case
    mSpellChainsNext.clear();                               // need for reload case

    // load known data for talents
    for (unsigned int i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        // not add ranks for 1 ranks talents (if exist non ranks spells then it will included in table data)
        if (!talentInfo->RankID[1])
            continue;

        for (int j = 0; j < MAX_TALENT_RANK; j++)
        {
            uint32 spell_id = talentInfo->RankID[j];
            if (!spell_id)
                continue;

            if (!sSpellStore.LookupEntry(spell_id))
            {
                //sLog.outErrorDb("Talent %u not exist as spell",spell_id);
                continue;
            }

            SpellChainNode node;
            node.prev  = (j > 0) ? talentInfo->RankID[j-1] : 0;
            node.first = talentInfo->RankID[0];
            node.rank  = j+1;
            node.req   = 0;

            mSpellChains[spell_id] = node;
        }
    }

    // load known data from spell abilities
    {
        // we can calculate ranks only after full data generation
        AbilitySpellPrevMap prevRanks;
        for(SkillLineAbilityMap::const_iterator ab_itr = mSkillLineAbilityMap.begin(); ab_itr != mSkillLineAbilityMap.end(); ++ab_itr)
        {
            uint32 spell_id = ab_itr->first;

            // skip GM/test/internal spells.begin Its not have ranks anyway
            if (ab_itr->second->skillId == SKILL_INTERNAL)
                continue;

            // some forward spells not exist and can be ignored (some outdated data)
            SpellEntry const* spell_entry = sSpellStore.LookupEntry(spell_id);
            if (!spell_entry)                                   // no cases
                continue;

            // ignore spell without forwards (non ranked or missing info in skill abilities)
            uint32 forward_id = ab_itr->second->forward_spellid;

            // by some strange reason < 3.x clients not have forward spell for 2366
            if (spell_id == 2366)                           // Herb Gathering, Apprentice
                forward_id = 2368;

            if (!forward_id)
                continue;

            // some forward spells not exist and can be ignored (some outdated data)
            SpellEntry const* forward_entry = sSpellStore.LookupEntry(forward_id);
            if (!forward_entry)
                continue;

            // some forward spells still exist but excluded from real use as ranks and not listed in skill abilities now
            SkillLineAbilityMapBounds bounds = mSkillLineAbilityMap.equal_range(forward_id);
            if (bounds.first == bounds.second)
                continue;

            // spell already listed in chains store
            SpellChainMap::const_iterator chain_itr = mSpellChains.find(forward_id);
            if (chain_itr != mSpellChains.end())
            {
                MANGOS_ASSERT(chain_itr->second.prev == spell_id && "Conflicting data in talents or spell abilities dbc");
                continue;
            }

            // spell already listed in prev ranks store
            AbilitySpellPrevMap::const_iterator prev_itr = prevRanks.find(forward_id);
            if (prev_itr != prevRanks.end())
            {
                MANGOS_ASSERT(prev_itr->second == spell_id && "Conflicting data in talents or spell abilities dbc");
                continue;
            }

            // prev rank listed in main chain table (can fill correct data directly)
            SpellChainMap::const_iterator prev_chain_itr = mSpellChains.find(spell_id);
            if (prev_chain_itr != mSpellChains.end())
            {
                SpellChainNode node;
                node.prev  = spell_id;
                node.first = prev_chain_itr->second.first;
                node.rank  = prev_chain_itr->second.rank+1;
                node.req   = 0;

                mSpellChains[forward_id] = node;
                continue;
            }

            // need temporary store for later rank calculation
            prevRanks[forward_id] = spell_id;
        }

        while (!prevRanks.empty())
        {
            uint32 spell_id = prevRanks.begin()->first;
            uint32 prev_id  = prevRanks.begin()->second;
            prevRanks.erase(prevRanks.begin());

            LoadSpellChains_AbilityHelper(mSpellChains, prevRanks, spell_id, prev_id);
        }
    }

    // load custom case
    QueryResult *result = WorldDatabase.Query("SELECT spell_id, prev_spell, first_spell, rank, req_spell FROM spell_chain");
    if (!result)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 spell chain records" );
        sLog.outErrorDb("`spell_chains` table is empty!");
        return;
    }

    uint32 dbc_count = mSpellChains.size();
    uint32 new_count = 0;
    uint32 req_count = 0;

    barGoLink bar( (int)result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id = fields[0].GetUInt32();

        SpellChainNode node;
        node.prev  = fields[1].GetUInt32();
        node.first = fields[2].GetUInt32();
        node.rank  = fields[3].GetUInt8();
        node.req   = fields[4].GetUInt32();

        if (!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_chain` does not exist",spell_id);
            continue;
        }

        SpellChainMap::iterator chain_itr = mSpellChains.find(spell_id);
        if (chain_itr != mSpellChains.end())
        {
            if (chain_itr->second.rank != node.rank)
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` expected rank %u by DBC data.",
                    spell_id,node.prev,node.first,node.rank,node.req,chain_itr->second.rank);
                continue;
            }

            if (chain_itr->second.prev != node.prev)
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` expected prev %u by DBC data.",
                    spell_id,node.prev,node.first,node.rank,node.req,chain_itr->second.prev);
                continue;
            }

            if (chain_itr->second.first != node.first)
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` expected first %u by DBC data.",
                    spell_id,node.prev,node.first,node.rank,node.req,chain_itr->second.first);
                continue;
            }

            // update req field by table data
            if (node.req)
            {
                chain_itr->second.req = node.req;
                ++req_count;
                continue;
            }

            // in other case redundant
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) already added (talent or spell ability with forward) and non need in `spell_chain`",
                spell_id,node.prev,node.first,node.rank,node.req);
            continue;
        }

        if (node.prev != 0 && !sSpellStore.LookupEntry(node.prev))
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has nonexistent previous rank spell.",
                spell_id,node.prev,node.first,node.rank,node.req);
            continue;
        }

        if(!sSpellStore.LookupEntry(node.first))
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not existing first rank spell.",
                spell_id,node.prev,node.first,node.rank,node.req);
            continue;
        }

        // check basic spell chain data integrity (note: rank can be equal 0 or 1 for first/single spell)
        if( (spell_id == node.first) != (node.rank <= 1) ||
            (spell_id == node.first) != (node.prev == 0) ||
            (node.rank <= 1) != (node.prev == 0) )
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not compatible chain data.",
                spell_id,node.prev,node.first,node.rank,node.req);
            continue;
        }

        if(node.req!=0 && !sSpellStore.LookupEntry(node.req))
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not existing required spell.",
                spell_id,node.prev,node.first,node.rank,node.req);
            continue;
        }

        // talents not required data in spell chain for work, but must be checked if present for integrity
        if(TalentSpellPos const* pos = GetTalentSpellPos(spell_id))
        {
            if(node.rank!=pos->rank+1)
            {
                sLog.outErrorDb("Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong rank.",
                    spell_id,node.prev,node.first,node.rank,node.req);
                continue;
            }

            if(TalentEntry const* talentEntry = sTalentStore.LookupEntry(pos->talent_id))
            {
                if(node.first!=talentEntry->RankID[0])
                {
                    sLog.outErrorDb("Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong first rank spell.",
                        spell_id,node.prev,node.first,node.rank,node.req);
                    continue;
                }

                if(node.rank > 1 && node.prev != talentEntry->RankID[node.rank-1-1])
                {
                    sLog.outErrorDb("Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong prev rank spell.",
                        spell_id,node.prev,node.first,node.rank,node.req);
                    continue;
                }

                if(node.req!=talentEntry->DependsOnSpell)
                {
                    sLog.outErrorDb("Talent %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has wrong required spell.",
                        spell_id,node.prev,node.first,node.rank,node.req);
                    continue;
                }
            }
        }

        mSpellChains[spell_id] = node;

        ++new_count;
    } while( result->NextRow() );

    delete result;

    // additional integrity checks
    for(SpellChainMap::const_iterator i = mSpellChains.begin(); i != mSpellChains.end(); ++i)
    {
        if(i->second.prev)
        {
            SpellChainMap::const_iterator i_prev = mSpellChains.find(i->second.prev);
            if(i_prev == mSpellChains.end())
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not found previous rank spell in table.",
                    i->first,i->second.prev,i->second.first,i->second.rank,i->second.req);
            }
            else if( i_prev->second.first != i->second.first )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has different first spell in chain compared to previous rank spell (prev: %u, first: %u, rank: %d, req: %u).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i->second.req,
                    i_prev->second.prev,i_prev->second.first,i_prev->second.rank,i_prev->second.req);
            }
            else if( i_prev->second.rank+1 != i->second.rank )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has different rank compared to previous rank spell (prev: %u, first: %u, rank: %d, req: %u).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i->second.req,
                    i_prev->second.prev,i_prev->second.first,i_prev->second.rank,i_prev->second.req);
            }
        }

        if(i->second.req)
        {
            SpellChainMap::const_iterator i_req = mSpellChains.find(i->second.req);
            if(i_req == mSpellChains.end())
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has not found required rank spell in table.",
                    i->first,i->second.prev,i->second.first,i->second.rank,i->second.req);
            }
            else if( i_req->second.first == i->second.first )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has required rank spell from same spell chain (prev: %u, first: %u, rank: %d, req: %u).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i->second.req,
                    i_req->second.prev,i_req->second.first,i_req->second.rank,i_req->second.req);
            }
            else if( i_req->second.req )
            {
                sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has required rank spell with required spell (prev: %u, first: %u, rank: %d, req: %u).",
                    i->first,i->second.prev,i->second.first,i->second.rank,i->second.req,
                    i_req->second.prev,i_req->second.first,i_req->second.rank,i_req->second.req);
            }
        }
    }

    // fill next rank cache
    for(SpellChainMap::const_iterator i = mSpellChains.begin(); i != mSpellChains.end(); ++i)
    {
        uint32 spell_id = i->first;
        SpellChainNode const& node = i->second;

        if(node.prev)
            mSpellChainsNext.insert(SpellChainMapNext::value_type(node.prev,spell_id));

        if(node.req)
            mSpellChainsNext.insert(SpellChainMapNext::value_type(node.req,spell_id));
    }

    // check single rank redundant cases (single rank talents not added by default so this can be only custom cases)
    for(SpellChainMap::const_iterator i = mSpellChains.begin(); i != mSpellChains.end(); ++i)
    {
        // skip non-first ranks, and spells with additional reqs
        if (i->second.rank > 1 || i->second.req)
            continue;

        if (mSpellChainsNext.find(i->first) == mSpellChainsNext.end())
        {
            sLog.outErrorDb("Spell %u (prev: %u, first: %u, rank: %d, req: %u) listed in `spell_chain` has single rank data, so redundant.",
                i->first,i->second.prev,i->second.first,i->second.rank,i->second.req);
        }
    }

    sLog.outString();
    sLog.outString( ">> Loaded %u spell chain records (%u from DBC data with %u req field updates, and %u loaded from table)", dbc_count+new_count, dbc_count, req_count, new_count);
}

void SpellMgr::LoadSpellLearnSkills()
{
    mSpellLearnSkills.clear();                              // need for reload case

    // search auto-learned skills and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    barGoLink bar( sSpellStore.GetNumRows() );
    for(uint32 spell = 0; spell < sSpellStore.GetNumRows(); ++spell)
    {
        bar.step();
        SpellEntry const* entry = sSpellStore.LookupEntry(spell);

        if(!entry)
            continue;

        for(int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if(entry->Effect[i] == SPELL_EFFECT_SKILL)
            {
                SpellLearnSkillNode dbc_node;
                dbc_node.skill    = entry->EffectMiscValue[i];
                dbc_node.step     = entry->CalculateSimpleValue(SpellEffectIndex(i));
                if ( dbc_node.skill != SKILL_RIDING )
                    dbc_node.value = 1;
                else
                    dbc_node.value = dbc_node.step * 75;
                dbc_node.maxvalue = dbc_node.step * 75;

                mSpellLearnSkills[spell] = dbc_node;
                ++dbc_count;
                break;
            }
        }
    }

    sLog.outString();
    sLog.outString( ">> Loaded %u Spell Learn Skills from DBC", dbc_count );
}

void SpellMgr::LoadSpellLearnSpells()
{
    mSpellLearnSpells.clear();                              // need for reload case

    //                                                0      1        2
    QueryResult *result = WorldDatabase.Query("SELECT entry, SpellID, Active FROM spell_learn_spell");
    if (!result)
    {
        barGoLink bar( 1 );
        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded 0 spell learn spells" );
        sLog.outErrorDb("`spell_learn_spell` table is empty!");
        return;
    }

    uint32 count = 0;

    barGoLink bar( (int)result->GetRowCount() );
    do
    {
        bar.step();
        Field *fields = result->Fetch();

        uint32 spell_id    = fields[0].GetUInt32();

        SpellLearnSpellNode node;
        node.spell      = fields[1].GetUInt32();
        node.active     = fields[2].GetBool();
        node.autoLearned= false;

        if (!sSpellStore.LookupEntry(spell_id))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` does not exist",spell_id);
            continue;
        }

        if (!sSpellStore.LookupEntry(node.spell))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` learning nonexistent spell %u",spell_id,node.spell);
            continue;
        }

        if (GetTalentSpellCost(node.spell))
        {
            sLog.outErrorDb("Spell %u listed in `spell_learn_spell` attempt learning talent spell %u, skipped",spell_id,node.spell);
            continue;
        }

        mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell_id,node));

        ++count;
    } while( result->NextRow() );

    delete result;

    // search auto-learned spells and add its to map also for use in unlearn spells/talents
    uint32 dbc_count = 0;
    for(uint32 spell = 0; spell < sSpellStore.GetNumRows(); ++spell)
    {
        SpellEntry const* entry = sSpellStore.LookupEntry(spell);

        if (!entry)
            continue;

        for(int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if(entry->Effect[i]==SPELL_EFFECT_LEARN_SPELL)
            {
                SpellLearnSpellNode dbc_node;
                dbc_node.spell       = entry->EffectTriggerSpell[i];
                dbc_node.active      = true;                // all dbc based learned spells is active (show in spell book or hide by client itself)

                // ignore learning nonexistent spells (broken/outdated/or generic learnig spell 483
                if (!sSpellStore.LookupEntry(dbc_node.spell))
                    continue;

                // talent or passive spells or skill-step spells auto-casted and not need dependent learning,
                // pet teaching spells don't must be dependent learning (casted)
                // other required explicit dependent learning
                dbc_node.autoLearned = entry->EffectImplicitTargetA[i]==TARGET_PET || GetTalentSpellCost(spell) > 0 || IsPassiveSpell(entry) || IsSpellHaveEffect(entry,SPELL_EFFECT_SKILL_STEP);

                SpellLearnSpellMapBounds db_node_bounds = GetSpellLearnSpellMapBounds(spell);

                bool found = false;
                for(SpellLearnSpellMap::const_iterator itr = db_node_bounds.first; itr != db_node_bounds.second; ++itr)
                {
                    if (itr->second.spell == dbc_node.spell)
                    {
                        sLog.outErrorDb("Spell %u auto-learn spell %u in spell.dbc then the record in `spell_learn_spell` is redundant, please fix DB.",
                            spell,dbc_node.spell);
                        found = true;
                        break;
                    }
                }

                if (!found)                                 // add new spell-spell pair if not found
                {
                    mSpellLearnSpells.insert(SpellLearnSpellMap::value_type(spell,dbc_node));
                    ++dbc_count;
                }
            }
        }
    }

    sLog.outString();
    sLog.outString( ">> Loaded %u spell learn spells + %u found in DBC", count, dbc_count );
}

void SpellMgr::LoadSpellScriptTarget()
{
    mSpellScriptTarget.clear();                             // need for reload case

    uint32 count = 0;

    QueryResult *result = WorldDatabase.Query("SELECT entry,type,targetEntry FROM spell_script_target");

    if (!result)
    {
        barGoLink bar(1);

        bar.step();

        sLog.outString();
        sLog.outErrorDb(">> Loaded 0 SpellScriptTarget. DB table `spell_script_target` is empty.");
        return;
    }

    barGoLink bar((int)result->GetRowCount());

    do
    {
        Field *fields = result->Fetch();
        bar.step();

        uint32 spellId     = fields[0].GetUInt32();
        uint32 type        = fields[1].GetUInt32();
        uint32 targetEntry = fields[2].GetUInt32();

        SpellEntry const* spellProto = sSpellStore.LookupEntry(spellId);

        if (!spellProto)
        {
            sLog.outErrorDb("Table `spell_script_target`: spellId %u listed for TargetEntry %u does not exist.",spellId,targetEntry);
            continue;
        }

        bool targetfound = false;
        for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        {
            if( spellProto->EffectImplicitTargetA[i] == TARGET_SCRIPT ||
                spellProto->EffectImplicitTargetB[i] == TARGET_SCRIPT ||
                spellProto->EffectImplicitTargetA[i] == TARGET_SCRIPT_COORDINATES ||
                spellProto->EffectImplicitTargetB[i] == TARGET_SCRIPT_COORDINATES ||
                spellProto->EffectImplicitTargetA[i] == TARGET_FOCUS_OR_SCRIPTED_GAMEOBJECT ||
                spellProto->EffectImplicitTargetB[i] == TARGET_FOCUS_OR_SCRIPTED_GAMEOBJECT ||
                spellProto->EffectImplicitTargetA[i] == TARGET_AREAEFFECT_CUSTOM ||
                spellProto->EffectImplicitTargetB[i] == TARGET_AREAEFFECT_CUSTOM)
            {
                targetfound = true;
                break;
            }
        }
        if (!targetfound)
        {
            sLog.outErrorDb("Table `spell_script_target`: spellId %u listed for TargetEntry %u does not have any implicit target TARGET_SCRIPT(38) or TARGET_SCRIPT_COORDINATES (46) or TARGET_FOCUS_OR_SCRIPTED_GAMEOBJECT (40).", spellId, targetEntry);
            continue;
        }

        if (type >= MAX_SPELL_TARGET_TYPE)
        {
            sLog.outErrorDb("Table `spell_script_target`: target type %u for TargetEntry %u is incorrect.",type,targetEntry);
            continue;
        }

        // Checks by target type
        switch (type)
        {
            case SPELL_TARGET_TYPE_GAMEOBJECT:
            {
                if (!targetEntry)
                    break;

                if (!sGOStorage.LookupEntry<GameObjectInfo>(targetEntry))
                {
                    sLog.outErrorDb("Table `spell_script_target`: gameobject template entry %u does not exist.",targetEntry);
                    continue;
                }
                break;
            }
            default:
                if (!targetEntry)
                {
                    sLog.outErrorDb("Table `spell_script_target`: target entry == 0 for not GO target type (%u).",type);
                    continue;
                }
                if (const CreatureInfo* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(targetEntry))
                {
                    if (spellId == 30427 && !cInfo->SkinLootId)
                    {
                        sLog.outErrorDb("Table `spell_script_target` has creature %u as a target of spellid 30427, but this creature has no skinlootid. Gas extraction will not work!", cInfo->Entry);
                        continue;
                    }
                }
                else
                {
                    sLog.outErrorDb("Table `spell_script_target`: creature template entry %u does not exist.",targetEntry);
                    continue;
                }
                break;
        }

        mSpellScriptTarget.insert(SpellScriptTarget::value_type(spellId,SpellTargetEntry(SpellTargetType(type),targetEntry)));

        ++count;
    } while (result->NextRow());

    delete result;

    // Check all spells
    /* Disabled (lot errors at this moment)
    for(uint32 i = 1; i < sSpellStore.nCount; ++i)
    {
        SpellEntry const * spellInfo = sSpellStore.LookupEntry(i);
        if(!spellInfo)
            continue;

        bool found = false;
        for(int j = 0; j < MAX_EFFECT_INDEX; ++j)
        {
            if( spellInfo->EffectImplicitTargetA[j] == TARGET_SCRIPT || spellInfo->EffectImplicitTargetA[j] != TARGET_SELF && spellInfo->EffectImplicitTargetB[j] == TARGET_SCRIPT )
            {
                SpellScriptTarget::const_iterator lower = GetBeginSpellScriptTarget(spellInfo->Id);
                SpellScriptTarget::const_iterator upper = GetEndSpellScriptTarget(spellInfo->Id);
                if(lower==upper)
                {
                    sLog.outErrorDb("Spell (ID: %u) has effect EffectImplicitTargetA/EffectImplicitTargetB = %u (TARGET_SCRIPT), but does not have record in `spell_script_target`",spellInfo->Id,TARGET_SCRIPT);
                    break;                                  // effects of spell
                }
            }
        }
    }
    */

    sLog.outString();
    sLog.outString(">> Loaded %u Spell Script Targets", count);
}

void SpellMgr::LoadSpellPetAuras()
{
    mSpellPetAuraMap.clear();                                  // need for reload case

    uint32 count = 0;

    //                                                0      1    2
    QueryResult *result = WorldDatabase.Query("SELECT spell, pet, aura FROM spell_pet_auras");
    if( !result )
    {

        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell pet auras", count );
        return;
    }

    barGoLink bar( (int)result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint32 spell = fields[0].GetUInt32();
        uint32 pet = fields[1].GetUInt32();
        uint32 aura = fields[2].GetUInt32();

        SpellPetAuraMap::iterator itr = mSpellPetAuraMap.find(spell);
        if(itr != mSpellPetAuraMap.end())
        {
            itr->second.AddAura(pet, aura);
        }
        else
        {
            SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell);
            if (!spellInfo)
            {
                sLog.outErrorDb("Spell %u listed in `spell_pet_auras` does not exist", spell);
                continue;
            }
            int i = 0;
            for(; i < MAX_EFFECT_INDEX; ++i)
                if((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA &&
                    spellInfo->EffectApplyAuraName[i] == SPELL_AURA_DUMMY) ||
                    spellInfo->Effect[i] == SPELL_EFFECT_DUMMY)
                    break;

            if(i == MAX_EFFECT_INDEX)
            {
                sLog.outError("Spell %u listed in `spell_pet_auras` does not have dummy aura or dummy effect", spell);
                continue;
            }

            SpellEntry const* spellInfo2 = sSpellStore.LookupEntry(aura);
            if (!spellInfo2)
            {
                sLog.outErrorDb("Aura %u listed in `spell_pet_auras` does not exist", aura);
                continue;
            }

            PetAura pa(pet, aura, spellInfo->EffectImplicitTargetA[i] == TARGET_PET, spellInfo->CalculateSimpleValue(SpellEffectIndex(i)));
            mSpellPetAuraMap[spell] = pa;
        }

        ++count;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell pet auras", count );
}

/// Some checks for spells, to prevent adding deprecated/broken spells for trainers, spell book, etc
bool SpellMgr::IsSpellValid(SpellEntry const* spellInfo, Player* pl, bool msg)
{
    // not exist
    if(!spellInfo)
        return false;

    bool need_check_reagents = false;

    // check effects
    for(int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        switch(spellInfo->Effect[i])
        {
            case 0:
                continue;

            // craft spell for crafting nonexistent item (break client recipes list show)
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if(!ObjectMgr::GetItemPrototype( spellInfo->EffectItemType[i] ))
                {
                    if(msg)
                    {
                        if(pl)
                            ChatHandler(pl).PSendSysMessage("Craft spell %u create item (Entry: %u) but item does not exist in item_template.",spellInfo->Id,spellInfo->EffectItemType[i]);
                        else
                            sLog.outErrorDb("Craft spell %u create item (Entry: %u) but item does not exist in item_template.",spellInfo->Id,spellInfo->EffectItemType[i]);
                    }
                    return false;
                }

                need_check_reagents = true;
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                SpellEntry const* spellInfo2 = sSpellStore.LookupEntry(spellInfo->EffectTriggerSpell[i]);
                if( !IsSpellValid(spellInfo2,pl,msg) )
                {
                    if(msg)
                    {
                        if(pl)
                            ChatHandler(pl).PSendSysMessage("Spell %u learn to broken spell %u, and then...",spellInfo->Id,spellInfo->EffectTriggerSpell[i]);
                        else
                            sLog.outErrorDb("Spell %u learn to invalid spell %u, and then...",spellInfo->Id,spellInfo->EffectTriggerSpell[i]);
                    }
                    return false;
                }
                break;
            }
        }
    }

    if(need_check_reagents)
    {
        for(int j = 0; j < MAX_SPELL_REAGENTS; ++j)
        {
            if(spellInfo->Reagent[j] > 0 && !ObjectMgr::GetItemPrototype( spellInfo->Reagent[j] ))
            {
                if(msg)
                {
                    if(pl)
                        ChatHandler(pl).PSendSysMessage("Craft spell %u requires reagent item (Entry: %u) but item does not exist in item_template.",spellInfo->Id,spellInfo->Reagent[j]);
                    else
                        sLog.outErrorDb("Craft spell %u requires reagent item (Entry: %u) but item does not exist in item_template.",spellInfo->Id,spellInfo->Reagent[j]);
                }
                return false;
            }
        }
    }

    return true;
}

void SpellMgr::LoadSpellAreas()
{
    mSpellAreaMap.clear();                                  // need for reload case
    mSpellAreaForQuestMap.clear();
    mSpellAreaForActiveQuestMap.clear();
    mSpellAreaForQuestEndMap.clear();
    mSpellAreaForAuraMap.clear();

    uint32 count = 0;

    //                                                0      1     2            3                   4          5           6         7       8
    QueryResult *result = WorldDatabase.Query("SELECT spell, area, quest_start, quest_start_active, quest_end, aura_spell, racemask, gender, autocast FROM spell_area");

    if( !result )
    {
        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outString( ">> Loaded %u spell area requirements", count );
        return;
    }

    barGoLink bar( (int)result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint32 spell = fields[0].GetUInt32();
        SpellArea spellArea;
        spellArea.spellId             = spell;
        spellArea.areaId              = fields[1].GetUInt32();
        spellArea.questStart          = fields[2].GetUInt32();
        spellArea.questStartCanActive = fields[3].GetBool();
        spellArea.questEnd            = fields[4].GetUInt32();
        spellArea.auraSpell           = fields[5].GetInt32();
        spellArea.raceMask            = fields[6].GetUInt32();
        spellArea.gender              = Gender(fields[7].GetUInt8());
        spellArea.autocast            = fields[8].GetBool();

        if(!sSpellStore.LookupEntry(spell))
        {
            sLog.outErrorDb("Spell %u listed in `spell_area` does not exist", spell);
            continue;
        }

        {
            bool ok = true;
            SpellAreaMapBounds sa_bounds = GetSpellAreaMapBounds(spellArea.spellId);
            for(SpellAreaMap::const_iterator itr = sa_bounds.first; itr != sa_bounds.second; ++itr)
            {
                if (spellArea.spellId != itr->second.spellId)
                    continue;
                if (spellArea.areaId != itr->second.areaId)
                    continue;
                if (spellArea.questStart != itr->second.questStart)
                    continue;
                if (spellArea.auraSpell != itr->second.auraSpell)
                    continue;
                if ((spellArea.raceMask & itr->second.raceMask) == 0)
                    continue;
                if (spellArea.gender != itr->second.gender)
                    continue;

                // duplicate by requirements
                ok =false;
                break;
            }

            if(!ok)
            {
                sLog.outErrorDb("Spell %u listed in `spell_area` already listed with similar requirements.", spell);
                continue;
            }

        }

        if(spellArea.areaId && !GetAreaEntryByAreaID(spellArea.areaId))
        {
            sLog.outErrorDb("Spell %u listed in `spell_area` have wrong area (%u) requirement", spell,spellArea.areaId);
            continue;
        }

        if(spellArea.questStart && !sObjectMgr.GetQuestTemplate(spellArea.questStart))
        {
            sLog.outErrorDb("Spell %u listed in `spell_area` have wrong start quest (%u) requirement", spell,spellArea.questStart);
            continue;
        }

        if(spellArea.questEnd)
        {
            if(!sObjectMgr.GetQuestTemplate(spellArea.questEnd))
            {
                sLog.outErrorDb("Spell %u listed in `spell_area` have wrong end quest (%u) requirement", spell,spellArea.questEnd);
                continue;
            }

            if(spellArea.questEnd==spellArea.questStart && !spellArea.questStartCanActive)
            {
                sLog.outErrorDb("Spell %u listed in `spell_area` have quest (%u) requirement for start and end in same time", spell,spellArea.questEnd);
                continue;
            }
        }

        if(spellArea.auraSpell)
        {
            SpellEntry const* spellInfo = sSpellStore.LookupEntry(abs(spellArea.auraSpell));
            if(!spellInfo)
            {
                sLog.outErrorDb("Spell %u listed in `spell_area` have wrong aura spell (%u) requirement", spell,abs(spellArea.auraSpell));
                continue;
            }

            switch(spellInfo->EffectApplyAuraName[EFFECT_INDEX_0])
            {
                case SPELL_AURA_DUMMY:
                case SPELL_AURA_GHOST:
                    break;
                default:
                    sLog.outErrorDb("Spell %u listed in `spell_area` have aura spell requirement (%u) without dummy/phase/ghost aura in effect 0", spell,abs(spellArea.auraSpell));
                    continue;
            }

            if(uint32(abs(spellArea.auraSpell))==spellArea.spellId)
            {
                sLog.outErrorDb("Spell %u listed in `spell_area` have aura spell (%u) requirement for itself", spell, abs(spellArea.auraSpell));
                continue;
            }

            // not allow autocast chains by auraSpell field (but allow use as alternative if not present)
            if (spellArea.autocast && spellArea.auraSpell > 0)
            {
                bool chain = false;
                SpellAreaForAuraMapBounds saBound = GetSpellAreaForAuraMapBounds(spellArea.spellId);
                for(SpellAreaForAuraMap::const_iterator itr = saBound.first; itr != saBound.second; ++itr)
                {
                    if (itr->second->autocast && itr->second->auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    sLog.outErrorDb("Spell %u listed in `spell_area` have aura spell (%u) requirement that itself autocast from aura", spell,spellArea.auraSpell);
                    continue;
                }

                SpellAreaMapBounds saBound2 = GetSpellAreaMapBounds(spellArea.auraSpell);
                for(SpellAreaMap::const_iterator itr2 = saBound2.first; itr2 != saBound2.second; ++itr2)
                {
                    if (itr2->second.autocast && itr2->second.auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if(chain)
                {
                    sLog.outErrorDb("Spell %u listed in `spell_area` have aura spell (%u) requirement that itself autocast from aura", spell,spellArea.auraSpell);
                    continue;
                }
            }
        }

        if(spellArea.raceMask && (spellArea.raceMask & RACEMASK_ALL_PLAYABLE)==0)
        {
            sLog.outErrorDb("Spell %u listed in `spell_area` have wrong race mask (%u) requirement", spell,spellArea.raceMask);
            continue;
        }

        if(spellArea.gender!=GENDER_NONE && spellArea.gender!=GENDER_FEMALE && spellArea.gender!=GENDER_MALE)
        {
            sLog.outErrorDb("Spell %u listed in `spell_area` have wrong gender (%u) requirement", spell,spellArea.gender);
            continue;
        }

        SpellArea const* sa = &mSpellAreaMap.insert(SpellAreaMap::value_type(spell,spellArea))->second;

        // for search by current zone/subzone at zone/subzone change
        if(spellArea.areaId)
            mSpellAreaForAreaMap.insert(SpellAreaForAreaMap::value_type(spellArea.areaId,sa));

        // for search at quest start/reward
        if(spellArea.questStart)
        {
            if(spellArea.questStartCanActive)
                mSpellAreaForActiveQuestMap.insert(SpellAreaForQuestMap::value_type(spellArea.questStart,sa));
            else
                mSpellAreaForQuestMap.insert(SpellAreaForQuestMap::value_type(spellArea.questStart,sa));
        }

        // for search at quest start/reward
        if(spellArea.questEnd)
            mSpellAreaForQuestEndMap.insert(SpellAreaForQuestMap::value_type(spellArea.questEnd,sa));

        // for search at aura apply
        if(spellArea.auraSpell)
            mSpellAreaForAuraMap.insert(SpellAreaForAuraMap::value_type(abs(spellArea.auraSpell),sa));

        ++count;
    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Loaded %u spell area requirements", count );
}

SpellCastResult SpellMgr::GetSpellAllowedInLocationError(SpellEntry const *spellInfo, uint32 map_id, uint32 zone_id, uint32 area_id, Player * player)
{
    // normal case
    if (spellInfo->AreaId > 0 && spellInfo->AreaId != zone_id && spellInfo->AreaId != area_id)
        return SPELL_FAILED_REQUIRES_AREA;
    try
    {
        // continent limitation (virtual continent), ignore for GM
        if ((spellInfo->AttributesEx4 & SPELL_ATTR_EX4_CAST_ONLY_IN_OUTLAND) && (player && !player->isGameMaster() && ((uint32)(player->GetSession()->GetSecurity()) < sWorld.getConfig(CONFIG_UINT32_AZEROTH_GMLEVEL_FLY)) || player->GetMap()->Instanceable()))
        {
            uint32 v_map = GetVirtualMapForMapAndZone(map_id, zone_id);
            MapEntry const* mapEntry = sMapStore.LookupEntry(v_map);
            if (!mapEntry || mapEntry->addon < 1 || !mapEntry->IsContinent())
                return SPELL_FAILED_REQUIRES_AREA;
        }
    }
    catch(...)
    {
         sWorld.SendGMWorldText(SECURITY_MODERATOR, LANG_ANTICRASH_NOTIFY, "SpellMgr::GetSpellAllowedInLocationError 2");
         sLog.outError("### Casso: SpellMgr::GetSpellAllowedInLocationError 2: Pokus o zamedzenie crashu aktivovany ###");
         sLog.outInterest("### Casso: SpellMgr::GetSpellAllowedInLocationError 2: Pokus o zamedzenie crashu aktivovany ###");
    }

    try
    {
        // zakaz fly mounta v urcitych zonach
        if (spellInfo->AttributesEx4 & SPELL_ATTR_EX4_CAST_ONLY_IN_OUTLAND && player && !player->isGameMaster())
        {
            if(player->GetMapId() != 0 && player->GetMapId() != 1 && player->GetMapId() != 530)
                return SPELL_FAILED_REQUIRES_AREA;

            switch(player->GetAreaId())
            {
                case 1637: // Orgrimmar
                    if(player->GetTeam() == ALLIANCE)
                    {
                        ChatHandler(player).SendSysMessage("Flying not allowed here! Teleporting...");
                        player->TeleportTo(1, 1000.78f, -4446.22f, 11.21f, 0.208f);
                        return SPELL_CAST_OK;
                    }
                case 3487: // Silvermoon City
                    if(player->GetTeam() == ALLIANCE)
                    {
                        ChatHandler(player).SendSysMessage("Flying not allowed here! Teleporting...");
                        player->TeleportTo(530, 9079.92f, -7193.23f, 55.61f, 5.946f);
                        return SPELL_CAST_OK;
                    }
                    //return SPELL_FAILED_REQUIRES_AREA;
                    return SPELL_CAST_OK;

                case 1519: // Stormwind
                    if(player->GetTeam() == HORDE)
                    {
                        ChatHandler(player).SendSysMessage("Flying not allowed here! Teleporting...");
                        player->TeleportTo(0, -9617.06f, -288.95f, 57.31f, 1.235f);
                        return SPELL_CAST_OK;
                    }
                    //return SPELL_FAILED_REQUIRES_AREA;
                    return SPELL_CAST_OK;

                // GH positions
                case 168:  // Tirisfal glades sea
                case 1256: // Azshara sea
                    return SPELL_FAILED_REQUIRES_AREA;
            }

            // ioqd dismount
            if(player->GetZoneId() == 4080)
                return SPELL_FAILED_REQUIRES_AREA;

            // Dire Maul - port
            if(player->GetZoneId() == 2557)
            {
                ChatHandler(player).SendSysMessage("Flying not allowed here! Teleporting...");
                player->TeleportTo(1, -4841.19f, 1309.44f, 81.3937f, 0.008f);
                return SPELL_CAST_OK;
            }
        }
    }
    catch(...)
    {
         sWorld.SendGMWorldText(SECURITY_MODERATOR, LANG_ANTICRASH_NOTIFY, "SpellMgr::GetSpellAllowedInLocationError");
         sLog.outError("### Casso: SpellMgr::GetSpellAllowedInLocationError: Pokus o zamedzenie crashu aktivovany ###");
         sLog.outInterest("### Casso: SpellMgr::GetSpellAllowedInLocationError: Pokus o zamedzenie crashu aktivovany ###");
    }

    // raid instance limitation
    if (spellInfo->AttributesEx6 & SPELL_ATTR_EX6_NOT_IN_RAID_INSTANCE)
    {
        MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
        if (!mapEntry || mapEntry->IsRaid())
            return SPELL_FAILED_REQUIRES_AREA;
    }

    // DB base check (if non empty then must fit at least single for allow)
    SpellAreaMapBounds saBounds = GetSpellAreaMapBounds(spellInfo->Id);
    if (saBounds.first != saBounds.second)
    {
        for(SpellAreaMap::const_iterator itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            if(itr->second.IsFitToRequirements(player,zone_id,area_id))
                return SPELL_CAST_OK;
        }
        return SPELL_FAILED_REQUIRES_AREA;
    }

    // bg spell checks

    // do not allow spells to be cast in arenas
    // - with SPELL_ATTR_EX4_NOT_USABLE_IN_ARENA flag
    // - with greater than 15 min CD
    if ((spellInfo->AttributesEx4 & SPELL_ATTR_EX4_NOT_USABLE_IN_ARENA) ||
         (GetSpellRecoveryTime(spellInfo) > 15 * MINUTE * IN_MILLISECONDS && !(spellInfo->AttributesEx4 & SPELL_ATTR_EX4_USABLE_IN_ARENA)))
        if (player && player->InArena())
            return SPELL_FAILED_NOT_IN_ARENA;

    // Spell casted only on battleground
    if ((spellInfo->AttributesEx3 & SPELL_ATTR_EX3_BATTLEGROUND))
        if (!player || !player->InBattleGround())
            return SPELL_FAILED_ONLY_BATTLEGROUNDS;

    switch(spellInfo->Id)
    {
        // a trinket in alterac valley allows to teleport to the boss
        case 22564:                                         // recall
        case 22563:                                         // recall
        {
            if (!player)
                return SPELL_FAILED_REQUIRES_AREA;
            BattleGround* bg = player->GetBattleGround();
            return map_id == 30 && bg
                && bg->GetStatus() != STATUS_WAIT_JOIN ? SPELL_CAST_OK : SPELL_FAILED_REQUIRES_AREA;
        }
        case 23333:                                         // Warsong Flag
        case 23335:                                         // Silverwing Flag
            return map_id == 489 && player && player->InBattleGround() ? SPELL_CAST_OK : SPELL_FAILED_REQUIRES_AREA;
        case 34976:                                         // Netherstorm Flag
            return map_id == 566 && player && player->InBattleGround() ? SPELL_CAST_OK : SPELL_FAILED_REQUIRES_AREA;
        case 2584:                                          // Waiting to Resurrect
        case 42792:                                         // Recently Dropped Flag
        case 43681:                                         // Inactive
        {
            return player && player->InBattleGround() ? SPELL_CAST_OK : SPELL_FAILED_ONLY_BATTLEGROUNDS;
        }
        case 22011:                                         // Spirit Heal Channel
        case 22012:                                         // Spirit Heal
        case 24171:                                         // Resurrection Impact Visual
        case 44535:                                         // Spirit Heal (mana)
        {
            MapEntry const* mapEntry = sMapStore.LookupEntry(map_id);
            if (!mapEntry)
                return SPELL_FAILED_REQUIRES_AREA;
            return mapEntry->IsBattleGround()? SPELL_CAST_OK : SPELL_FAILED_ONLY_BATTLEGROUNDS;
        }
        case 44521:                                         // Preparation
        {
            if (!player)
                return SPELL_FAILED_REQUIRES_AREA;

            BattleGround* bg = player->GetBattleGround();
            return bg && bg->GetStatus()==STATUS_WAIT_JOIN ? SPELL_CAST_OK : SPELL_FAILED_ONLY_BATTLEGROUNDS;
        }
        case 32724:                                         // Gold Team (Alliance)
        case 32725:                                         // Green Team (Alliance)
        case 35774:                                         // Gold Team (Horde)
        case 35775:                                         // Green Team (Horde)
        {
            return player && player->InArena() ? SPELL_CAST_OK : SPELL_FAILED_ONLY_IN_ARENA;
        }
        case 32727:                                         // Arena Preparation
        {
            if (!player)
                return SPELL_FAILED_REQUIRES_AREA;
            if (!player->InArena())
                return SPELL_FAILED_REQUIRES_AREA;

            BattleGround* bg = player->GetBattleGround();
            return bg && bg->GetStatus()==STATUS_WAIT_JOIN ? SPELL_CAST_OK : SPELL_FAILED_ONLY_IN_ARENA;
        }
    }

    return SPELL_CAST_OK;
}

void SpellMgr::LoadSkillLineAbilityMap()
{
    mSkillLineAbilityMap.clear();

    barGoLink bar( (int)sSkillLineAbilityStore.GetNumRows() );
    uint32 count = 0;

    for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); ++i)
    {
        bar.step();
        SkillLineAbilityEntry const *SkillInfo = sSkillLineAbilityStore.LookupEntry(i);
        if(!SkillInfo)
            continue;

        mSkillLineAbilityMap.insert(SkillLineAbilityMap::value_type(SkillInfo->spellId,SkillInfo));
        ++count;
    }

    sLog.outString();
    sLog.outString(">> Loaded %u SkillLineAbility MultiMap Data", count);
}

void SpellMgr::CheckUsedSpells(char const* table)
{
    uint32 countSpells = 0;
    uint32 countMasks = 0;

    //                                                 0       1               2               3         4           5             6          7          8         9    10
    QueryResult *result = WorldDatabase.PQuery("SELECT spellid,SpellFamilyName,SpellFamilyMask,SpellIcon,SpellVisual,SpellCategory,EffectType,EffectAura,EffectIdx,Name,Code FROM %s",table);

    if( !result )
    {
        barGoLink bar( 1 );

        bar.step();

        sLog.outString();
        sLog.outErrorDb("`%s` table is empty!",table);
        return;
    }

    barGoLink bar( (int)result->GetRowCount() );

    do
    {
        Field *fields = result->Fetch();

        bar.step();

        uint32 spell       = fields[0].GetUInt32();
        int32  family      = fields[1].GetInt32();
        uint64 familyMask  = fields[2].GetUInt64();
        int32  spellIcon   = fields[3].GetInt32();
        int32  spellVisual = fields[4].GetInt32();
        int32  category    = fields[5].GetInt32();
        int32  effectType  = fields[6].GetInt32();
        int32  auraType    = fields[7].GetInt32();
        int32  effectIdx   = fields[8].GetInt32();
        std::string name   = fields[9].GetCppString();
        std::string code   = fields[10].GetCppString();

        // checks of correctness requirements itself

        if (family < -1 || family > SPELLFAMILY_UNK3)
        {
            sLog.outError("Table '%s' for spell %u have wrong SpellFamily value(%u), skipped.",table,spell,family);
            continue;
        }

        // TODO: spellIcon check need dbc loading
        if (spellIcon < -1)
        {
            sLog.outError("Table '%s' for spell %u have wrong SpellIcon value(%u), skipped.",table,spell,spellIcon);
            continue;
        }

        // TODO: spellVisual check need dbc loading
        if (spellVisual < -1)
        {
            sLog.outError("Table '%s' for spell %u have wrong SpellVisual value(%u), skipped.",table,spell,spellVisual);
            continue;
        }

        // TODO: for spellCategory better check need dbc loading
        if (category < -1 || (category >=0 && sSpellCategoryStore.find(category) == sSpellCategoryStore.end()))
        {
            sLog.outError("Table '%s' for spell %u have wrong SpellCategory value(%u), skipped.",table,spell,category);
            continue;
        }

        if (effectType < -1 || effectType >= TOTAL_SPELL_EFFECTS)
        {
            sLog.outError("Table '%s' for spell %u have wrong SpellEffect type value(%u), skipped.",table,spell,effectType);
            continue;
        }

        if (auraType < -1 || auraType >= TOTAL_AURAS)
        {
            sLog.outError("Table '%s' for spell %u have wrong SpellAura type value(%u), skipped.",table,spell,auraType);
            continue;
        }

        if (effectIdx < -1 || effectIdx >= 3)
        {
            sLog.outError("Table '%s' for spell %u have wrong EffectIdx value(%u), skipped.",table,spell,effectIdx);
            continue;
        }

        // now checks of requirements

        if(spell)
        {
            ++countSpells;

            SpellEntry const* spellEntry = sSpellStore.LookupEntry(spell);
            if(!spellEntry)
            {
                sLog.outError("Spell %u '%s' not exist but used in %s.",spell,name.c_str(),code.c_str());
                continue;
            }

            if (family >= 0 && spellEntry->SpellFamilyName != uint32(family))
            {
                sLog.outError("Spell %u '%s' family(%u) <> %u but used in %s.",spell,name.c_str(),spellEntry->SpellFamilyName,family,code.c_str());
                continue;
            }

            if(familyMask != UI64LIT(0xFFFFFFFFFFFFFFFF))
            {
                if(familyMask == UI64LIT(0x0000000000000000))
                {
                    if(spellEntry->SpellFamilyFlags != 0)
                    {
                        sLog.outError("Spell %u '%s' not fit to (" UI64FMTD ") but used in %s.",
                            spell, name.c_str(), familyMask, code.c_str());
                        continue;
                    }

                }
                else
                {
                    if((spellEntry->SpellFamilyFlags & familyMask)==0)
                    {
                        sLog.outError("Spell %u '%s' not fit to (" I64FMT ") but used in %s.",spell,name.c_str(),familyMask,code.c_str());
                        continue;
                    }

                }
            }

            if (spellIcon >= 0 && spellEntry->SpellIconID != uint32(spellIcon))
            {
                sLog.outError("Spell %u '%s' icon(%u) <> %u but used in %s.",spell,name.c_str(),spellEntry->SpellIconID,spellIcon,code.c_str());
                continue;
            }

            if (spellVisual >= 0 && spellEntry->SpellVisual != uint32(spellVisual))
            {
                sLog.outError("Spell %u '%s' visual(%u) <> %u but used in %s.",spell,name.c_str(),spellEntry->SpellVisual,spellVisual,code.c_str());
                continue;
            }

            if (category >= 0 && spellEntry->Category != uint32(category))
            {
                sLog.outError("Spell %u '%s' category(%u) <> %u but used in %s.",spell,name.c_str(),spellEntry->Category,category,code.c_str());
                continue;
            }

            if (effectIdx >= EFFECT_INDEX_0)
            {
                if (effectType >= 0 && spellEntry->Effect[effectIdx] != uint32(effectType))
                {
                    sLog.outError("Spell %u '%s' effect%d <> %u but used in %s.",spell,name.c_str(),effectIdx+1,effectType,code.c_str());
                    continue;
                }

                if (auraType >= 0 && spellEntry->EffectApplyAuraName[effectIdx] != uint32(auraType))
                {
                    sLog.outError("Spell %u '%s' aura%d <> %u but used in %s.",spell,name.c_str(),effectIdx+1,auraType,code.c_str());
                    continue;
                }

            }
            else
            {
                if (effectType >= 0 && !IsSpellHaveEffect(spellEntry,SpellEffects(effectType)))
                {
                    sLog.outError("Spell %u '%s' not have effect %u but used in %s.",spell,name.c_str(),effectType,code.c_str());
                    continue;
                }

                if (auraType >= 0 && !IsSpellHaveAura(spellEntry, AuraType(auraType)))
                {
                    sLog.outError("Spell %u '%s' not have aura %u but used in %s.",spell,name.c_str(),auraType,code.c_str());
                    continue;
                }
            }
        }
        else
        {
            ++countMasks;

            bool found = false;
            for(uint32 spellId = 1; spellId < sSpellStore.GetNumRows(); ++spellId)
            {
                SpellEntry const* spellEntry = sSpellStore.LookupEntry(spellId);
                if (!spellEntry)
                    continue;

                if (family >=0 && spellEntry->SpellFamilyName != uint32(family))
                    continue;

                if (familyMask != UI64LIT(0xFFFFFFFFFFFFFFFF))
                {
                    if(familyMask == UI64LIT(0x0000000000000000))
                    {
                        if(spellEntry->SpellFamilyFlags != 0)
                            continue;
                    }
                    else
                    {
                        if ((spellEntry->SpellFamilyFlags & familyMask)==0)
                            continue;
                    }
                }

                if (spellIcon >= 0 && spellEntry->SpellIconID != uint32(spellIcon))
                    continue;

                if (spellVisual >= 0 && spellEntry->SpellVisual != uint32(spellVisual))
                    continue;

                if (category >= 0 && spellEntry->Category != uint32(category))
                    continue;

                if (effectIdx >= 0)
                {
                    if (effectType >=0 && spellEntry->Effect[effectIdx] != uint32(effectType))
                        continue;

                    if (auraType >=0 && spellEntry->EffectApplyAuraName[effectIdx] !=uint32(auraType))
                        continue;
                }
                else
                {
                    if (effectType >=0 && !IsSpellHaveEffect(spellEntry,SpellEffects(effectType)))
                        continue;

                    if (auraType >=0 && !IsSpellHaveAura(spellEntry,AuraType(auraType)))
                        continue;
                }

                found = true;
                break;
            }

            if (!found)
            {
                if (effectIdx >= 0)
                    sLog.outError("Spells '%s' not found for family %i (" I64FMT ") icon(%i) visual(%i) category(%i) effect%d(%i) aura%d(%i) but used in %s",
                        name.c_str(),family,familyMask,spellIcon,spellVisual,category,effectIdx+1,effectType,effectIdx+1,auraType,code.c_str());
                else
                    sLog.outError("Spells '%s' not found for family %i (" I64FMT ") icon(%i) visual(%i) category(%i) effect(%i) aura(%i) but used in %s",
                        name.c_str(),family,familyMask,spellIcon,spellVisual,category,effectType,auraType,code.c_str());
                continue;
            }
        }

    } while( result->NextRow() );

    delete result;

    sLog.outString();
    sLog.outString( ">> Checked %u spells and %u spell masks", countSpells, countMasks );
}

DiminishingGroup GetDiminishingReturnsGroupForSpell(SpellEntry const* spellproto, bool triggered)
{
    // Explicit Diminishing Groups
    switch(spellproto->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
            // some generic arena related spells have by some strange reason MECHANIC_TURN
            if  (spellproto->Mechanic == MECHANIC_TURN)
                return DIMINISHING_NONE;
            break;
        case SPELLFAMILY_ROGUE:
        {
            // Kidney Shot
            if (spellproto->SpellFamilyFlags & UI64LIT(0x00000200000))
                return DIMINISHING_KIDNEYSHOT;
            // Blind
            else if (spellproto->SpellFamilyFlags & UI64LIT(0x00001000000))
                return DIMINISHING_BLIND_CYCLONE;
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Freezing trap
            if (spellproto->SpellFamilyFlags & UI64LIT(0x00000000008))
                return DIMINISHING_FREEZE;
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Fear
            if (spellproto->SpellFamilyFlags & UI64LIT(0x40840000000))
                return DIMINISHING_WARLOCK_FEAR;
            // Curses/etc
            if (spellproto->SpellFamilyFlags & UI64LIT(0x00080000000))
                return DIMINISHING_LIMITONLY;
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            // Cyclone
            if (spellproto->SpellFamilyFlags & UI64LIT(0x02000000000))
                return DIMINISHING_BLIND_CYCLONE;
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            // Hamstring - limit duration to 10s in PvP
            if (spellproto->SpellFamilyFlags & UI64LIT(0x00000000002))
                return DIMINISHING_LIMITONLY;
            break;
        }
        default:
            break;
    }

    // Get by mechanic
    uint32 mechanic = GetAllSpellMechanicMask(spellproto);
    if (!mechanic)
        return DIMINISHING_NONE;

    if (mechanic & (1<<(MECHANIC_STUN-1)))
        return triggered ? DIMINISHING_TRIGGER_STUN : DIMINISHING_CONTROL_STUN;
    if (mechanic & (1<<(MECHANIC_SLEEP-1)))
        return DIMINISHING_SLEEP;
    if (mechanic & (1<<(MECHANIC_POLYMORPH-1)))
        return DIMINISHING_POLYMORPH;
    if (mechanic & (1<<(MECHANIC_ROOT-1)))
        return triggered ? DIMINISHING_TRIGGER_ROOT : DIMINISHING_CONTROL_ROOT;
    if (mechanic & (1<<(MECHANIC_FEAR-1)))
        return DIMINISHING_FEAR;
    if (mechanic & (1<<(MECHANIC_CHARM-1)))
        return DIMINISHING_CHARM;
    if (mechanic & (1<<(MECHANIC_SILENCE-1)))
        return DIMINISHING_SILENCE;
    if (mechanic & (1<<(MECHANIC_DISARM-1)))
        return DIMINISHING_DISARM;
    if (mechanic & (1<<(MECHANIC_FREEZE-1)))
        return DIMINISHING_FREEZE;
    if (mechanic & ((1<<(MECHANIC_KNOCKOUT-1))|(1<<(MECHANIC_SAPPED-1))))
        return DIMINISHING_KNOCKOUT;
    if (mechanic & (1<<(MECHANIC_BANISH-1)))
        return DIMINISHING_BANISH;
    if (mechanic & (1<<(MECHANIC_HORROR-1)))
        return DIMINISHING_DEATHCOIL;

    return DIMINISHING_NONE;
}

bool IsDiminishingReturnsGroupDurationLimited(DiminishingGroup group)
{
    switch(group)
    {
        case DIMINISHING_CONTROL_STUN:
        case DIMINISHING_TRIGGER_STUN:
        case DIMINISHING_KIDNEYSHOT:
        case DIMINISHING_SLEEP:
        case DIMINISHING_CONTROL_ROOT:
        case DIMINISHING_TRIGGER_ROOT:
        case DIMINISHING_FEAR:
        case DIMINISHING_WARLOCK_FEAR:
        case DIMINISHING_CHARM:
        case DIMINISHING_POLYMORPH:
        case DIMINISHING_FREEZE:
        case DIMINISHING_KNOCKOUT:
        case DIMINISHING_BLIND_CYCLONE:
        case DIMINISHING_BANISH:
        case DIMINISHING_LIMITONLY:
            return true;
        default:
            return false;
    }
    return false;
}

DiminishingReturnsType GetDiminishingReturnsGroupType(DiminishingGroup group)
{
    switch(group)
    {
        case DIMINISHING_BLIND_CYCLONE:
        case DIMINISHING_CONTROL_STUN:
        case DIMINISHING_TRIGGER_STUN:
        case DIMINISHING_KIDNEYSHOT:
            return DRTYPE_ALL;
        case DIMINISHING_SLEEP:
        case DIMINISHING_CONTROL_ROOT:
        case DIMINISHING_TRIGGER_ROOT:
        case DIMINISHING_FEAR:
        case DIMINISHING_CHARM:
        case DIMINISHING_POLYMORPH:
        case DIMINISHING_SILENCE:
        case DIMINISHING_DISARM:
        case DIMINISHING_DEATHCOIL:
        case DIMINISHING_FREEZE:
        case DIMINISHING_BANISH:
        case DIMINISHING_WARLOCK_FEAR:
        case DIMINISHING_KNOCKOUT:
            return DRTYPE_PLAYER;
        default:
            break;
    }

    return DRTYPE_NONE;
}

bool SpellArea::IsFitToRequirements(Player const* player, uint32 newZone, uint32 newArea) const
{
    if(gender!=GENDER_NONE)
    {
        // not in expected gender
        if(!player || gender != player->getGender())
            return false;
    }

    if(raceMask)
    {
        // not in expected race
        if(!player || !(raceMask & player->getRaceMask()))
            return false;
    }

    if(areaId)
    {
        // not in expected zone
        if(newZone!=areaId && newArea!=areaId)
            return false;
    }

    if(questStart)
    {
        // not in expected required quest state
        if(!player || (!questStartCanActive || !player->IsActiveQuest(questStart)) && !player->GetQuestRewardStatus(questStart))
            return false;
    }

    if(questEnd)
    {
        // not in expected forbidden quest state
        if(!player || player->GetQuestRewardStatus(questEnd))
            return false;
    }

    if(auraSpell)
    {
        // not have expected aura
        if(!player)
            return false;
        if(auraSpell > 0)
            // have expected aura
            return player->HasAura(auraSpell, EFFECT_INDEX_0);
        else
            // not have expected aura
            return !player->HasAura(-auraSpell, EFFECT_INDEX_0);
    }

    return true;
}
