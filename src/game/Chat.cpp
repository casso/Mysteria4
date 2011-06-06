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

#include "Chat.h"
#include "Language.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "UpdateMask.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "AccountMgr.h"
#include "SpellMgr.h"
#include "PoolManager.h"
#include "GameEventMgr.h"

// Supported shift-links (client generated and server side)
// |color|Harea:area_id|h[name]|h|r
// |color|Hareatrigger:id|h[name]|h|r
// |color|Hareatrigger_target:id|h[name]|h|r
// |color|Hcreature:creature_guid|h[name]|h|r
// |color|Hcreature_entry:creature_id|h[name]|h|r
// |color|Henchant:recipe_spell_id|h[prof_name: recipe_name]|h|r          - client, at shift click in recipes list dialog
// |color|Hgameevent:id|h[name]|h|r
// |color|Hgameobject:go_guid|h[name]|h|r
// |color|Hgameobject_entry:go_id|h[name]|h|r
// |color|Hitem:item_id:perm_ench_id:gem1:gem2:gem3:0:0:0:0|h[name]|h|r   - client, item icon shift click
// |color|Hitemset:itemset_id|h[name]|h|r
// |color|Hplayer:name|h[name]|h|r                                        - client, in some messages, at click copy only name instead link, so no way generate it in client string send to server
// |color|Hpool:pool_id|h[name]|h|r
// |color|Hquest:quest_id:quest_level|h[name]|h|r                         - client, quest list name shift-click
// |color|Hskill:skill_id|h[name]|h|r
// |color|Hspell:spell_id|h[name]|h|r                                     - client, spellbook spell icon shift-click
// |color|Htalent:talent_id,rank|h[name]|h|r                              - client, talent icon shift-click rank==-1 if shift-copy unlearned talent
// |color|Htaxinode:id|h[name]|h|r
// |color|Htele:id|h[name]|h|r
// |color|Htitle:id|h[name]|h|r

bool ChatHandler::load_command_table = true;

ChatCommand * ChatHandler::getCommandTable()
{
    static ChatCommand accountSetCommandTable[] =
    {
        { "addon",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleAccountSetAddonCommand,     "", NULL },
        { "gmlevel",        SECURITY_CONSOLE,        true,  &ChatHandler::HandleAccountSetGmLevelCommand,   "", NULL },
        { "password",       SECURITY_CONSOLE,        true,  &ChatHandler::HandleAccountSetPasswordCommand,  "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand accountCommandTable[] =
    {
        { "characters",     SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleAccountCharactersCommand,   "", NULL },
        { "create",         SECURITY_CONSOLE,        true,  &ChatHandler::HandleAccountCreateCommand,       "", NULL },
        { "delete",         SECURITY_CONSOLE,        true,  &ChatHandler::HandleAccountDeleteCommand,       "", NULL },
        { "onlinelist",     SECURITY_CONSOLE,        true,  &ChatHandler::HandleAccountOnlineListCommand,   "", NULL },
        { "lock",           SECURITY_PLAYER,         true,  &ChatHandler::HandleAccountLockCommand,         "", NULL },
        { "set",            SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", accountSetCommandTable },
        { "password",       SECURITY_PLAYER,         true,  &ChatHandler::HandleAccountPasswordCommand,     "", NULL },
        { "",               SECURITY_PLAYER,         true,  &ChatHandler::HandleAccountCommand,             "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand auctionCommandTable[] =
    {
        { "alliance",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleAuctionAllianceCommand,     "", NULL },
        { "goblin",         SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleAuctionGoblinCommand,       "", NULL },
        { "horde",          SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleAuctionHordeCommand,        "", NULL },
        { "",               SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleAuctionCommand,             "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand banCommandTable[] =
    {
        { "account",        SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleBanAccountCommand,          "", NULL },
        { "character",      SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleBanCharacterCommand,        "", NULL },
        { "ip",             SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleBanIPCommand,               "", NULL },
        { "mail",           SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleBanMailCommand,             "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand baninfoCommandTable[] =
    {
        { "account",        SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleBanInfoAccountCommand,      "", NULL },
        { "character",      SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleBanInfoCharacterCommand,    "", NULL },
        { "ip",             SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleBanInfoIPCommand,           "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand banlistCommandTable[] =
    {
        { "account",        SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleBanListAccountCommand,      "", NULL },
        { "character",      SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleBanListCharacterCommand,    "", NULL },
        { "ip",             SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleBanListIPCommand,           "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand castCommandTable[] =
    {
        { "back",           SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleCastBackCommand,            "", NULL },
        { "dist",           SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleCastDistCommand,            "", NULL },
        { "self",           SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleCastSelfCommand,            "", NULL },
        { "target",         SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleCastTargetCommand,          "", NULL },
        { "",               SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleCastCommand,                "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand characterDeletedCommandTable[] =
    {
        { "delete",         SECURITY_CONSOLE,        true,  &ChatHandler::HandleCharacterDeletedDeleteCommand, "", NULL },
        { "list",           SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleCharacterDeletedListCommand,"", NULL },
        { "restore",        SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleCharacterDeletedRestoreCommand,"", NULL },
        { "old",            SECURITY_CONSOLE,        true,  &ChatHandler::HandleCharacterDeletedOldCommand, "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand characterCommandTable[] =
    {
        { "deleted",        SECURITY_GAMEMASTER,     true,  NULL,                                           "", characterDeletedCommandTable},
        { "erase",          SECURITY_CONSOLE,        true,  &ChatHandler::HandleCharacterEraseCommand,      "", NULL },
        { "level",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleCharacterLevelCommand,      "", NULL },
        { "rename",         SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleCharacterRenameCommand,     "", NULL },
        { "reputation",     SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleCharacterReputationCommand, "", NULL },
        { "titles",         SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleCharacterTitlesCommand,     "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand debugPlayCommandTable[] =
    {
        { "cinematic",      SECURITY_MODERATOR,      false, &ChatHandler::HandleDebugPlayCinematicCommand,       "", NULL },
        { "sound",          SECURITY_MODERATOR,      false, &ChatHandler::HandleDebugPlaySoundCommand,           "", NULL },
        { NULL,             0,                       false, NULL,                                                "", NULL }
    };

    static ChatCommand debugSendCommandTable[] =
    {
        { "buyerror",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendBuyErrorCommand,        "", NULL },
        { "channelnotify",  SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendChannelNotifyCommand,   "", NULL },
        { "chatmmessage",   SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendChatMsgCommand,         "", NULL },
        { "equiperror",     SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendEquipErrorCommand,      "", NULL },
        { "opcode",         SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendOpcodeCommand,          "", NULL },
        { "poi",            SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendPoiCommand,             "", NULL },
        { "qpartymsg",      SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendQuestPartyMsgCommand,   "", NULL },
        { "qinvalidmsg",    SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendQuestInvalidMsgCommand, "", NULL },
        { "sellerror",      SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendSellErrorCommand,       "", NULL },
        { "spellfail",      SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendSpellFailCommand,       "", NULL },
        { NULL,             0,                       false, NULL,                                                "", NULL }
    };

    static ChatCommand debugCommandTable[] =
    {
        { "anim",           SECURITY_GAMEMASTER,     false, &ChatHandler::HandleDebugAnimCommand,                "", NULL },
        { "arena",          SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugArenaCommand,               "", NULL },
        { "bg",             SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugBattlegroundCommand,        "", NULL },
        { "getitemstate",   SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugGetItemStateCommand,        "", NULL },
        { "lootrecipient",  SECURITY_GAMEMASTER,     false, &ChatHandler::HandleDebugGetLootRecipientCommand,    "", NULL },
        { "getitemvalue",   SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugGetItemValueCommand,        "", NULL },
        { "getvalue",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugGetValueCommand,            "", NULL },
        { "moditemvalue",   SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugModItemValueCommand,        "", NULL },
        { "modvalue",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugModValueCommand,            "", NULL },
        { "play",           SECURITY_MODERATOR,      false, NULL,                                                "", debugPlayCommandTable },
        { "send",           SECURITY_ADMINISTRATOR,  false, NULL,                                                "", debugSendCommandTable },
        { "setaurastate",   SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSetAuraStateCommand,        "", NULL },
        { "setitemvalue",   SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSetItemValueCommand,        "", NULL },
        { "setvalue",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSetValueCommand,            "", NULL },
        { "spellcheck",     SECURITY_CONSOLE,        true,  &ChatHandler::HandleDebugSpellCheckCommand,          "", NULL },
        { "spellmods",      SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSpellModsCommand,           "", NULL },
        { "uws",            SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDebugUpdateWorldStateCommand,    "", NULL },
        { NULL,             0,                       false, NULL,                                                "", NULL }
    };

    static ChatCommand eventCommandTable[] =
    {
        { "list",           SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleEventListCommand,           "", NULL },
        { "start",          SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleEventStartCommand,          "", NULL },
        { "stop",           SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleEventStopCommand,           "", NULL },
        { "",               SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleEventInfoCommand,           "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand gmCommandTable[] =
    {
        { "chat",           SECURITY_MODERATOR,      false, &ChatHandler::HandleGMChatCommand,              "", NULL },
        { "fly",            SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleGMFlyCommand,               "", NULL },
        { "ingame",         SECURITY_PLAYER,         true,  &ChatHandler::HandleGMListIngameCommand,        "", NULL },
        { "list",           SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleGMListFullCommand,          "", NULL },
        { "visible",        SECURITY_MODERATOR,      false, &ChatHandler::HandleGMVisibleCommand,           "", NULL },
        { "",               SECURITY_MODERATOR,      false, &ChatHandler::HandleGMCommand,                  "", NULL },
        { "setview",        SECURITY_MODERATOR,      false, &ChatHandler::HandleSetViewCommand,             "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand goCommandTable[] =
    {
        { "creature",       SECURITY_MODERATOR,      false, &ChatHandler::HandleGoCreatureCommand,          "", NULL },
        { "graveyard",      SECURITY_MODERATOR,      false, &ChatHandler::HandleGoGraveyardCommand,         "", NULL },
        { "grid",           SECURITY_MODERATOR,      false, &ChatHandler::HandleGoGridCommand,              "", NULL },
        { "object",         SECURITY_MODERATOR,      false, &ChatHandler::HandleGoObjectCommand,            "", NULL },
        { "taxinode",       SECURITY_MODERATOR,      false, &ChatHandler::HandleGoTaxinodeCommand,          "", NULL },
        { "trigger",        SECURITY_MODERATOR,      false, &ChatHandler::HandleGoTriggerCommand,           "", NULL },
        { "zonexy",         SECURITY_MODERATOR,      false, &ChatHandler::HandleGoZoneXYCommand,            "", NULL },
        { "xy",             SECURITY_MODERATOR,      false, &ChatHandler::HandleGoXYCommand,                "", NULL },
        { "xyz",            SECURITY_MODERATOR,      false, &ChatHandler::HandleGoXYZCommand,               "", NULL },
        { "",               SECURITY_MODERATOR,      false, &ChatHandler::HandleGoCommand,                  "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand gobjectCommandTable[] =
    {
        { "add",            SECURITY_GAMEMASTER,     false, &ChatHandler::HandleGameObjectAddCommand,       "", NULL },
        { "delete",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandleGameObjectDeleteCommand,    "", NULL },
        { "move",           SECURITY_GAMEMASTER,     false, &ChatHandler::HandleGameObjectMoveCommand,      "", NULL },
        { "near",           SECURITY_GAMEMASTER,     false, &ChatHandler::HandleGameObjectNearCommand,      "", NULL },
        { "target",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandleGameObjectTargetCommand,    "", NULL },
        { "turn",           SECURITY_GAMEMASTER,     false, &ChatHandler::HandleGameObjectTurnCommand,      "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand guildCommandTable[] =
    {
        { "create",         SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleGuildCreateCommand,         "", NULL },
        { "delete",         SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleGuildDeleteCommand,         "", NULL },
        { "invite",         SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleGuildInviteCommand,         "", NULL },
        { "uninvite",       SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleGuildUninviteCommand,       "", NULL },
        { "rank",           SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleGuildRankCommand,           "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand honorCommandTable[] =
    {
        { "add",            SECURITY_GAMEMASTER,     false, &ChatHandler::HandleHonorAddCommand,            "", NULL },
        { "addkill",        SECURITY_GAMEMASTER,     false, &ChatHandler::HandleHonorAddKillCommand,        "", NULL },
        { "update",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandleHonorUpdateCommand,         "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand instanceCommandTable[] =
    {
        { "listbinds",      SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleInstanceListBindsCommand,   "", NULL },
        { "unbind",         SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleInstanceUnbindCommand,      "", NULL },
        { "stats",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleInstanceStatsCommand,       "", NULL },
        { "savedata",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleInstanceSaveDataCommand,    "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand learnCommandTable[] =
    {
        { "all",            SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleLearnAllCommand,            "", NULL },
        { "all_gm",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandleLearnAllGMCommand,          "", NULL },
        { "all_crafts",     SECURITY_GAMEMASTER,     false, &ChatHandler::HandleLearnAllCraftsCommand,      "", NULL },
        { "all_default",    SECURITY_MODERATOR,      false, &ChatHandler::HandleLearnAllDefaultCommand,     "", NULL },
        { "all_lang",       SECURITY_MODERATOR,      false, &ChatHandler::HandleLearnAllLangCommand,        "", NULL },
        { "all_myclass",    SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleLearnAllMyClassCommand,     "", NULL },
        { "all_myspells",   SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleLearnAllMySpellsCommand,    "", NULL },
        { "all_mytalents",  SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleLearnAllMyTalentsCommand,   "", NULL },
        { "all_recipes",    SECURITY_GAMEMASTER,     false, &ChatHandler::HandleLearnAllRecipesCommand,     "", NULL },
        { "",               SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleLearnCommand,               "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand listCommandTable[] =
    {
        { "auras",          SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleListAurasCommand,           "", NULL },
        { "creature",       SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleListCreatureCommand,        "", NULL },
        { "item",           SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleListItemCommand,            "", NULL },
        { "object",         SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleListObjectCommand,          "", NULL },
        { "talents",        SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleListTalentsCommand,         "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand lookupAccountCommandTable[] =
    {
        { "email",          SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleLookupAccountEmailCommand,  "", NULL },
        { "ip",             SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleLookupAccountIpCommand,     "", NULL },
        { "name",           SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleLookupAccountNameCommand,   "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand lookupPlayerCommandTable[] =
    {
        { "account",        SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleLookupPlayerAccountCommand, "", NULL },
        { "email",          SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleLookupPlayerEmailCommand,   "", NULL },
        { "ip",             SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleLookupPlayerIpCommand,      "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand lookupCommandTable[] =
    {
        { "account",        SECURITY_GAMEMASTER,     true,  NULL,                                           "", lookupAccountCommandTable },
        { "area",           SECURITY_MODERATOR,      true,  &ChatHandler::HandleLookupAreaCommand,          "", NULL },
        { "creature",       SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupCreatureCommand,      "", NULL },
        { "event",          SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleLookupEventCommand,         "", NULL },
        { "faction",        SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupFactionCommand,       "", NULL },
        { "item",           SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupItemCommand,          "", NULL },
        { "itemset",        SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupItemSetCommand,       "", NULL },
        { "object",         SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupObjectCommand,        "", NULL },
        { "quest",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupQuestCommand,         "", NULL },
        { "player",         SECURITY_GAMEMASTER,     true,  NULL,                                           "", lookupPlayerCommandTable },
        { "pool",           SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleLookupPoolCommand,          "", NULL },
        { "skill",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupSkillCommand,         "", NULL },
        { "spell",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupSpellCommand,         "", NULL },
        { "taxinode",       SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupTaxiNodeCommand,      "", NULL },
        { "tele",           SECURITY_MODERATOR,      true,  &ChatHandler::HandleLookupTeleCommand,          "", NULL },
        { "title",          SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleLookupTitleCommand,         "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand modifyCommandTable[] =
    {
        { "hp",             SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyHPCommand,            "", NULL },
        { "mana",           SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyManaCommand,          "", NULL },
        { "rage",           SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyRageCommand,          "", NULL },
        { "energy",         SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyEnergyCommand,        "", NULL },
        { "money",          SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyMoneyCommand,         "", NULL },
        { "speed",          SECURITY_MODERATOR,      false, &ChatHandler::HandleModifySpeedCommand,         "", NULL },
        { "swim",           SECURITY_MODERATOR,      false, &ChatHandler::HandleModifySwimCommand,          "", NULL },
        { "scale",          SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyScaleCommand,         "", NULL },
        { "bwalk",          SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyBWalkCommand,         "", NULL },
        { "fly",            SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyFlyCommand,           "", NULL },
        { "aspeed",         SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyASpeedCommand,        "", NULL },
        { "faction",        SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyFactionCommand,       "", NULL },
        { "tp",             SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyTalentCommand,        "", NULL },
        { "mount",          SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyMountCommand,         "", NULL },
        { "honor",          SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyHonorCommand,         "", NULL },
        { "rep",            SECURITY_GAMEMASTER,     false, &ChatHandler::HandleModifyRepCommand,           "", NULL },
        { "arena",          SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyArenaCommand,         "", NULL },
        { "drunk",          SECURITY_MODERATOR,      false, &ChatHandler::HandleModifyDrunkCommand,         "", NULL },
        { "standstate",     SECURITY_GAMEMASTER,     false, &ChatHandler::HandleModifyStandStateCommand,    "", NULL },
        { "morph",          SECURITY_GAMEMASTER,     false, &ChatHandler::HandleModifyMorphCommand,         "", NULL },
        { "gender",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandleModifyGenderCommand,        "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand npcCommandTable[] =
    {
        { "add",            SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcAddCommand,              "", NULL },
        { "additem",        SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcAddVendorItemCommand,    "", NULL },
        { "addmove",        SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcAddMoveCommand,          "", NULL },
        { "allowmove",      SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleNpcAllowMovementCommand,    "", NULL },
        { "changeentry",    SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleNpcChangeEntryCommand,      "", NULL },
        { "changelevel",    SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcChangeLevelCommand,      "", NULL },
        { "delete",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcDeleteCommand,           "", NULL },
        { "delitem",        SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcDelVendorItemCommand,    "", NULL },
        { "factionid",      SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcFactionIdCommand,        "", NULL },
        { "flag",           SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcFlagCommand,             "", NULL },
        { "follow",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcFollowCommand,           "", NULL },
        { "info",           SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleNpcInfoCommand,             "", NULL },
        { "move",           SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcMoveCommand,             "", NULL },
        { "playemote",      SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleNpcPlayEmoteCommand,        "", NULL },
        { "setmodel",       SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcSetModelCommand,         "", NULL },
        { "setmovetype",    SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcSetMoveTypeCommand,      "", NULL },
        { "spawndist",      SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcSpawnDistCommand,        "", NULL },
        { "spawntime",      SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcSpawnTimeCommand,        "", NULL },
        { "say",            SECURITY_MODERATOR,      false, &ChatHandler::HandleNpcSayCommand,              "", NULL },
        { "textemote",      SECURITY_MODERATOR,      false, &ChatHandler::HandleNpcTextEmoteCommand,        "", NULL },
        { "unfollow",       SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcUnFollowCommand,         "", NULL },
        { "whisper",        SECURITY_MODERATOR,      false, &ChatHandler::HandleNpcWhisperCommand,          "", NULL },
        { "yell",           SECURITY_MODERATOR,      false, &ChatHandler::HandleNpcYellCommand,             "", NULL },
        { "tame",           SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcTameCommand,             "", NULL },
        { "setdeathstate",  SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcSetDeathStateCommand,    "", NULL },

        //{ TODO: fix or remove this commands
        { "addweapon",      SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleNpcAddWeaponCommand,        "", NULL },
        { "name",           SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcNameCommand,             "", NULL },
        { "subname",        SECURITY_GAMEMASTER,     false, &ChatHandler::HandleNpcSubNameCommand,          "", NULL },
        //}

        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand pdumpCommandTable[] =
    {
        { "load",           SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandlePDumpLoadCommand,           "", NULL },
        { "write",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandlePDumpWriteCommand,          "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand poolCommandTable[] =
    {
        { "list",           SECURITY_GAMEMASTER,     false, &ChatHandler::HandlePoolListCommand,            "", NULL },
        { "spawns",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandlePoolSpawnsCommand,          "", NULL },
        { "",               SECURITY_GAMEMASTER,     true,  &ChatHandler::HandlePoolInfoCommand,            "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand questCommandTable[] =
    {
        { "add",            SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleQuestAddCommand,            "", NULL },
        { "complete",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleQuestCompleteCommand,       "", NULL },
        { "remove",         SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleQuestRemoveCommand,         "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand reloadCommandTable[] =
    {
        { "all",            SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllCommand,           "", NULL },
        { "all_area",       SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllAreaCommand,       "", NULL },
        { "all_eventai",    SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllEventAICommand,    "", NULL },
        { "all_gossips",    SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllGossipsCommand,    "", NULL },
        { "all_item",       SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllItemCommand,       "", NULL },
        { "all_locales",    SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllLocalesCommand,    "", NULL },
        { "all_loot",       SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllLootCommand,       "", NULL },
        { "all_npc",        SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllNpcCommand,        "", NULL },
        { "all_quest",      SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllQuestCommand,      "", NULL },
        { "all_scripts",    SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllScriptsCommand,    "", NULL },
        { "all_spell",      SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllSpellCommand,      "", NULL },

        { "config",         SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadConfigCommand,        "", NULL },

        { "areatrigger_involvedrelation",SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadQuestAreaTriggersCommand,       "", NULL },
        { "areatrigger_tavern",          SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadAreaTriggerTavernCommand,       "", NULL },
        { "areatrigger_teleport",        SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadAreaTriggerTeleportCommand,     "", NULL },
        { "command",                     SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadCommandCommand,                 "", NULL },
        { "creature_ai_scripts",         SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadEventAIScriptsCommand,          "", NULL },
        { "creature_ai_summons",         SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadEventAISummonsCommand,          "", NULL },
        { "creature_ai_texts",           SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadEventAITextsCommand,            "", NULL },
        { "creature_battleground",       SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadBattleEventCommand,             "", NULL },
        { "creature_involvedrelation",   SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadCreatureQuestInvRelationsCommand,"",NULL },
        { "creature_loot_template",      SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesCreatureCommand,   "", NULL },
        { "creature_questrelation",      SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadCreatureQuestRelationsCommand,  "", NULL },
        { "db_script_string",            SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadDbScriptStringCommand,          "", NULL },
        { "disenchant_loot_template",    SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesDisenchantCommand, "", NULL },
        { "event_scripts",               SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadEventScriptsCommand,            "", NULL },
        { "fishing_loot_template",       SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesFishingCommand,    "", NULL },
        { "game_graveyard_zone",         SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGameGraveyardZoneCommand,       "", NULL },
        { "game_tele",                   SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGameTeleCommand,                "", NULL },
        { "gameobject_involvedrelation", SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGOQuestInvRelationsCommand,     "", NULL },
        { "gameobject_loot_template",    SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesGameobjectCommand, "", NULL },
        { "gameobject_questrelation",    SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGOQuestRelationsCommand,        "", NULL },
        { "gameobject_scripts",          SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGameObjectScriptsCommand,       "", NULL },
        { "gameobject_battleground",     SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadBattleEventCommand,             "", NULL },
        { "gossip_menu",                 SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGossipMenuCommand,              "", NULL },
        { "gossip_menu_option",          SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGossipMenuOptionCommand,        "", NULL },
        { "gossip_scripts",              SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGossipScriptsCommand,           "", NULL },
        { "item_enchantment_template",   SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadItemEnchantementsCommand,       "", NULL },
        { "item_loot_template",          SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesItemCommand,       "", NULL },
        { "item_required_target",        SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadItemRequiredTragetCommand,      "", NULL },
        { "locales_creature",            SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesCreatureCommand,         "", NULL },
        { "locales_gameobject",          SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesGameobjectCommand,       "", NULL },
        { "locales_gossip_menu_option",  SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesGossipMenuOptionCommand, "", NULL },
        { "locales_item",                SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesItemCommand,             "", NULL },
        { "locales_npc_text",            SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesNpcTextCommand,          "", NULL },
        { "locales_page_text",           SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesPageTextCommand,         "", NULL },
        { "locales_points_of_interest",  SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesPointsOfInterestCommand, "", NULL },
        { "locales_quest",               SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesQuestCommand,            "", NULL },
        { "mail_level_reward",           SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadMailLevelRewardCommand,         "", NULL },
        { "mail_loot_template",          SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesMailCommand,       "", NULL },
        { "mangos_string",               SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadMangosStringCommand,            "", NULL },
        { "npc_gossip",                  SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadNpcGossipCommand,               "", NULL },
        { "npc_text",                    SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadNpcTextCommand,                 "", NULL },
        { "npc_trainer",                 SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadNpcTrainerCommand,              "", NULL },
        { "npc_vendor",                  SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadNpcVendorCommand,               "", NULL },
        { "page_text",                   SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadPageTextsCommand,               "", NULL },
        { "pickpocketing_loot_template", SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesPickpocketingCommand,"",NULL},
        { "points_of_interest",          SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadPointsOfInterestCommand,        "", NULL },
        { "prospecting_loot_template",   SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesProspectingCommand,"", NULL },
        { "quest_end_scripts",           SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadQuestEndScriptsCommand,         "", NULL },
        { "quest_start_scripts",         SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadQuestStartScriptsCommand,       "", NULL },
        { "quest_template",              SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadQuestTemplateCommand,           "", NULL },
        { "reference_loot_template",     SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesReferenceCommand,  "", NULL },
        { "reserved_name",               SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadReservedNameCommand,            "", NULL },
        { "reputation_reward_rate",      SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadReputationRewardRateCommand,    "", NULL },
        { "reputation_spillover_template",SECURITY_ADMINISTRATOR, true, &ChatHandler::HandleReloadReputationSpilloverTemplateCommand,"", NULL },
        { "skill_discovery_template",    SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSkillDiscoveryTemplateCommand,  "", NULL },
        { "skill_extra_item_template",   SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSkillExtraItemTemplateCommand,  "", NULL },
        { "skill_fishing_base_level",    SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSkillFishingBaseLevelCommand,   "", NULL },
        { "skinning_loot_template",      SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesSkinningCommand,   "", NULL },
        { "spell_affect",                SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellAffectCommand,             "", NULL },
        { "spell_area",                  SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellAreaCommand,               "", NULL },
        { "spell_bonus_data",            SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellBonusesCommand,            "", NULL },
        { "spell_chain",                 SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellChainCommand,              "", NULL },
        { "spell_elixir",                SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellElixirCommand,             "", NULL },
        { "spell_learn_spell",           SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellLearnSpellCommand,         "", NULL },
        { "spell_pet_auras",             SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellPetAurasCommand,           "", NULL },
        { "spell_proc_event",            SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellProcEventCommand,          "", NULL },
        { "spell_proc_item_enchant",     SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellProcItemEnchantCommand,    "", NULL },
        { "spell_script_target",         SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellScriptTargetCommand,       "", NULL },
        { "spell_scripts",               SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellScriptsCommand,            "", NULL },
        { "spell_target_position",       SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellTargetPositionCommand,     "", NULL },
        { "spell_threats",               SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellThreatsCommand,            "", NULL },
        { "dbc",                         SECURITY_ADMINISTRATOR, true,  &ChatHandler::HandleReloadDBCCommand,                     "", NULL },

        { NULL,                          0,                      false, NULL,                                                     "", NULL }
    };

    static ChatCommand resetCommandTable[] =
    {
        { "honor",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleResetHonorCommand,          "", NULL },
        { "level",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleResetLevelCommand,          "", NULL },
        { "spells",         SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleResetSpellsCommand,         "", NULL },
        { "stats",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleResetStatsCommand,          "", NULL },
        { "talents",        SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleResetTalentsCommand,        "", NULL },
        { "all",            SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleResetAllCommand,            "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand sendMassCommandTable[] =
    {
        { "items",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleSendMassItemsCommand,       "", NULL },
        { "mail",           SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleSendMassMailCommand,        "", NULL },
        { "money",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleSendMassMoneyCommand,       "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand sendCommandTable[] =
    {
        { "mass",           SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", sendMassCommandTable },

        { "items",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleSendItemsCommand,           "", NULL },
        { "mail",           SECURITY_MODERATOR,      true,  &ChatHandler::HandleSendMailCommand,            "", NULL },
        { "message",        SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleSendMessageCommand,         "", NULL },
        { "money",          SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleSendMoneyCommand,           "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand serverIdleRestartCommandTable[] =
    {
        { "cancel",         SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleServerShutDownCancelCommand,"", NULL },
        { ""   ,            SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleServerIdleRestartCommand,   "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand serverIdleShutdownCommandTable[] =
    {
        { "cancel",         SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleServerShutDownCancelCommand,"", NULL },
        { ""   ,            SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleServerIdleShutDownCommand,  "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand serverRestartCommandTable[] =
    {
        { "cancel",         SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleServerShutDownCancelCommand,"", NULL },
        { ""   ,            SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleServerRestartCommand,       "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand serverShutdownCommandTable[] =
    {
        { "cancel",         SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleServerShutDownCancelCommand,"", NULL },
        { ""   ,            SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleServerShutDownCommand,      "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand serverLogCommandTable[] =
    {
        { "filter",         SECURITY_CONSOLE,        true,  &ChatHandler::HandleServerLogFilterCommand,     "", NULL },
        { "level",          SECURITY_CONSOLE,        true,  &ChatHandler::HandleServerLogLevelCommand,      "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand serverSetCommandTable[] =
    {
        { "motd",           SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleServerSetMotdCommand,       "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand serverCommandTable[] =
    {
        { "corpses",        SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleServerCorpsesCommand,       "", NULL },
        { "exit",           SECURITY_CONSOLE,        true,  &ChatHandler::HandleServerExitCommand,          "", NULL },
        { "idlerestart",    SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", serverIdleRestartCommandTable },
        { "idleshutdown",   SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", serverShutdownCommandTable },
        { "info",           SECURITY_PLAYER,         true,  &ChatHandler::HandleServerInfoCommand,          "", NULL },
        { "log",            SECURITY_CONSOLE,        true,  NULL,                                           "", serverLogCommandTable },
        { "motd",           SECURITY_PLAYER,         true,  &ChatHandler::HandleServerMotdCommand,          "", NULL },
        { "plimit",         SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleServerPLimitCommand,        "", NULL },
        { "restart",        SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", serverRestartCommandTable },
        { "shutdown",       SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", serverShutdownCommandTable },
        { "set",            SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", serverSetCommandTable },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand teleCommandTable[] =
    {
        { "add",            SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleTeleAddCommand,             "", NULL },
        { "del",            SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleTeleDelCommand,             "", NULL },
        { "name",           SECURITY_MODERATOR,      true,  &ChatHandler::HandleTeleNameCommand,            "", NULL },
        { "group",          SECURITY_MODERATOR,      false, &ChatHandler::HandleTeleGroupCommand,           "", NULL },
        { "",               SECURITY_MODERATOR,      false, &ChatHandler::HandleTeleCommand,                "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand titlesCommandTable[] =
    {
        { "add",            SECURITY_GAMEMASTER,     false, &ChatHandler::HandleTitlesAddCommand,           "", NULL },
        { "current",        SECURITY_GAMEMASTER,     false, &ChatHandler::HandleTitlesCurrentCommand,       "", NULL },
        { "remove",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandleTitlesRemoveCommand,        "", NULL },
        { "setmask",        SECURITY_GAMEMASTER,     false, &ChatHandler::HandleTitlesSetMaskCommand,       "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand triggerCommandTable[] =
    {
        { "active",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandleTriggerActiveCommand,       "", NULL },
        { "near",           SECURITY_GAMEMASTER,     false, &ChatHandler::HandleTriggerNearCommand,         "", NULL },
        { "",               SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleTriggerCommand,             "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand unbanCommandTable[] =
    {
        { "account",        SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleUnBanAccountCommand,      "", NULL },
        { "character",      SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleUnBanCharacterCommand,    "", NULL },
        { "ip",             SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleUnBanIPCommand,           "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand wpCommandTable[] =
    {
        { "show",           SECURITY_GAMEMASTER,     false, &ChatHandler::HandleWpShowCommand,              "", NULL },
        { "add",            SECURITY_GAMEMASTER,     false, &ChatHandler::HandleWpAddCommand,               "", NULL },
        { "modify",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandleWpModifyCommand,            "", NULL },
        { "export",         SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleWpExportCommand,            "", NULL },
        { "import",         SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleWpImportCommand,            "", NULL },
        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    static ChatCommand commandTable[] =
    {
        { "account",        SECURITY_PLAYER,         true,  NULL,                                           "", accountCommandTable  },
        { "auction",        SECURITY_ADMINISTRATOR,  false, NULL,                                           "", auctionCommandTable  },
        { "cast",           SECURITY_ADMINISTRATOR,  false, NULL,                                           "", castCommandTable     },
        { "character",      SECURITY_GAMEMASTER,     true,  NULL,                                           "", characterCommandTable},
        { "debug",          SECURITY_MODERATOR,      true,  NULL,                                           "", debugCommandTable    },
        { "event",          SECURITY_GAMEMASTER,     false, NULL,                                           "", eventCommandTable    },
        { "gm",             SECURITY_PLAYER,         true,  NULL,                                           "", gmCommandTable       },
        { "honor",          SECURITY_GAMEMASTER,     false, NULL,                                           "", honorCommandTable    },
        { "go",             SECURITY_MODERATOR,      false, NULL,                                           "", goCommandTable       },
        { "gobject",        SECURITY_GAMEMASTER,     false, NULL,                                           "", gobjectCommandTable  },
        { "guild",          SECURITY_GAMEMASTER,     true,  NULL,                                           "", guildCommandTable    },
        { "instance",       SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", instanceCommandTable },
        { "learn",          SECURITY_MODERATOR,      false, NULL,                                           "", learnCommandTable    },
        { "list",           SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", listCommandTable     },
        { "lookup",         SECURITY_MODERATOR,      true,  NULL,                                           "", lookupCommandTable   },
        { "modify",         SECURITY_MODERATOR,      false, NULL,                                           "", modifyCommandTable   },
        { "npc",            SECURITY_MODERATOR,      false, NULL,                                           "", npcCommandTable      },
        { "pool",           SECURITY_GAMEMASTER,     true,  NULL,                                           "", poolCommandTable     },
        { "pdump",          SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", pdumpCommandTable    },
        { "quest",          SECURITY_ADMINISTRATOR,  false, NULL,                                           "", questCommandTable    },
        { "reload",         SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", reloadCommandTable   },
        { "reset",          SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", resetCommandTable    },
        { "server",         SECURITY_PLAYER,         true,  NULL,                                           "", serverCommandTable   },
        { "tele",           SECURITY_MODERATOR,      true,  NULL,                                           "", teleCommandTable     },
        { "titles",         SECURITY_GAMEMASTER,     false, NULL,                                           "", titlesCommandTable   },
        { "trigger",        SECURITY_GAMEMASTER,     false, NULL,                                           "", triggerCommandTable  },
        { "wp",             SECURITY_GAMEMASTER,     false, NULL,                                           "", wpCommandTable       },

        { "aura",           SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleAuraCommand,                "", NULL },
        { "unaura",         SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleUnAuraCommand,              "", NULL },
        { "announce",       SECURITY_MODERATOR,      true,  &ChatHandler::HandleAnnounceCommand,            "", NULL },
        { "notify",         SECURITY_MODERATOR,      true,  &ChatHandler::HandleNotifyCommand,              "", NULL },
        { "goname",         SECURITY_MODERATOR,      false, &ChatHandler::HandleGonameCommand,              "", NULL },
        { "namego",         SECURITY_MODERATOR,      false, &ChatHandler::HandleNamegoCommand,              "", NULL },
        { "groupgo",        SECURITY_MODERATOR,      false, &ChatHandler::HandleGroupgoCommand,             "", NULL },
        { "commands",       SECURITY_PLAYER,         true,  &ChatHandler::HandleCommandsCommand,            "", NULL },
        { "demorph",        SECURITY_GAMEMASTER,     false, &ChatHandler::HandleDeMorphCommand,             "", NULL },
        { "die",            SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDieCommand,                 "", NULL },
        { "revive",         SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleReviveCommand,              "", NULL },
        { "dismount",       SECURITY_PLAYER,         false, &ChatHandler::HandleDismountCommand,            "", NULL },
        { "gps",            SECURITY_MODERATOR,      false, &ChatHandler::HandleGPSCommand,                 "", NULL },
        { "guid",           SECURITY_GAMEMASTER,     false, &ChatHandler::HandleGUIDCommand,                "", NULL },
        { "help",           SECURITY_PLAYER,         true,  &ChatHandler::HandleHelpCommand,                "", NULL },
        { "itemmove",       SECURITY_GAMEMASTER,     false, &ChatHandler::HandleItemMoveCommand,            "", NULL },
        { "cooldown",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleCooldownCommand,            "", NULL },
        { "unlearn",        SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleUnLearnCommand,             "", NULL },
        { "distance",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleGetDistanceCommand,         "", NULL },
        { "recall",         SECURITY_MODERATOR,      false, &ChatHandler::HandleRecallCommand,              "", NULL },
        { "save",           SECURITY_PLAYER,         false, &ChatHandler::HandleSaveCommand,                "", NULL },
        { "saveall",        SECURITY_MODERATOR,      true,  &ChatHandler::HandleSaveAllCommand,             "", NULL },
        { "kick",           SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleKickPlayerCommand,          "", NULL },
        { "ban",            SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", banCommandTable      },
        { "unban",          SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", unbanCommandTable    },
        { "baninfo",        SECURITY_ADMINISTRATOR,  false, NULL,                                           "", baninfoCommandTable  },
        { "banlist",        SECURITY_ADMINISTRATOR,  true,  NULL,                                           "", banlistCommandTable  },
        { "start",          SECURITY_PLAYER,         false, &ChatHandler::HandleStartCommand,               "", NULL },
        { "taxicheat",      SECURITY_MODERATOR,      false, &ChatHandler::HandleTaxiCheatCommand,           "", NULL },
        { "linkgrave",      SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleLinkGraveCommand,           "", NULL },
        { "neargrave",      SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleNearGraveCommand,           "", NULL },
        { "explorecheat",   SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleExploreCheatCommand,        "", NULL },
        { "hover",          SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleHoverCommand,               "", NULL },
        { "levelup",        SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleLevelUpCommand,             "", NULL },
        { "showarea",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleShowAreaCommand,            "", NULL },
        { "hidearea",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleHideAreaCommand,            "", NULL },
        { "additem",        SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleAddItemCommand,             "", NULL },
        { "additemset",     SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleAddItemSetCommand,          "", NULL },
        { "bank",           SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleBankCommand,                "", NULL },
        { "wchange",        SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleChangeWeatherCommand,       "", NULL },
        { "ticket",         SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleTicketCommand,              "", NULL },
        { "delticket",      SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleDelTicketCommand,           "", NULL },
        { "maxskill",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleMaxSkillCommand,            "", NULL },
        { "setskill",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleSetSkillCommand,            "", NULL },
        { "whispers",       SECURITY_MODERATOR,      false, &ChatHandler::HandleWhispersCommand,            "", NULL },
        { "pinfo",          SECURITY_GAMEMASTER,     true,  &ChatHandler::HandlePInfoCommand,               "", NULL },
        { "respawn",        SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleRespawnCommand,             "", NULL },
        { "send",           SECURITY_MODERATOR,      true,  NULL,                                           "", sendCommandTable     },
        { "loadscripts",    SECURITY_ADMINISTRATOR,  true,  &ChatHandler::HandleLoadScriptsCommand,         "", NULL },
        { "mute",           SECURITY_MODERATOR,      true,  &ChatHandler::HandleMuteCommand,                "", NULL },
        { "unmute",         SECURITY_MODERATOR,      true,  &ChatHandler::HandleUnmuteCommand,              "", NULL },
        { "movegens",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleMovegensCommand,            "", NULL },
        { "cometome",       SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleComeToMeCommand,            "", NULL },
        { "damage",         SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleDamageCommand,              "", NULL },
        { "combatstop",     SECURITY_GAMEMASTER,     false, &ChatHandler::HandleCombatStopCommand,          "", NULL },
        { "flusharenapoints",SECURITY_ADMINISTRATOR, false, &ChatHandler::HandleFlushArenaPointsCommand,    "", NULL },
        { "repairitems",    SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleRepairitemsCommand,         "", NULL },
        { "stable",         SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleStableCommand,              "", NULL },
        { "waterwalk",      SECURITY_GAMEMASTER,     false, &ChatHandler::HandleWaterwalkCommand,           "", NULL },
        { "quit",           SECURITY_CONSOLE,        true,  &ChatHandler::HandleQuitCommand,                "", NULL },

        // Vlastne prikazy
        { "additemtoall",   SECURITY_ADMINISTRATOR,  false, &ChatHandler::HandleAddItemToAllCommand,        "", NULL },
        { "wsg",            SECURITY_VIP,            false, &ChatHandler::HandleWSGCommand,                 "", NULL },
        { "ab",             SECURITY_VIP,            false, &ChatHandler::HandleABCommand,                  "", NULL },
        { "av",             SECURITY_VIP,            false, &ChatHandler::HandleAVCommand,                  "", NULL },
        { "eos",            SECURITY_VIP,            false, &ChatHandler::HandleEOSCommand,                 "", NULL },
        { "freeze",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandleFreezeCommand,              "", NULL },
        { "unfreeze",       SECURITY_GAMEMASTER,     false, &ChatHandler::HandleUnFreezeCommand,            "", NULL },
        { "listfreeze",     SECURITY_GAMEMASTER,     false, &ChatHandler::HandleListFreezeCommand,          "", NULL },
        { "viponline",      SECURITY_GAMEMASTER,     true,  &ChatHandler::HandleVIPOnlineCommand,           "", NULL },
        { "mc",             SECURITY_VIP,            false, &ChatHandler::HandleMCCommand,                  "", NULL },
        { "unsick",         SECURITY_GAMEMASTER,     false, &ChatHandler::HandleUnsicknessCommand,          "", NULL },
        { "dtoken",         SECURITY_EVENTMASTER,    false, &ChatHandler::HandleDungeonTokenCommand,        "", NULL },
        { "dmark",          SECURITY_EVENTMASTER,    false, &ChatHandler::HandleDungeonMarkCommand,         "", NULL },
        { "htoken",         SECURITY_EVENTMASTER,    false, &ChatHandler::HandleHonorTokenCommand,          "", NULL },
        { "hmark",          SECURITY_EVENTMASTER,    false, &ChatHandler::HandleHonorMarkCommand,           "", NULL },
        { "gocheater",      SECURITY_GAMEMASTER,     false, &ChatHandler::HandleGoCheaterCommand,           "", NULL },
        { "gh",             SECURITY_VIP,            false, &ChatHandler::HandleGuildHouseCommand,          "", NULL },
        { "playsoundtoall", SECURITY_GAMEMASTER,     false, &ChatHandler::HandlePlaySoundToAllCommand,      "", NULL },

        { NULL,             0,                       false, NULL,                                           "", NULL }
    };

    if(load_command_table)
    {
        load_command_table = false;

        // check hardcoded part integrity
        CheckIntegrity(commandTable, NULL);

        QueryResult *result = WorldDatabase.Query("SELECT name,security,help FROM command");
        if (result)
        {
            do
            {
                Field *fields = result->Fetch();
                std::string name = fields[0].GetCppString();

                SetDataForCommandInTable(commandTable, name.c_str(), fields[1].GetUInt16(), fields[2].GetCppString());

            } while(result->NextRow());
            delete result;
        }
    }

    return commandTable;
}

ChatHandler::ChatHandler(WorldSession* session) : m_session(session) {}

ChatHandler::ChatHandler(Player* player) : m_session(player->GetSession()) {}

ChatHandler::~ChatHandler() {}

const char *ChatHandler::GetMangosString(int32 entry) const
{
    return m_session->GetMangosString(entry);
}

uint32 ChatHandler::GetAccountId() const
{
    return m_session->GetAccountId();
}

AccountTypes ChatHandler::GetAccessLevel() const
{
    return m_session->GetSecurity();
}

bool ChatHandler::isAvailable(ChatCommand const& cmd) const
{
    // check security level only for simple  command (without child commands)
    return GetAccessLevel() >= (AccountTypes)cmd.SecurityLevel;
}

std::string ChatHandler::GetNameLink() const
{
    return GetNameLink(m_session->GetPlayer());
}

bool ChatHandler::HasLowerSecurity(Player* target, ObjectGuid guid, bool strong)
{
    WorldSession* target_session = NULL;
    uint32 target_account = 0;

    if (target)
        target_session = target->GetSession();
    else if (!guid.IsEmpty())
        target_account = sObjectMgr.GetPlayerAccountIdByGUID(guid);

    if(!target_session && !target_account)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return true;
    }

    return HasLowerSecurityAccount(target_session,target_account,strong);
}

bool ChatHandler::HasLowerSecurityAccount(WorldSession* target, uint32 target_account, bool strong)
{
    AccountTypes target_sec;

    // ignore only for non-players for non strong checks (when allow apply command at least to same sec level)
    if (GetAccessLevel() > SECURITY_VIP && !strong && !sWorld.getConfig(CONFIG_BOOL_GM_LOWER_SECURITY))
        return false;

    if (target)
        target_sec = target->GetSecurity();
    else if (target_account)
        target_sec = sAccountMgr.GetSecurity(target_account);
    else
        return true;                                        // caller must report error for (target==NULL && target_account==0)

    if (GetAccessLevel() < target_sec || (strong && GetAccessLevel() <= target_sec))
    {
        SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage(true);
        return true;
    }

    return false;
}

bool ChatHandler::hasStringAbbr(const char* name, const char* part)
{
    // non "" command
    if( *name )
    {
        // "" part from non-"" command
        if( !*part )
            return false;

        for(;;)
        {
            if( !*part )
                return true;
            else if( !*name )
                return false;
            else if( tolower( *name ) != tolower( *part ) )
                return false;
            ++name; ++part;
        }
    }
    // allow with any for ""

    return true;
}

void ChatHandler::SendSysMessage(const char *str)
{
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = mangos_strdup(str);
    char* pos = buf;

    while(char* line = LineFromMessage(pos))
    {
        FillSystemMessageData(&data, line);
        m_session->SendPacket(&data);
    }

    delete [] buf;
}

void ChatHandler::SendGlobalSysMessage(const char *str)
{
    // Chat output
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = mangos_strdup(str);
    char* pos = buf;

    while(char* line = LineFromMessage(pos))
    {
        FillSystemMessageData(&data, line);
        sWorld.SendGlobalMessage(&data);
    }

    delete [] buf;
}

void ChatHandler::SendSysMessage(int32 entry)
{
    SendSysMessage(GetMangosString(entry));
}

void ChatHandler::PSendSysMessage(int32 entry, ...)
{
    const char *format = GetMangosString(entry);
    va_list ap;
    char str [2048];
    va_start(ap, entry);
    vsnprintf(str,2048,format, ap );
    va_end(ap);
    SendSysMessage(str);
}

void ChatHandler::PSendSysMessage(const char *format, ...)
{
    va_list ap;
    char str [2048];
    va_start(ap, format);
    vsnprintf(str,2048,format, ap );
    va_end(ap);
    SendSysMessage(str);
}

void ChatHandler::CheckIntegrity( ChatCommand *table, ChatCommand *parentCommand )
{
    for(uint32 i = 0; table[i].Name != NULL; ++i)
    {
        ChatCommand* command = &table[i];

        if (parentCommand && command->SecurityLevel < parentCommand->SecurityLevel)
            sLog.outBasic("Subcommand '%s' of command '%s' have less access level (%u) that parent (%u)",
                command->Name, parentCommand->Name, command->SecurityLevel, parentCommand->SecurityLevel);

        if (!parentCommand && strlen(command->Name)==0)
            sLog.outError("Subcommand '' at top level");

        if (command->ChildCommands)
        {
            if (command->Handler)
            {
                if (parentCommand)
                    sLog.outError("Subcommand '%s' of command '%s' have handler and subcommands in same time, must be used '' subcommand for handler instead.",
                        command->Name, parentCommand->Name);
                else
                    sLog.outError("First level command '%s' have handler and subcommands in same time, must be used '' subcommand for handler instead.",
                        command->Name);
            }

            if (parentCommand && strlen(command->Name)==0)
                sLog.outError("Subcommand '' of command '%s' have subcommands", parentCommand->Name);

            CheckIntegrity(command->ChildCommands, command);
        }
        else if (!command->Handler)
        {
            if (parentCommand)
                sLog.outError("Subcommand '%s' of command '%s' not have handler and subcommands in same time. Must have some from its!",
                    command->Name, parentCommand->Name);
            else
                sLog.outError("First level command '%s' not have handler and subcommands in same time. Must have some from its!",
                    command->Name);
        }
    }
}

/**
 * Search (sub)command for command line available for chat handler access level
 *
 * @param text  Command line string that will parsed for (sub)command search
 *
 * @return Pointer to found command structure or NULL if appropriate command not found
 */
ChatCommand const* ChatHandler::FindCommand(char const* text)
{
    ChatCommand* command = NULL;
    char const* textPtr = text;

    return FindCommand(getCommandTable(), textPtr, command) == CHAT_COMMAND_OK ? command : NULL;
}

/**
 * Search (sub)command for command line available for chat handler access level with options and fail case additional info
 *
 * @param table         Pointer to command C-style array first level command where will be searched
 * @param text          Command line string that will parsed for (sub)command search,
 *                      it modified at return from function and pointed to not parsed tail
 * @param command       At success this is found command, at other cases this is last found parent command
 *                      before subcommand search fail
 * @param parentCommand Output arg for optional return parent command for command arg.
 * @param cmdNamePtr    Output arg for optional return last parsed command name.
 * @param allAvailable  Optional arg (with false default value) control use command access level checks while command search.
 * @param exactlyName   Optional arg (with false default value) control use exactly name in checks while command search.
 *
 * @return one from enum value of ChatCommandSearchResult. Output args return values highly dependent from this return result:
 *
 *      CHAT_COMMAND_OK       - Command found!
 *                              text point to non parsed tail with possible command specific data, command store found command pointer,
 *                              parentCommand have parent of found command or NULL if command found in table array directly
 *                              cmdNamePtr store found command name in original form from command line
 *      CHAT_COMMAND_UNKNOWN  - Command not found in table directly
 *                              text only skip possible whitespaces,
 *                              command is NULL
 *                              parentCommand is NULL
 *                              cmdNamePtr store command name that not found as it extracted from command line
 *      CHAT_COMMAND_UNKNOWN_SUBCOMMAND - Subcommand not found in some deed subcomand lists
 *                              text point to non parsed tail including not found command name in command line,
 *                              command store last found parent command if any
 *                              parentCommand have parent of command in command arg or NULL
 *                              cmdNamePtr store command name that not found as it extracted from command line
 */
ChatCommandSearchResult ChatHandler::FindCommand(ChatCommand* table, char const* &text, ChatCommand*& command, ChatCommand** parentCommand /*= NULL*/, std::string* cmdNamePtr /*= NULL*/, bool allAvailable /*= false*/, bool exactlyName /*= false*/)
{
    std::string cmd = "";

    // skip whitespaces
    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        ++text;
    }

    while (*text == ' ') ++text;

    // search first level command in table
    for(uint32 i = 0; table[i].Name != NULL; ++i)
    {
        if (exactlyName)
        {
            size_t len = strlen(table[i].Name);
            if (strncmp(table[i].Name, cmd.c_str(), len+1) != 0)
                continue;
        }
        else
        {
            if (!hasStringAbbr(table[i].Name, cmd.c_str()))
                continue;
        }
        // select subcommand from child commands list
        if (table[i].ChildCommands != NULL)
        {
            char const* oldchildtext = text;
            ChatCommand* parentSubcommand = NULL;
            ChatCommandSearchResult res = FindCommand(table[i].ChildCommands, text, command, &parentSubcommand, cmdNamePtr, allAvailable, exactlyName);

            switch(res)
            {
                case CHAT_COMMAND_OK:
                {
                    // if subcommand success search not return parent command, then this parent command is owner of child commands
                    if (parentCommand)
                        *parentCommand = parentSubcommand ? parentSubcommand : &table[i];

                    // Name == "" is special case: restore original command text for next level "" (where parentSubcommand==NULL)
                    if (strlen(command->Name)==0 && !parentSubcommand)
                        text = oldchildtext;

                    return CHAT_COMMAND_OK;
                }
                case CHAT_COMMAND_UNKNOWN:
                {
                    // command not found directly in child command list, return child command list owner
                    command = &table[i];
                    if (parentCommand)
                        *parentCommand = NULL;              // we don't known parent of table list at this point

                    text = oldchildtext;                    // restore text to stated just after parse found parent command
                    return CHAT_COMMAND_UNKNOWN_SUBCOMMAND; // we not found subcommand for table[i]
                }
                case CHAT_COMMAND_UNKNOWN_SUBCOMMAND:
                default:
                {
                    // some deep subcommand not found, if this second level subcommand then parentCommand can be NULL, use known value for it
                    if (parentCommand)
                        *parentCommand = parentSubcommand ? parentSubcommand : &table[i];
                    return res;
                }
            }
        }

        // must be available (not checked for subcommands case because parent command expected have most low access that all subcommands always
        if (!allAvailable && !isAvailable(table[i]))
            continue;

        // must be have handler is explicitly selected
        if (!table[i].Handler)
            continue;

        // command found directly in to table
        command = &table[i];

        // unknown table owner at this point
        if (parentCommand)
            *parentCommand = NULL;

        if (cmdNamePtr)
            *cmdNamePtr = cmd;

        return CHAT_COMMAND_OK;
    }

    // command not found in table directly
    command = NULL;

    // unknown table owner at this point
    if (parentCommand)
        *parentCommand = NULL;

    if (cmdNamePtr)
        *cmdNamePtr = cmd;

    return CHAT_COMMAND_UNKNOWN;
}

/**
 * Execute (sub)command available for chat handler access level with options in command line string
 *
 * @param text  Command line string that will parsed for (sub)command search and command specific data
 *
 * Command output and errors in command execution will send to chat handler.
 */
void ChatHandler::ExecuteCommand(const char* text)
{
    std::string fullcmd = text;                             // original `text` can't be used. It content destroyed in command code processing.

    ChatCommand* command = NULL;
    ChatCommand* parentCommand = NULL;

    ChatCommandSearchResult res = FindCommand(getCommandTable(), text, command, &parentCommand);

    switch(res)
    {
        case CHAT_COMMAND_OK:
        {
            SetSentErrorMessage(false);
            if ((this->*(command->Handler))((char*)text))   // text content destroyed at call
            {
                if (command->SecurityLevel > SECURITY_VIP)
                {
                    // chat case
                    if (m_session)
                    {
                        Player* p = m_session->GetPlayer();
                        ObjectGuid sel_guid = p->GetSelectionGuid();
                        sLog.outCommand(GetAccountId(),"Command: %s [Player: %s (Account: %u) X: %f Y: %f Z: %f Map: %u Selected: %s]",
                            fullcmd.c_str(),p->GetName(),GetAccountId(),p->GetPositionX(),p->GetPositionY(),p->GetPositionZ(),p->GetMapId(),
                            sel_guid.GetString().c_str());
                    }
                    else                                        // 0 account -> console
                    {
                        sLog.outCommand(GetAccountId(),"Command: %s [Account: %u from %s]",
                            fullcmd.c_str(),GetAccountId(),GetAccountId() ? "RA-connection" : "Console");
                    }
                }
            }
            // some commands have custom error messages. Don't send the default one in these cases.
            else if (!HasSentErrorMessage())
            {
                if (!command->Help.empty())
                    SendSysMessage(command->Help.c_str());
                else
                    SendSysMessage(LANG_CMD_SYNTAX);

                if (ChatCommand* showCommand = (strlen(command->Name)==0 && parentCommand ? parentCommand : command))
                    if (ChatCommand* childs = showCommand->ChildCommands)
                        ShowHelpForSubCommands(childs, showCommand->Name);

                SetSentErrorMessage(true);
            }
            break;
        }
        case CHAT_COMMAND_UNKNOWN_SUBCOMMAND:
        {
            SendSysMessage(LANG_NO_SUBCMD);
            ShowHelpForCommand(command->ChildCommands,text);
            SetSentErrorMessage(true);
            break;
        }
        case CHAT_COMMAND_UNKNOWN:
        {
            SendSysMessage(LANG_NO_CMD);
            SetSentErrorMessage(true);
            break;
        }
    }
}

/**
 * Function find appropriate command and update command security level and help text
 *
 * @param commandTable  Table for first level command search
 * @param text          Command line string that will parsed for (sub)command search
 * @param security      New security level for command
 * @param help          New help text  for command
 *
 * @return true if command has been found, and false in other case
 *
 * All problems found while command search and updated output as to DB errors log
 */
bool ChatHandler::SetDataForCommandInTable(ChatCommand *commandTable, const char* text, uint32 security, std::string const& help)
{
    std::string fullcommand = text;                         // original `text` can't be used. It content destroyed in command code processing.

    ChatCommand* command = NULL;
    std::string cmdName;

    ChatCommandSearchResult res = FindCommand(commandTable, text, command, NULL, &cmdName, true, true);

    switch(res)
    {
        case CHAT_COMMAND_OK:
        {
            if (command->SecurityLevel != security)
                DETAIL_LOG("Table `command` overwrite for command '%s' default security (%u) by %u",
                    fullcommand.c_str(),command->SecurityLevel,security);

            command->SecurityLevel = security;
            command->Help          = help;
            return true;
        }
        case CHAT_COMMAND_UNKNOWN_SUBCOMMAND:
        {
            // command have subcommands, but not '' subcommand and then any data in `command` useless for it.
            if (cmdName.empty())
                sLog.outErrorDb("Table `command` have command '%s' that only used with some subcommand selection, it can't have help or overwritten access level, skip.", fullcommand.c_str());
            else
                sLog.outErrorDb("Table `command` have unexpected subcommand '%s' in command '%s', skip.", cmdName.c_str(), fullcommand.c_str());
            return false;
        }
        case CHAT_COMMAND_UNKNOWN:
        {
            sLog.outErrorDb("Table `command` have nonexistent command '%s', skip.", cmdName.c_str());
            return false;
        }
    }

    return false;
}

bool ChatHandler::ParseCommands(const char* text)
{
    MANGOS_ASSERT(text);
    MANGOS_ASSERT(*text);

    //if(m_session->GetSecurity() == SEC_PLAYER)
    //    return false;

    /// chat case (.command or !command format)
    if (m_session)
    {
        if(text[0] != '!' && text[0] != '.')
            return false;

        /// ignore single . and ! in line
        if (strlen(text) < 2)
            return false;
    }

    /// ignore messages staring from many dots.
    if ((text[0] == '.' && text[1] == '.') || (text[0] == '!' && text[1] == '!'))
        return false;

    /// skip first . or ! (in console allowed use command with . and ! and without its)
    if (text[0] == '!' || text[0] == '.')
        ++text;

    ExecuteCommand(text);

    return true;
}

bool ChatHandler::ShowHelpForSubCommands(ChatCommand *table, char const* cmd)
{
    std::string list;
    for(uint32 i = 0; table[i].Name != NULL; ++i)
    {
        // must be available (ignore handler existence for show command with possible available subcommands
        if (!isAvailable(table[i]))
            continue;

        if (m_session)
            list += "\n    ";
        else
            list += "\n\r    ";

        list += table[i].Name;

        if (table[i].ChildCommands)
            list += " ...";
    }

    if (list.empty())
        return false;

    if (table==getCommandTable())
    {
        SendSysMessage(LANG_AVIABLE_CMD);
        PSendSysMessage("%s",list.c_str());
    }
    else
        PSendSysMessage(LANG_SUBCMDS_LIST,cmd,list.c_str());

    return true;
}

bool ChatHandler::ShowHelpForCommand(ChatCommand *table, const char* cmd)
{
    char const* oldCmd = cmd;
    ChatCommand* command = NULL;
    ChatCommand* parentCommand = NULL;

    ChatCommand* showCommand = NULL;
    ChatCommand* childCommands = NULL;

    ChatCommandSearchResult res = FindCommand(table, cmd, command, &parentCommand);

    switch(res)
    {
        case CHAT_COMMAND_OK:
        {
            // for "" subcommand use parent command if any for subcommands list output
            if (strlen(command->Name) == 0 && parentCommand)
            {
                showCommand = parentCommand;
                cmd = "";
            }
            else
                showCommand = command;

            childCommands = showCommand->ChildCommands;
            break;
        }
        case CHAT_COMMAND_UNKNOWN_SUBCOMMAND:
            showCommand = command;
            childCommands = showCommand->ChildCommands;
            break;
        case CHAT_COMMAND_UNKNOWN:
            // not show command list at error in first level command find fail
            childCommands = table != getCommandTable() || strlen(oldCmd) == 0 ? table : NULL;
            command = NULL;
            break;
    }

    if (command && !command->Help.empty())
        SendSysMessage(command->Help.c_str());

    if (childCommands)
        if (ShowHelpForSubCommands(childCommands, showCommand ? showCommand->Name : ""))
            return true;

    if (command && command->Help.empty())
        SendSysMessage(LANG_NO_HELP_CMD);

    return command || childCommands;
}

bool ChatHandler::isValidChatMessage(const char* message)
{
/*

valid examples:
|cffa335ee|Hitem:812:0:0:0:0:0:0:0:70|h[Glowing Brightwood Staff]|h|r
|cff808080|Hquest:2278:47|h[The Platinum Discs]|h|r
|cff4e96f7|Htalent:2232:-1|h[Taste for Blood]|h|r
|cff71d5ff|Hspell:21563|h[Command]|h|r
|cffffd000|Henchant:3919|h[Engineering: Rough Dynamite]|h|r

| will be escaped to ||
*/

    if(strlen(message) > 255)
        return false;

    const char validSequence[6] = "cHhhr";
    const char* validSequenceIterator = validSequence;

    // more simple checks
    if (sWorld.getConfig(CONFIG_UINT32_CHAT_STRICT_LINK_CHECKING_SEVERITY) < 3)
    {
        const std::string validCommands = "cHhr|";

        while(*message)
        {
            // find next pipe command
            message = strchr(message, '|');

            if(!message)
                return true;

            ++message;
            char commandChar = *message;
            if(validCommands.find(commandChar) == std::string::npos)
                return false;

            ++message;
            // validate sequence
            if(sWorld.getConfig(CONFIG_UINT32_CHAT_STRICT_LINK_CHECKING_SEVERITY) == 2)
            {
                if(commandChar == *validSequenceIterator)
                {
                    if (validSequenceIterator == validSequence+4)
                        validSequenceIterator = validSequence;
                    else
                        ++validSequenceIterator;
                }
                else if(commandChar != '|')
                    return false;
            }
        }
        return true;
    }

    std::istringstream reader(message);
    char buffer[256];

    uint32 color;

    ItemPrototype const* linkedItem;
    Quest const* linkedQuest;
    SpellEntry const *linkedSpell;
    ItemRandomPropertiesEntry const* itemProperty;
    ItemRandomSuffixEntry const* itemSuffix;

    while(!reader.eof())
    {
        if (validSequence == validSequenceIterator)
        {
            linkedItem = NULL;
            linkedQuest = NULL;
            linkedSpell = NULL;
            itemProperty = NULL;
            itemSuffix = NULL;

            reader.ignore(255, '|');
        }
        else if(reader.get() != '|')
        {
            DEBUG_LOG("ChatHandler::isValidChatMessage sequence aborted unexpectedly");
            return false;
        }

        // pipe has always to be followed by at least one char
        if ( reader.peek() == '\0')
        {
            DEBUG_LOG("ChatHandler::isValidChatMessage pipe followed by \\0");
            return false;
        }

        // no further pipe commands
        if (reader.eof())
            break;

        char commandChar;
        reader >> commandChar;

        // | in normal messages is escaped by ||
        if (commandChar != '|')
        {
            if(commandChar == *validSequenceIterator)
            {
                if (validSequenceIterator == validSequence+4)
                    validSequenceIterator = validSequence;
                else
                    ++validSequenceIterator;
            }
            else
            {
                DEBUG_LOG("ChatHandler::isValidChatMessage invalid sequence, expected %c but got %c", *validSequenceIterator, commandChar);
                return false;
            }
        }
        else if(validSequence != validSequenceIterator)
        {
            // no escaped pipes in sequences
            DEBUG_LOG("ChatHandler::isValidChatMessage got escaped pipe in sequence");
            return false;
        }

        switch (commandChar)
        {
            case 'c':
                color = 0;
                // validate color, expect 8 hex chars
                for(int i=0; i<8; i++)
                {
                    char c;
                    reader >> c;
                    if(!c)
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage got \\0 while reading color in |c command");
                        return false;
                    }

                    color <<= 4;
                    // check for hex char
                    if(c >= '0' && c <='9')
                    {
                        color |= c-'0';
                        continue;
                    }
                    if(c >= 'a' && c <='f')
                    {
                        color |= 10+c-'a';
                        continue;
                    }
                    DEBUG_LOG("ChatHandler::isValidChatMessage got non hex char '%c' while reading color", c);
                    return false;
                }
                break;
            case 'H':
                // read chars up to colon  = link type
                reader.getline(buffer, 256, ':');

                if (strcmp(buffer, "item") == 0)
                {
                    // read item entry
                    reader.getline(buffer, 256, ':');

                    linkedItem = ObjectMgr::GetItemPrototype(atoi(buffer));
                    if(!linkedItem)
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage got invalid itemID %u in |item command", atoi(buffer));
                        return false;
                    }

                    if (color != ItemQualityColors[linkedItem->Quality])
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage linked item has color %u, but user claims %u", ItemQualityColors[linkedItem->Quality],
                                color);
                        return false;
                    }

                    // the itementry is followed by several integers which describe an instance of this item

                    // position relative after itemEntry
                    const uint8 randomPropertyPosition = 6;

                    int32 propertyId = 0;
                    bool negativeNumber = false;
                    char c;
                    for(uint8 i=0; i<randomPropertyPosition; ++i)
                    {
                        propertyId = 0;
                        negativeNumber = false;
                        while((c = reader.get())!=':')
                        {
                            if(c >='0' && c<='9')
                            {
                                propertyId*=10;
                                propertyId += c-'0';
                            } else if(c == '-')
                                negativeNumber = true;
                            else
                                return false;
                        }
                    }
                    if (negativeNumber)
                        propertyId *= -1;

                    if (propertyId > 0)
                    {
                        itemProperty = sItemRandomPropertiesStore.LookupEntry(propertyId);
                        if (!itemProperty)
                            return false;
                    }
                    else if(propertyId < 0)
                    {
                        itemSuffix = sItemRandomSuffixStore.LookupEntry(-propertyId);
                        if (!itemSuffix)
                            return false;
                    }

                    // ignore other integers
                    while ((c >= '0' && c <= '9') || c== ':')
                    {
                        reader.ignore(1);
                        c = reader.peek();
                    }
                }
                else if(strcmp(buffer, "quest") == 0)
                {
                    // no color check for questlinks, each client will adapt it anyway
                    uint32 questid= 0;
                    // read questid
                    char c = reader.peek();
                    while(c >='0' && c<='9')
                    {
                        reader.ignore(1);
                        questid *= 10;
                        questid += c-'0';
                        c = reader.peek();
                    }

                    linkedQuest = sObjectMgr.GetQuestTemplate(questid);

                    if(!linkedQuest)
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage Questtemplate %u not found", questid);
                        return false;
                    }
                    c = reader.peek();
                    // level
                    while(c !='|' && c!='\0')
                    {
                        reader.ignore(1);
                        c = reader.peek();
                    }
                }
                else if(strcmp(buffer, "talent") == 0)
                {
                    // talent links are always supposed to be blue
                    if(color != CHAT_LINK_COLOR_TALENT)
                        return false;

                    // read talent entry
                    reader.getline(buffer, 256, ':');
                    TalentEntry const *talentInfo = sTalentStore.LookupEntry(atoi(buffer));
                    if(!talentInfo)
                        return false;

                    linkedSpell = sSpellStore.LookupEntry(talentInfo->RankID[0]);
                    if(!linkedSpell)
                        return false;

                    char c = reader.peek();
                    // skillpoints? whatever, drop it
                    while(c !='|' && c!='\0')
                    {
                        reader.ignore(1);
                        c = reader.peek();
                    }
                }
                else if(strcmp(buffer, "spell") == 0)
                {
                    if(color != CHAT_LINK_COLOR_SPELL)
                        return false;

                    uint32 spellid = 0;
                    // read spell entry
                    char c = reader.peek();
                    while(c >='0' && c<='9')
                    {
                        reader.ignore(1);
                        spellid *= 10;
                        spellid += c-'0';
                        c = reader.peek();
                    }
                    linkedSpell = sSpellStore.LookupEntry(spellid);
                    if(!linkedSpell)
                        return false;
                }
                else if(strcmp(buffer, "enchant") == 0)
                {
                    if(color != CHAT_LINK_COLOR_ENCHANT)
                        return false;

                    uint32 spellid = 0;
                    // read spell entry
                    char c = reader.peek();
                    while(c >='0' && c<='9')
                    {
                        reader.ignore(1);
                        spellid *= 10;
                        spellid += c-'0';
                        c = reader.peek();
                    }
                    linkedSpell = sSpellStore.LookupEntry(spellid);
                    if(!linkedSpell)
                        return false;
                }
                else
                {
                    DEBUG_LOG("ChatHandler::isValidChatMessage user sent unsupported link type '%s'", buffer);
                    return false;
                }
                break;
            case 'h':
                // if h is next element in sequence, this one must contain the linked text :)
                if (*validSequenceIterator == 'h')
                {
                    // links start with '['
                    if (reader.get() != '[')
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage link caption doesn't start with '['");
                        return false;
                    }
                    reader.getline(buffer, 256, ']');

                    // verify the link name
                    if (linkedSpell)
                    {
                        // spells with that flag have a prefix of "$PROFESSION: "
                        if (linkedSpell->Attributes & SPELL_ATTR_TRADESPELL)
                        {
                            // lookup skillid
                            SkillLineAbilityMapBounds bounds = sSpellMgr.GetSkillLineAbilityMapBounds(linkedSpell->Id);
                            if (bounds.first == bounds.second)
                            {
                                return false;
                            }

                            SkillLineAbilityEntry const *skillInfo = bounds.first->second;

                            if (!skillInfo)
                            {
                                return false;
                            }

                            SkillLineEntry const *skillLine = sSkillLineStore.LookupEntry(skillInfo->skillId);
                            if (!skillLine)
                            {
                                return false;
                            }

                            for(uint8 i=0; i<MAX_LOCALE; ++i)
                            {
                                uint32 skillLineNameLength = strlen(skillLine->name[i]);
                                if (skillLineNameLength > 0 && strncmp(skillLine->name[i], buffer, skillLineNameLength) == 0)
                                {
                                    // found the prefix, remove it to perform spellname validation below
                                    // -2 = strlen(": ")
                                    uint32 spellNameLength = strlen(buffer)-skillLineNameLength-2;
                                    memmove(buffer, buffer+skillLineNameLength+2, spellNameLength+1);
                                }
                            }
                        }
                        bool foundName = false;
                        for(uint8 i=0; i<MAX_LOCALE; ++i)
                        {
                            if (*linkedSpell->SpellName[i] && strcmp(linkedSpell->SpellName[i], buffer) == 0)
                            {
                                foundName = true;
                                break;
                            }
                        }
                        if (!foundName)
                            return false;
                    }
                    else if (linkedQuest)
                    {
                        if (linkedQuest->GetTitle() != buffer)
                        {
                            QuestLocale const *ql = sObjectMgr.GetQuestLocale(linkedQuest->GetQuestId());

                            if (!ql)
                            {
                                DEBUG_LOG("ChatHandler::isValidChatMessage default questname didn't match and there is no locale");
                                return false;
                            }

                            bool foundName = false;
                            for(uint8 i=0; i<ql->Title.size(); i++)
                            {
                                if (ql->Title[i] == buffer)
                                {
                                    foundName = true;
                                    break;
                                }
                            }
                            if (!foundName)
                            {
                                DEBUG_LOG("ChatHandler::isValidChatMessage no quest locale title matched");
                                return false;
                            }
                        }
                    }
                    else if(linkedItem)
                    {
                        char* const* suffix = itemSuffix?itemSuffix->nameSuffix:(itemProperty?itemProperty->nameSuffix:NULL);

                        std::string expectedName = std::string(linkedItem->Name1);
                        if (suffix)
                        {
                            expectedName += " ";
                            expectedName += suffix[LOCALE_enUS];
                        }

                        if (expectedName != buffer)
                        {
                            ItemLocale const *il = sObjectMgr.GetItemLocale(linkedItem->ItemId);

                            bool foundName = false;
                            for(uint8 i=LOCALE_koKR; i<MAX_LOCALE; ++i)
                            {
                                int8 dbIndex = sObjectMgr.GetIndexForLocale(LocaleConstant(i));
                                if (dbIndex == -1 || il == NULL || (size_t)dbIndex >= il->Name.size())
                                    // using strange database/client combinations can lead to this case
                                    expectedName = linkedItem->Name1;
                                else
                                    expectedName = il->Name[dbIndex];
                                if (suffix)
                                {
                                    expectedName += " ";
                                    expectedName += suffix[i];
                                }
                                if ( expectedName == buffer)
                                {
                                    foundName = true;
                                    break;
                                }
                            }
                            if (!foundName)
                            {
                                DEBUG_LOG("ChatHandler::isValidChatMessage linked item name wasn't found in any localization");
                                return false;
                            }
                        }
                    }
                    // that place should never be reached - if nothing linked has been set in |H
                    // it will return false before
                    else
                        return false;
                }
                break;
            case 'r':
            case '|':
                // no further payload
                break;
            default:
                DEBUG_LOG("ChatHandler::isValidChatMessage got invalid command |%c", commandChar);
                return false;
        }
    }

    // check if every opened sequence was also closed properly
    if(validSequence != validSequenceIterator)
        DEBUG_LOG("ChatHandler::isValidChatMessage EOF in active sequence");

    return validSequence == validSequenceIterator;
}

//Note: target_guid used only in CHAT_MSG_WHISPER_INFORM mode (in this case channelName ignored)
void ChatHandler::FillMessageData( WorldPacket *data, WorldSession* session, uint8 type, uint32 language, const char *channelName, uint64 target_guid, const char *message, Unit *speaker)
{
    uint32 messageLength = (message ? strlen(message) : 0) + 1;

    data->Initialize(SMSG_MESSAGECHAT, 100);                // guess size
    *data << uint8(type);
    if ((type != CHAT_MSG_CHANNEL && type != CHAT_MSG_WHISPER) || language == LANG_ADDON)
        *data << uint32(language);
    else
        *data << uint32(LANG_UNIVERSAL);

    switch(type)
    {
        case CHAT_MSG_SAY:
        case CHAT_MSG_PARTY:
        case CHAT_MSG_RAID:
        case CHAT_MSG_GUILD:
        case CHAT_MSG_OFFICER:
        case CHAT_MSG_YELL:
        case CHAT_MSG_WHISPER:
        case CHAT_MSG_CHANNEL:
        case CHAT_MSG_RAID_LEADER:
        case CHAT_MSG_RAID_WARNING:
        case CHAT_MSG_BG_SYSTEM_NEUTRAL:
        case CHAT_MSG_BG_SYSTEM_ALLIANCE:
        case CHAT_MSG_BG_SYSTEM_HORDE:
        case CHAT_MSG_BATTLEGROUND:
        case CHAT_MSG_BATTLEGROUND_LEADER:
            target_guid = session ? session->GetPlayer()->GetGUID() : 0;
            break;
        case CHAT_MSG_MONSTER_SAY:
        case CHAT_MSG_MONSTER_PARTY:
        case CHAT_MSG_MONSTER_YELL:
        case CHAT_MSG_MONSTER_WHISPER:
        case CHAT_MSG_MONSTER_EMOTE:
        case CHAT_MSG_RAID_BOSS_WHISPER:
        case CHAT_MSG_RAID_BOSS_EMOTE:
        {
            *data << speaker->GetObjectGuid();
            *data << uint32(0);                             // 2.1.0
            *data << uint32(strlen(speaker->GetName()) + 1);
            *data << speaker->GetName();
            ObjectGuid listener_guid;
            *data << listener_guid;
            if (!listener_guid.IsEmpty() && !listener_guid.IsPlayer())
            {
                *data << uint32(1);                         // string listener_name_length
                *data << uint8(0);                          // string listener_name
            }
            *data << uint32(messageLength);
            *data << message;
            *data << uint8(0);
            return;
        }
        default:
            if (type != CHAT_MSG_REPLY && type != CHAT_MSG_IGNORED && type != CHAT_MSG_DND && type != CHAT_MSG_AFK)
                target_guid = 0;                            // only for CHAT_MSG_WHISPER_INFORM used original value target_guid
            break;
    }

    *data << uint64(target_guid);                           // there 0 for BG messages
    *data << uint32(0);                                     // can be chat msg group or something

    if (type == CHAT_MSG_CHANNEL)
    {
        MANGOS_ASSERT(channelName);
        *data << channelName;
    }

    *data << uint64(target_guid);
    *data << uint32(messageLength);
    *data << message;
    if(session != 0 && type != CHAT_MSG_REPLY && type != CHAT_MSG_DND && type != CHAT_MSG_AFK)
        *data << uint8(session->GetPlayer()->chatTag());
    else
        *data << uint8(0);
}

Player * ChatHandler::getSelectedPlayer()
{
    if(!m_session)
        return NULL;

    ObjectGuid guid  = m_session->GetPlayer()->GetSelectionGuid();

    if (guid.IsEmpty())
        return m_session->GetPlayer();

    return sObjectMgr.GetPlayer(guid);
}

Unit* ChatHandler::getSelectedUnit()
{
    if(!m_session)
        return NULL;

    ObjectGuid guid = m_session->GetPlayer()->GetSelectionGuid();

    if (guid.IsEmpty())
        return m_session->GetPlayer();

    // can be selected player at another map
    return ObjectAccessor::GetUnit(*m_session->GetPlayer(), guid);
}

Creature* ChatHandler::getSelectedCreature()
{
    if(!m_session)
        return NULL;

    return m_session->GetPlayer()->GetMap()->GetAnyTypeCreature(m_session->GetPlayer()->GetSelectionGuid());
}

/**
 * Function skip all whitespaces in args string
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *             allowed NULL string pointer stored in *args
 */
void ChatHandler::SkipWhiteSpaces(char** args)
{
    if(!*args)
        return;

    while(isWhiteSpace(**args))
        ++(*args);
}

/**
 * Function extract to val arg signed integer value or fail
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val  return extracted value if function success, in fail case original value unmodified
 * @return     true if value extraction successful
 */
bool  ChatHandler::ExtractInt32(char** args, int32& val)
{
    if (!*args || !**args)
        return false;

    char* tail = *args;

    long valRaw = strtol(*args, &tail, 10);

    if (tail != *args && isWhiteSpace(*tail))
        *(tail++) = '\0';
    else if (tail && *tail)                                 // some not whitespace symbol
        return false;                                       // args not modified and can be re-parsed

    if (valRaw < std::numeric_limits<int32>::min() || valRaw > std::numeric_limits<int32>::max())
        return false;

    // value successfully extracted
    val = int32(valRaw);
    *args = tail;
    return true;
}

/**
 * Function extract to val arg optional signed integer value or use default value. Fail if extracted not signed integer.
 *
 * @param args    variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val     return extracted value if function success, in fail case original value unmodified
 * @param defVal  default value used if no data for extraction in args
 * @return        true if value extraction successful
 */
bool  ChatHandler::ExtractOptInt32(char** args, int32& val, int32 defVal)
{
    if (!*args || !**args)
    {
        val = defVal;
        return true;
    }

    return ExtractInt32(args, val);
}

/**
 * Function extract to val arg unsigned integer value or fail
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val  return extracted value if function success, in fail case original value unmodified
 * @param base set used base for extracted value format (10 for decimal, 16 for hex, etc), 0 let auto select by system internal function
 * @return     true if value extraction successful
 */
bool  ChatHandler::ExtractUInt32Base(char** args, uint32& val, uint32 base)
{
    if (!*args || !**args)
        return false;

    char* tail = *args;

    unsigned long valRaw = strtoul(*args, &tail, base);

    if (tail != *args && isWhiteSpace(*tail))
        *(tail++) = '\0';
    else if (tail && *tail)                                 // some not whitespace symbol
        return false;                                       // args not modified and can be re-parsed

    if (valRaw > std::numeric_limits<uint32>::max())
        return false;

    // value successfully extracted
    val = uint32(valRaw);
    *args = tail;

    SkipWhiteSpaces(args);
    return true;
}

/**
 * Function extract to val arg optional unsigned integer value or use default value. Fail if extracted not unsigned integer.
 *
 * @param args    variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val     return extracted value if function success, in fail case original value unmodified
 * @param defVal  default value used if no data for extraction in args
 * @return        true if value extraction successful
 */
bool  ChatHandler::ExtractOptUInt32(char** args, uint32& val, uint32 defVal)
{
    if (!*args || !**args)
    {
        val = defVal;
        return true;
    }

    return ExtractUInt32(args, val);
}

/**
 * Function extract to val arg float value or fail
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val  return extracted value if function success, in fail case original value unmodified
 * @return     true if value extraction successful
 */
bool  ChatHandler::ExtractFloat(char** args, float& val)
{
    if (!*args || !**args)
        return false;

    char* tail = *args;

    double valRaw = strtod(*args, &tail);

    if (tail != *args && isWhiteSpace(*tail))
        *(tail++) = '\0';
    else if (tail && *tail)                                 // some not whitespace symbol
        return false;                                       // args not modified and can be re-parsed

    // value successfully extracted
    val = float(valRaw);
    *args = tail;

    SkipWhiteSpaces(args);
    return true;
}

/**
 * Function extract to val arg optional float value or use default value. Fail if extracted not float.
 *
 * @param args    variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val     return extracted value if function success, in fail case original value unmodified
 * @param defVal  default value used if no data for extraction in args
 * @return        true if value extraction successful
 */
bool  ChatHandler::ExtractOptFloat(char** args, float& val, float defVal)
{
    if (!*args || !**args)
    {
        val = defVal;
        return true;
    }

    return ExtractFloat(args, val);
}

/**
 * Function extract name-like string (from non-numeric or special symbol until whitespace)
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param lit  optional explicit literal requirement. function fail if literal is not starting substring of lit.
 *             Note: function in same way fail if no any literal or literal not fit in this case. Need additional check for select specific fail case
 * @return     name/number-like string without whitespaces, or NULL if args empty or not appropriate content.
 */
char* ChatHandler::ExtractLiteralArg(char** args, char const* lit /*= NULL*/)
{
    if (!*args || !**args)
        return NULL;

    char* head = *args;

    // reject quoted string or link (|-started text)
    switch (head[0])
    {
        // reject quoted string
        case '[': case '\'': case '"':
            return NULL;
        // reject link (|-started text)
        case '|':
            // client replace all | by || in raw text
            if (head[1] != '|')
                return NULL;
            ++head;                                         // skip one |
            break;
        default: break;
    }

    if (lit)
    {
        int l = strlen(lit);
        int diff = strncmp(head, lit, l);

        if (diff != 0)
            return NULL;

        if (head[l] && !isWhiteSpace(head[l]))
            return NULL;

        char* arg = head;

        if (head[l])
        {
            head[l] = '\0';

            head += l + 1;

            *args = head;
        }
        else
            *args = head + l;

        SkipWhiteSpaces(args);
        return arg;
    }

    char* name = strtok(head, " ");

    char* tail = strtok(NULL, "");

    *args = tail ? tail : (char*)"";                        // *args don't must be NULL

    SkipWhiteSpaces(args);

    return name;
}

/**
 * Function extract quote-like string (any characters guarded by some special character, in our cases ['")
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param asis control save quote string wrappers
 * @return     quote-like string, or NULL if args empty or not appropriate content.
 */
char* ChatHandler::ExtractQuotedArg( char** args, bool asis /*= false*/ )
{
    if (!*args || !**args)
        return NULL;

    if (**args != '\'' && **args != '"' && **args != '[')
        return NULL;

    char guard = (*args)[0];

    if (guard == '[')
        guard = ']';

    char* tail = (*args)+1;                                 // start scan after first quote symbol
    char* head = asis ? *args : tail;                       // start arg

    while (*tail && *tail != guard)
        ++tail;

    if (!*tail || (tail[1] && !isWhiteSpace(tail[1])))      // fail
        return NULL;

    if (!tail[1])                                           // quote is last char in string
    {
        if (!asis)
            *tail = '\0';
    }
    else                                                    // quote isn't last char
    {
        if (asis)
            ++tail;

        *tail = '\0';
    }

    *args = tail+1;

    SkipWhiteSpaces(args);

    return head;
}

/**
 * Function extract quote-like string or literal if quote not detected
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param asis control save quote string wrappers
 * @return     quote/literal string, or NULL if args empty or not appropriate content.
 */
char* ChatHandler::ExtractQuotedOrLiteralArg(char** args, bool asis /*= false*/)
{
    char *arg = ExtractQuotedArg(args, asis);
    if (!arg)
        arg = ExtractLiteralArg(args);
    return arg;
}

/**
 * Function extract on/off literals as boolean values
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val  return extracted value if function success, in fail case original value unmodified
 * @return     true at success
 */
bool  ChatHandler::ExtractOnOff(char** args, bool& value)
{
    char* arg = ExtractLiteralArg(args);
    if (!arg)
        return false;

    if (strncmp(arg, "on", 3) == 0)
        value = true;
    else if (strncmp(arg, "off", 4) == 0)
        value = false;
    else
        return false;

    return true;
}

/**
 * Function extract shift-link-like string (any characters guarded by | and |h|r with some additional internal structure check)
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *
 * @param linkTypes  optional NULL-terminated array of link types, shift-link must fit one from link type from array if provided or extraction fail
 *
 * @param found_idx  if not NULL then at return index in linkTypes that fit shift-link type, if extraction fail then non modified
 *
 * @param keyPair    if not NULL then pointer to 2-elements array for return start and end pointer for found key
 *                   if extraction fail then non modified
 *
 * @param somethingPair then pointer to 2-elements array for return start and end pointer if found.
 *                   if not NULL then shift-link must have data field, if extraction fail then non modified
 *
 * @return     shift-link-like string, or NULL if args empty or not appropriate content.
 */
char* ChatHandler::ExtractLinkArg(char** args, char const* const* linkTypes /*= NULL*/, int* foundIdx /*= NULL*/, char** keyPair /*= NULL*/, char** somethingPair /*= NULL*/)
{
    if (!*args || !**args)
        return NULL;

    // skip if not linked started or encoded single | (doubled by client)
    if ((*args)[0] != '|' || (*args)[1] == '|')
        return NULL;

    // |color|Hlinktype:key:data...|h[name]|h|r

    char* head = *args;

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r
    // or
    // [name] Shift-click form |linkType:key|h[name]|h|r

    // |color|Hlinktype:key:data...|h[name]|h|r

    char* tail = (*args)+1;                                 // skip |

    if (*tail != 'H')                                       // skip color part, some links can not have color part
    {
        while (*tail && *tail != '|')
            ++tail;

        if (!*tail)
            return NULL;

        // |Hlinktype:key:data...|h[name]|h|r

        ++tail;                                             // skip |
    }

    // Hlinktype:key:data...|h[name]|h|r

    if (*tail != 'H')
        return NULL;

    int linktype_idx = 0;

    if (linkTypes)                                          // check link type if provided
    {
        // check linktypes (its include H in name)
        for (; linkTypes[linktype_idx]; ++linktype_idx)
        {
            // exactly string with follow : or |
            int l = strlen(linkTypes[linktype_idx]);
            if (strncmp(tail, linkTypes[linktype_idx], l) == 0 &&
                (tail[l] == ':' || tail[l] == '|'))
                break;
        }

        // is search fail?
        if (!linkTypes[linktype_idx])                       // NULL terminator in last element
            return NULL;

        tail += strlen(linkTypes[linktype_idx]);            // skip linktype string

        // :key:data...|h[name]|h|r

        if (*tail != ':')
            return NULL;
    }
    else
    {
        while (*tail && *tail != ':')                       // skip linktype string
            ++tail;

        if (!*tail)
            return NULL;
    }

    ++tail;

    // key:data...|h[name]|h|r
    char* keyStart = tail;                                  // remember key start for return
    char* keyEnd   = tail;                                  // key end for truncate, will updated

    while (*tail && *tail != '|' && *tail != ':')
        ++tail;

    if (!*tail)
        return NULL;

    keyEnd = tail;                                          // remember key end for truncate

    // |h[name]|h|r or :something...|h[name]|h|r

    char* somethingStart = tail+1;
    char* somethingEnd   = tail+1;                          // will updated later if need

    if (*tail == ':' && somethingPair)                      // optional data extraction
    {
        // :something...|h[name]|h|r

        if (*tail == ':')
            ++tail;

        // something|h[name]|h|r or something:something2...|h[name]|h|r

        while (*tail && *tail != '|' && *tail != ':')
            ++tail;

        if (!*tail)
            return NULL;

        somethingEnd = tail;                                // remember data end for truncate
    }

    // |h[name]|h|r or :something2...|h[name]|h|r

    while (*tail && (*tail != '|' || *(tail+1) != 'h'))     // skip ... part if exist
        ++tail;

    if (!*tail)
        return NULL;

    // |h[name]|h|r

    tail += 2;                                              // skip |h

    // [name]|h|r
    if (!*tail || *tail != '[')
        return NULL;

    while (*tail && (*tail != ']' || *(tail+1) != '|'))     // skip name part
        ++tail;

    tail += 2;                                              // skip ]|

    // h|r
    if (!*tail || *tail != 'h'  || *(tail+1) != '|')
        return NULL;

    tail += 2;                                              // skip h|

    // r
    if (!*tail || *tail != 'r' || (*(tail+1) && !isWhiteSpace(*(tail+1))))
        return NULL;

    ++tail;                                                 // skip r

    // success

    if (*tail)                                              // truncate all link string
        *(tail++) = '\0';

    if (foundIdx)
        *foundIdx = linktype_idx;

    if (keyPair)
    {
        keyPair[0] = keyStart;
        keyPair[1] = keyEnd;
    }

    if (somethingPair)
    {
        somethingPair[0] = somethingStart;
        somethingPair[1] = somethingEnd;
    }

    *args = tail;

    SkipWhiteSpaces(args);

    return head;
}

/**
 * Function extract name/number/quote/shift-link-like string
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param asis control save quote string wrappers
 * @return     extracted arg string, or NULL if args empty or not appropriate content.
 */
char* ChatHandler::ExtractArg( char** args, bool asis /*= false*/ )
{
    if (!*args || !**args)
        return NULL;

    char* arg = ExtractQuotedOrLiteralArg(args, asis);
    if (!arg)
        arg = ExtractLinkArg(args);

    return arg;
}

/**
 * Function extract name/quote/number/shift-link-like string, and return it if args have more non-whitespace data
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *             if args have only single arg then args still pointing to this arg (unmodified pointer)
 * @return     extracted string, or NULL if args empty or not appropriate content or have single arg totally.
 */
char* ChatHandler::ExtractOptNotLastArg(char** args)
{
    char* arg = ExtractArg(args, true);

    // have more data
    if (*args && **args)
        return arg;

    // optional name not found
    *args = arg ? arg : (char*)"";                          // *args don't must be NULL

    return NULL;
}

/**
 * Function extract data from shift-link "|color|LINKTYPE:RETURN:SOMETHING1|h[name]|h|r if linkType == LINKTYPE
 * It also extract literal/quote if not shift-link in args
 *
 * @param args       variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *                   if args have sift link with linkType != LINKTYPE then args still pointing to this arg (unmodified pointer)
 *
 * @param linkType   shift-link must fit by link type to this arg value or extraction fail
 *
 * @param something1 if not NULL then shift-link must have data field and it returned into this arg
 *                   if extraction fail then non modified
 *
 * @return           extracted key, or NULL if args empty or not appropriate content or not fit to linkType.
 */
char* ChatHandler::ExtractKeyFromLink(char** text, char const* linkType, char** something1 /*= NULL*/)
{
    char const* linkTypes[2];
    linkTypes[0] = linkType;
    linkTypes[1] = NULL;

    int foundIdx;

    return ExtractKeyFromLink(text, linkTypes, &foundIdx, something1);
}

/**
 * Function extract data from shift-link "|color|LINKTYPE:RETURN:SOMETHING1|h[name]|h|r if LINKTYPE in linkTypes array
 * It also extract literal/quote if not shift-link in args
 *
 * @param args       variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *                   if args have sift link with linkType != LINKTYPE then args still pointing to this arg (unmodified pointer)
 *
 * @param linkTypes  NULL-terminated array of link types, shift-link must fit one from link type from array or extraction fail
 *
 * @param found_idx  if not NULL then at return index in linkTypes that fit shift-link type, for non-link case return -1
 *                   if extraction fail then non modified
 *
 * @param something1 if not NULL then shift-link must have data field and it returned into this arg
 *                   if extraction fail then non modified
 *
 * @return           extracted key, or NULL if args empty or not appropriate content or not fit to linkType.
 */
char* ChatHandler::ExtractKeyFromLink(char** text, char const* const* linkTypes, int* found_idx, char** something1 /*= NULL*/)
{
    // skip empty
    if (!*text || !**text)
        return NULL;

    // return non link case
    char* arg = ExtractQuotedOrLiteralArg(text);
    if (arg)
    {
        if (found_idx)
            *found_idx = -1;                                // special index case

        return arg;
    }

    char* keyPair[2];
    char* somethingPair[2];

    arg = ExtractLinkArg(text, linkTypes, found_idx, keyPair, something1 ? somethingPair : NULL);
    if (!arg)
        return NULL;

    *keyPair[1] = '\0';                                     // truncate key string

    if (something1)
    {
        *somethingPair[1] = '\0';                           // truncate data string
        *something1 = somethingPair[0];
    }

    return keyPair[0];
}

/**
 * Function extract uint32 key from shift-link "|color|LINKTYPE:RETURN|h[name]|h|r if linkType == LINKTYPE
 * It also extract direct number if not shift-link in args
 *
 * @param args       variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *                   if args have sift link with linkType != LINKTYPE then args still pointing to this arg (unmodified pointer)
 *
 * @param linkType   shift-link must fit by link type to this arg value or extraction fail
 *
 * @param value      store result value at success return, not modified at fail
 *
 * @return           true if extraction succesful
 */
bool ChatHandler::ExtractUint32KeyFromLink(char** text, char const* linkType, uint32& value)
{
    char* arg = ExtractKeyFromLink(text, linkType);
    if (!arg)
        return false;

    return ExtractUInt32(&arg, value);
}

GameObject* ChatHandler::GetGameObjectWithGuid(uint32 lowguid,uint32 entry)
{
    if (!m_session)
        return NULL;

    Player* pl = m_session->GetPlayer();

    return pl->GetMap()->GetGameObject(ObjectGuid(HIGHGUID_GAMEOBJECT, entry, lowguid));
}

enum SpellLinkType
{
    SPELL_LINK_RAW     =-1,                                 // non-link case
    SPELL_LINK_SPELL   = 0,
    SPELL_LINK_TALENT  = 1,
    SPELL_LINK_ENCHANT = 2,
};

static char const* const spellKeys[] =
{
    "Hspell",                                               // normal spell
    "Htalent",                                              // talent spell
    "Henchant",                                             // enchanting recipe spell
    NULL
};

uint32 ChatHandler::ExtractSpellIdFromLink(char** text)
{
    // number or [name] Shift-click form |color|Henchant:recipe_spell_id|h[prof_name: recipe_name]|h|r
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Htalent:talent_id,rank|h[name]|h|r
    int type;
    char* param1_str = NULL;
    char* idS = ExtractKeyFromLink(text, spellKeys, &type, &param1_str);
    if (!idS)
        return 0;

    uint32 id;
    if (!ExtractUInt32(&idS, id))
        return 0;

    switch(type)
    {
        case SPELL_LINK_RAW:
        case SPELL_LINK_SPELL:
        case SPELL_LINK_ENCHANT:
            return id;
        case SPELL_LINK_TALENT:
        {
            // talent
            TalentEntry const* talentEntry = sTalentStore.LookupEntry(id);
            if(!talentEntry)
                return 0;

            int32 rank;
            if (!ExtractInt32(&param1_str, rank))
                return 0;

            if (rank < 0)                                   // unlearned talent have in shift-link field -1 as rank
                rank = 0;

            return rank < MAX_TALENT_RANK ? talentEntry->RankID[rank] : 0;
        }
    }

    // unknown type?
    return 0;
}

GameTele const* ChatHandler::ExtractGameTeleFromLink(char** text)
{
    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    char* cId = ExtractKeyFromLink(text,"Htele");
    if (!cId)
        return NULL;

    // id case (explicit or from shift link)
    uint32 id;
    if (ExtractUInt32(&cId, id))
        return sObjectMgr.GetGameTele(id);
    else
        return sObjectMgr.GetGameTele(cId);
}

enum GuidLinkType
{
    GUID_LINK_RAW        =-1,                               // non-link case
    GUID_LINK_PLAYER     = 0,
    GUID_LINK_CREATURE   = 1,
    GUID_LINK_GAMEOBJECT = 2,
};

static char const* const guidKeys[] =
{
    "Hplayer",
    "Hcreature",
    "Hgameobject",
    NULL
};

ObjectGuid ChatHandler::ExtractGuidFromLink(char** text)
{
    int type = 0;

    // |color|Hcreature:creature_guid|h[name]|h|r
    // |color|Hgameobject:go_guid|h[name]|h|r
    // |color|Hplayer:name|h[name]|h|r
    char* idS = ExtractKeyFromLink(text, guidKeys, &type);
    if (!idS)
        return ObjectGuid();

    switch(type)
    {
        case GUID_LINK_RAW:
        case GUID_LINK_PLAYER:
        {
            std::string name = idS;
            if (!normalizePlayerName(name))
                return ObjectGuid();

            if (Player* player = sObjectMgr.GetPlayer(name.c_str()))
                return player->GetObjectGuid();

            if (uint64 guid = sObjectMgr.GetPlayerGUIDByName(name))
                return ObjectGuid(guid);

            return ObjectGuid();
        }
        case GUID_LINK_CREATURE:
        {
            uint32 lowguid;
            if (!ExtractUInt32(&idS, lowguid))
                return ObjectGuid();

            if (CreatureData const* data = sObjectMgr.GetCreatureData(lowguid))
                return ObjectGuid(HIGHGUID_UNIT, data->id, lowguid);
            else
                return ObjectGuid();
        }
        case GUID_LINK_GAMEOBJECT:
        {
            uint32 lowguid;
            if (!ExtractUInt32(&idS, lowguid))
                return ObjectGuid();

            if(GameObjectData const* data = sObjectMgr.GetGOData(lowguid) )
                return ObjectGuid(HIGHGUID_GAMEOBJECT, data->id, lowguid);
            else
                return ObjectGuid();
        }
    }

    // unknown type?
    return ObjectGuid();
}

enum LocationLinkType
{
    LOCATION_LINK_RAW               =-1,                    // non-link case
    LOCATION_LINK_PLAYER            = 0,
    LOCATION_LINK_TELE              = 1,
    LOCATION_LINK_TAXINODE          = 2,
    LOCATION_LINK_CREATURE          = 3,
    LOCATION_LINK_GAMEOBJECT        = 4,
    LOCATION_LINK_CREATURE_ENTRY    = 5,
    LOCATION_LINK_GAMEOBJECT_ENTRY  = 6,
    LOCATION_LINK_AREATRIGGER       = 7,
    LOCATION_LINK_AREATRIGGER_TARGET= 8,
};

static char const* const locationKeys[] =
{
    "Htele",
    "Htaxinode",
    "Hplayer",
    "Hcreature",
    "Hgameobject",
    "Hcreature_entry",
    "Hgameobject_entry",
    "Hareatrigger",
    "Hareatrigger_target",
    NULL
};

bool ChatHandler::ExtractLocationFromLink(char** text, uint32& mapid, float& x, float& y, float& z)
{
    int type = 0;

    // |color|Hplayer:name|h[name]|h|r
    // |color|Htele:id|h[name]|h|r
    // |color|Htaxinode:id|h[name]|h|r
    // |color|Hcreature:creature_guid|h[name]|h|r
    // |color|Hgameobject:go_guid|h[name]|h|r
    // |color|Hcreature_entry:creature_id|h[name]|h|r
    // |color|Hgameobject_entry:go_id|h[name]|h|r
    // |color|Hareatrigger:id|h[name]|h|r
    // |color|Hareatrigger_target:id|h[name]|h|r
    char* idS = ExtractKeyFromLink(text, locationKeys, &type);
    if (!idS)
        return false;

    switch(type)
    {
        case LOCATION_LINK_RAW:
        case LOCATION_LINK_PLAYER:
        {
            std::string name = idS;
            if (!normalizePlayerName(name))
                return false;

            if (Player* player = sObjectMgr.GetPlayer(name.c_str()))
            {
                mapid = player->GetMapId();
                x = player->GetPositionX();
                y = player->GetPositionY();
                z = player->GetPositionZ();
                return true;
            }

            ObjectGuid guid = sObjectMgr.GetPlayerGUIDByName(name);
            if (!guid.IsEmpty())
            {
                // to point where player stay (if loaded)
                float o;
                bool in_flight;
                return Player::LoadPositionFromDB(guid, mapid, x, y, z, o, in_flight);
            }

            return false;
        }
        case LOCATION_LINK_TELE:
        {
            uint32 id;
            if (!ExtractUInt32(&idS, id))
                return false;

            GameTele const* tele = sObjectMgr.GetGameTele(id);
            if (!tele)
                return false;
            mapid = tele->mapId;
            x = tele->position_x;
            y = tele->position_y;
            z = tele->position_z;
            return true;
        }
        case LOCATION_LINK_TAXINODE:
        {
            uint32 id;
            if (!ExtractUInt32(&idS, id))
                return false;

            TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(id);
            if (!node)
                return false;
            mapid = node->map_id;
            x = node->x;
            y = node->y;
            z = node->z;
            return true;
        }
        case LOCATION_LINK_CREATURE:
        {
            uint32 lowguid;
            if (!ExtractUInt32(&idS, lowguid))
                return false;

            if(CreatureData const* data = sObjectMgr.GetCreatureData(lowguid) )
            {
                mapid = data->mapid;
                x = data->posX;
                y = data->posY;
                z = data->posZ;
                return true;
            }
            else
                return false;
        }
        case LOCATION_LINK_GAMEOBJECT:
        {
            uint32 lowguid;
            if (!ExtractUInt32(&idS, lowguid))
                return false;

            if(GameObjectData const* data = sObjectMgr.GetGOData(lowguid) )
            {
                mapid = data->mapid;
                x = data->posX;
                y = data->posY;
                z = data->posZ;
                return true;
            }
            else
                return false;
        }
        case LOCATION_LINK_CREATURE_ENTRY:
        {
            uint32 id;
            if (!ExtractUInt32(&idS, id))
                return false;

            if (ObjectMgr::GetCreatureTemplate(id))
            {
                FindCreatureData worker(id, m_session ? m_session->GetPlayer() : NULL);

                sObjectMgr.DoCreatureData(worker);

                if (CreatureDataPair const* dataPair = worker.GetResult())
                {
                    mapid = dataPair->second.mapid;
                    x = dataPair->second.posX;
                    y = dataPair->second.posY;
                    z = dataPair->second.posZ;
                    return true;
                }
                else
                    return false;
            }
            else
                return false;
        }
        case LOCATION_LINK_GAMEOBJECT_ENTRY:
        {
            uint32 id;
            if (!ExtractUInt32(&idS, id))
                return false;

            if (ObjectMgr::GetGameObjectInfo(id))
            {
                FindGOData worker(id, m_session ? m_session->GetPlayer() : NULL);

                sObjectMgr.DoGOData(worker);

                if (GameObjectDataPair const* dataPair = worker.GetResult())
                {
                    mapid = dataPair->second.mapid;
                    x = dataPair->second.posX;
                    y = dataPair->second.posY;
                    z = dataPair->second.posZ;
                    return true;
                }
                else
                    return false;
            }
            else
                return false;
        }
        case LOCATION_LINK_AREATRIGGER:
        {
            uint32 id;
            if (!ExtractUInt32(&idS, id))
                return false;

            AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(id);
            if (!atEntry)
            {
                PSendSysMessage(LANG_COMMAND_GOAREATRNOTFOUND, id);
                SetSentErrorMessage(true);
                return false;
            }

            mapid = atEntry->mapid;
            x = atEntry->x;
            y = atEntry->y;
            z = atEntry->z;
            return true;
        }
        case LOCATION_LINK_AREATRIGGER_TARGET:
        {
            uint32 id;
            if (!ExtractUInt32(&idS, id))
                return false;

            if (!sAreaTriggerStore.LookupEntry(id))
            {
                PSendSysMessage(LANG_COMMAND_GOAREATRNOTFOUND, id);
                SetSentErrorMessage(true);
                return false;
            }

            AreaTrigger const* at = sObjectMgr.GetAreaTrigger(id);
            if(!at)
            {
                PSendSysMessage(LANG_AREATRIGER_NOT_HAS_TARGET, id);
                SetSentErrorMessage(true);
                return false;
            }

            mapid = at->target_mapId;
            x = at->target_X;
            y = at->target_Y;
            z = at->target_Z;
            return true;
        }
    }

    // unknown type?
    return false;
}

std::string ChatHandler::ExtractPlayerNameFromLink(char** text)
{
    // |color|Hplayer:name|h[name]|h|r
    char* name_str = ExtractKeyFromLink(text, "Hplayer");
    if(!name_str)
        return "";

    std::string name = name_str;
    if(!normalizePlayerName(name))
        return "";

    return name;
}

/**
 * Function extract at least one from request player data (pointer/guid/name) from args name/shift-link or selected player if no args
 *
 * @param args        variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *
 * @param player      optional arg   One from 3 optional args must be provided at least (or more).
 * @param player_guid optional arg   For function success only one from provided args need get result
 * @param player_name optional arg   But if early arg get value then all later args will have its (if requested)
 *                                   if player_guid requested and not found then name also will not found
 *                                   So at success can be returned 2 cases: (player/guid/name) or (guid/name)
 *
 * @return           true if extraction successful
 */
bool ChatHandler::ExtractPlayerTarget(char** args, Player** player /*= NULL*/, ObjectGuid* player_guid /*= NULL*/,std::string* player_name /*= NULL*/)
{
    if (*args && **args)
    {
        std::string name = ExtractPlayerNameFromLink(args);
        if (name.empty())
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        Player* pl = sObjectMgr.GetPlayer(name.c_str());

        // if allowed player pointer
        if(player)
            *player = pl;

        // if need guid value from DB (in name case for check player existence)
        ObjectGuid guid = !pl && (player_guid || player_name) ? sObjectMgr.GetPlayerGUIDByName(name) : uint64(0);

        // if allowed player guid (if no then only online players allowed)
        if(player_guid)
            *player_guid = pl ? pl->GetObjectGuid() : guid;

        if(player_name)
            *player_name = pl || !guid.IsEmpty() ? name : "";
    }
    else
    {
        Player* pl = getSelectedPlayer();
        // if allowed player pointer
        if(player)
            *player = pl;
        // if allowed player guid (if no then only online players allowed)
        if(player_guid)
            *player_guid = pl ? pl->GetObjectGuid() : ObjectGuid();

        if(player_name)
            *player_name = pl ? pl->GetName() : "";
    }

    // some from req. data must be provided (note: name is empty if player not exist)
    if((!player || !*player) && (!player_guid || player_guid->IsEmpty()) && (!player_name || player_name->empty()))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

uint32 ChatHandler::ExtractAccountId(char** args, std::string* accountName /*= NULL*/, Player** targetIfNullArg /*= NULL*/)
{
    uint32 account_id = 0;

    ///- Get the account name from the command line
    char* account_str = ExtractLiteralArg(args);

    if (!account_str)
    {
        if (!targetIfNullArg)
            return 0;

        /// only target player different from self allowed (if targetPlayer!=NULL then not console)
        Player* targetPlayer = getSelectedPlayer();
        if (!targetPlayer)
            return 0;

        account_id = targetPlayer->GetSession()->GetAccountId();

        if (accountName)
            sAccountMgr.GetName(account_id, *accountName);

        if (targetIfNullArg)
            *targetIfNullArg = targetPlayer;

        return account_id;
    }

    std::string account_name;

    if (ExtractUInt32(&account_str, account_id))
    {
        if (!sAccountMgr.GetName(account_id, account_name))
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_str);
            SetSentErrorMessage(true);
            return 0;
        }
    }
    else
    {
        account_name = account_str;
        if (!AccountMgr::normalizeString(account_name))
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return 0;
        }

        account_id = sAccountMgr.GetId(account_name);
        if (!account_id)
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST,account_name.c_str());
            SetSentErrorMessage(true);
            return 0;
        }
    }

    if (accountName)
        *accountName = account_name;

    if (targetIfNullArg)
        *targetIfNullArg = NULL;

    return account_id;
}

struct RaceMaskName
{
    char const* literal;
    uint32 raceMask;
};

static RaceMaskName const raceMaskNames[] =
{
    // races
    { "human",    (1<<(RACE_HUMAN-1))   },
    { "orc",      (1<<(RACE_ORC-1))     },
    { "dwarf",    (1<<(RACE_DWARF-1))   },
    { "nightelf", (1<<(RACE_NIGHTELF-1))},
    { "undead",   (1<<(RACE_UNDEAD-1))  },
    { "tauren",   (1<<(RACE_TAUREN-1))  },
    { "gnome",    (1<<(RACE_GNOME-1))   },
    { "troll",    (1<<(RACE_TROLL-1))   },
    { "bloodelf", (1<<(RACE_BLOODELF-1))},
    { "draenei",  (1<<(RACE_DRAENEI-1)) },

    // masks
    { "alliance", RACEMASK_ALLIANCE },
    { "horde",    RACEMASK_HORDE },
    { "all", RACEMASK_ALL_PLAYABLE },

    // terminator
    { NULL, 0 }
};

bool ChatHandler::ExtractRaceMask(char** text, uint32& raceMask, char const** maskName /*=NULL*/)
{
    if (ExtractUInt32(text, raceMask))
    {
        if (maskName)
            *maskName = "custom mask";
    }
    else
    {
        for (RaceMaskName const* itr = raceMaskNames; itr->literal; ++itr)
        {
            if (ExtractLiteralArg(text, itr->literal))
            {
                raceMask = itr->raceMask;

                if (maskName)
                    *maskName = itr->literal;
                break;
            }
        }

        if (!raceMask)
            return false;
    }

    return true;
}

std::string ChatHandler::GetNameLink(Player* chr) const
{
    return playerLink(chr->GetName());
}

bool ChatHandler::needReportToTarget(Player* chr) const
{
    Player* pl = m_session->GetPlayer();
    return pl != chr && pl->IsVisibleGloballyFor(chr);
}

LocaleConstant ChatHandler::GetSessionDbcLocale() const
{
    return m_session->GetSessionDbcLocale();
}

int ChatHandler::GetSessionDbLocaleIndex() const
{
    return m_session->GetSessionDbLocaleIndex();
}

const char *CliHandler::GetMangosString(int32 entry) const
{
    return sObjectMgr.GetMangosStringForDBCLocale(entry);
}

uint32 CliHandler::GetAccountId() const
{
    return m_accountId;
}

AccountTypes CliHandler::GetAccessLevel() const
{
    return m_loginAccessLevel;
}

bool CliHandler::isAvailable(ChatCommand const& cmd) const
{
    // skip non-console commands in console case
    if (!cmd.AllowConsole)
        return false;

    // normal case
    return GetAccessLevel() >= (AccountTypes)cmd.SecurityLevel;
}

void CliHandler::SendSysMessage(const char *str)
{
    m_print(m_callbackArg, str);
    m_print(m_callbackArg, "\r\n");
}

std::string CliHandler::GetNameLink() const
{
    return GetMangosString(LANG_CONSOLE_COMMAND);
}

bool CliHandler::needReportToTarget(Player* /*chr*/) const
{
    return true;
}

LocaleConstant CliHandler::GetSessionDbcLocale() const
{
    return sWorld.GetDefaultDbcLocale();
}

int CliHandler::GetSessionDbLocaleIndex() const
{
    return sObjectMgr.GetDBCLocaleIndex();
}

// Check/ Output if a NPC or GO (by guid) is part of a pool or game event
template <typename T>
void ChatHandler::ShowNpcOrGoSpawnInformation(uint32 guid)
{
    if (uint16 pool_id = sPoolMgr.IsPartOfAPool<T>(guid))
    {
        uint16 top_pool_id = sPoolMgr.IsPartOfTopPool<Pool>(pool_id);
        if (!top_pool_id || top_pool_id == pool_id)
            PSendSysMessage(LANG_NPC_GO_INFO_POOL, pool_id);
        else
            PSendSysMessage(LANG_NPC_GO_INFO_TOP_POOL, pool_id, top_pool_id);

        if (int16 event_id = sGameEventMgr.GetGameEventId<Pool>(top_pool_id))
        {
            GameEventMgr::GameEventDataMap const& events = sGameEventMgr.GetEventMap();
            GameEventData const& eventData = events[std::abs(event_id)];

            if (event_id > 0)
                PSendSysMessage(LANG_NPC_GO_INFO_POOL_GAME_EVENT_S, top_pool_id, std::abs(event_id), eventData.description.c_str());
            else
                PSendSysMessage(LANG_NPC_GO_INFO_POOL_GAME_EVENT_D, top_pool_id, std::abs(event_id), eventData.description.c_str());
        }
    }
    else if (int16 event_id = sGameEventMgr.GetGameEventId<T>(guid))
    {
        GameEventMgr::GameEventDataMap const& events = sGameEventMgr.GetEventMap();
        GameEventData const& eventData = events[std::abs(event_id)];

        if (event_id > 0)
            PSendSysMessage(LANG_NPC_GO_INFO_GAME_EVENT_S, std::abs(event_id), eventData.description.c_str());
        else
            PSendSysMessage(LANG_NPC_GO_INFO_GAME_EVENT_D, std::abs(event_id), eventData.description.c_str());
    }
}

// Prepare ShortString for a NPC or GO (by guid) with pool or game event IDs
template <typename T>
std::string ChatHandler::PrepareStringNpcOrGoSpawnInformation(uint32 guid)
{
    std::string str = "";
    if (uint16 pool_id = sPoolMgr.IsPartOfAPool<T>(guid))
    {
        uint16 top_pool_id = sPoolMgr.IsPartOfTopPool<T>(guid);
        if (int16 event_id = sGameEventMgr.GetGameEventId<Pool>(top_pool_id))
        {
            char buffer[100];
            const char* format = GetMangosString(LANG_NPC_GO_INFO_POOL_EVENT_STRING);
            sprintf(buffer, format, pool_id, event_id);
            str = buffer;
        }
        else
        {
            char buffer[100];
            const char* format = GetMangosString(LANG_NPC_GO_INFO_POOL_STRING);
            sprintf(buffer, format, pool_id);
            str = buffer;
        }
    }
    else if (int16 event_id = sGameEventMgr.GetGameEventId<T>(guid))
    {
        char buffer[100];
        const char* format = GetMangosString(LANG_NPC_GO_INFO_EVENT_STRING);
        sprintf(buffer, format, event_id);
        str = buffer;
    }

    return str;
}

// Instantiate template for helper function
template void ChatHandler::ShowNpcOrGoSpawnInformation<Creature>(uint32 guid);
template void ChatHandler::ShowNpcOrGoSpawnInformation<GameObject>(uint32 guid);

template std::string ChatHandler::PrepareStringNpcOrGoSpawnInformation<Creature>(uint32 guid);
template std::string ChatHandler::PrepareStringNpcOrGoSpawnInformation<GameObject>(uint32 guid);
