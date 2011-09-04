#include "Chat.h"
#include "SpellAuras.h"
#include "Language.h"
#include "World.h"

#include "GridNotifiers.h"
#include "MoveMap.h"                                        // for mmap manager
#include "PathFinder.h"                                     // for mmap commands
#include "Database/DatabaseEnv.h"

bool ChatHandler::HandleUnsicknessCommand(char* /*args*/)
{    
    m_session->GetPlayer()->RemoveAurasDueToSpell(SPELL_ID_PASSIVE_RESURRECTION_SICKNESS);

    return true;
}

bool ChatHandler::HandleDungeonTokenCommand(char* args)
{
    uint32 itemId = 23580;
	int32 count;

	// ak nieje parameter, pocet je defaultne 1
    if (!*args)
        count = 1;
	else
        count = strtol(args, NULL, 10);

	// ak je vlozena hodnota chybna, vrati 0
    if (count == 0)
        return false;

	// Ziskanie targetu hraca
    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
	// Ak nieje target, da Eventerovi samotnemu.
	//TODO: ma to vobec vyznam?
    if(!plTarget)
        plTarget = pl;

	// Sprava do Logu (len pri detajlnom log-ovani)
    sLog.outDetail(GetMangosString(LANG_ADDITEM), itemId, count);

	// Snaha vytvorit objekt (malo by sa podarit ale aj motyka vystreli...)
    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(itemId);
    if(!pProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
        SetSentErrorMessage(true);
        return false;
    }

    // odobratie itemu (zaporny pocet)
    if (count < 0)
    {
        plTarget->DestroyItemCount(itemId, -count, true, false);
        PSendSysMessage(LANG_REMOVEITEM, itemId, -count, GetNameLink(plTarget).c_str());
        return true;
    }

    // Pridanie itemu
    uint32 noSpaceForCount = 0;

    // kontrola miesta v bagu a najdenie miesta
    ItemPosCountVec dest;
    uint8 msg = plTarget->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount );
    if( msg != EQUIP_ERR_OK )                               // convert to possible store amount
        count -= noSpaceForCount;

    if( count == 0 || dest.empty())                         // can't add any
    {
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount );
        SetSentErrorMessage(true);
        return false;
    }

    Item* item = plTarget->StoreNewItem( dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

    // Odstranenie bindovania ak gm si da item sam aby ho mohol tradeovat
	// 
	// TODO ma to vyznam?
	/*
    if(pl==plTarget)
        for(ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
            if(Item* item1 = pl->GetItemByPos(itr->pos))
                item1->SetBinding( false );
	*/

	// Pridanie itemu
    if(count > 0 && item)
    {
        pl->SendNewItem(item,count,false,true);
        if(pl!=plTarget)
            plTarget->SendNewItem(item,count,true,false);
    }

	// Chyba pri nedostatku miesta
    if(noSpaceForCount > 0)
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

    return true;
}

bool ChatHandler::HandleDungeonMarkCommand(char* args)
{
    uint32 itemId = 34368;
	int32 count;

	// ak nieje parameter, pocet je defaultne 1
    if (!*args)
        count = 1;
	else
        count = strtol(args, NULL, 10);

	// ak je vlozena hodnota chybna, vrati 0
    if (count == 0)
        return false;

	// Ziskanie targetu hraca
    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
	// Ak nieje target, da Eventerovi samotnemu.
	//TODO: ma to vobec vyznam?
    if(!plTarget)
        plTarget = pl;

	// Sprava do Logu (len pri detajlnom log-ovani)
    sLog.outDetail(GetMangosString(LANG_ADDITEM), itemId, count);

	// Snaha vytvorit objekt (malo by sa podarit ale aj motyka vystreli...)
    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(itemId);
    if(!pProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
        SetSentErrorMessage(true);
        return false;
    }

    // odobratie itemu (zaporny pocet)
    if (count < 0)
    {
        plTarget->DestroyItemCount(itemId, -count, true, false);
        PSendSysMessage(LANG_REMOVEITEM, itemId, -count, GetNameLink(plTarget).c_str());
        return true;
    }

    // Pridanie itemu
    uint32 noSpaceForCount = 0;

    // kontrola miesta v bagu a najdenie miesta
    ItemPosCountVec dest;
    uint8 msg = plTarget->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount );
    if( msg != EQUIP_ERR_OK )                               // convert to possible store amount
        count -= noSpaceForCount;

    if( count == 0 || dest.empty())                         // can't add any
    {
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount );
        SetSentErrorMessage(true);
        return false;
    }

    Item* item = plTarget->StoreNewItem( dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

    // Odstranenie bindovania ak gm si da item sam aby ho mohol tradeovat
	// 
	// TODO ma to vyznam?
	/*
    if(pl==plTarget)
        for(ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
            if(Item* item1 = pl->GetItemByPos(itr->pos))
                item1->SetBinding( false );
	*/

	// Pridanie itemu
    if(count > 0 && item)
    {
        pl->SendNewItem(item,count,false,true);
        if(pl!=plTarget)
            plTarget->SendNewItem(item,count,true,false);
    }

	// Chyba pri nedostatku miesta
    if(noSpaceForCount > 0)
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

    return true;
}

bool ChatHandler::HandleHonorTokenCommand(char* args)
{
    uint32 itemId = 10663;
	int32 count;

	// ak nieje parameter, pocet je defaultne 1
    if (!*args)
        count = 1;
	else
        count = strtol(args, NULL, 10);

	// ak je vlozena hodnota chybna, vrati 0
    if (count == 0)
        return false;

	// Ziskanie targetu hraca
    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
	// Ak nieje target, da Eventerovi samotnemu.
	//TODO: ma to vobec vyznam?
    if(!plTarget)
        plTarget = pl;

	// Sprava do Logu (len pri detajlnom log-ovani)
    sLog.outDetail(GetMangosString(LANG_ADDITEM), itemId, count);

	// Snaha vytvorit objekt (malo by sa podarit ale aj motyka vystreli...)
    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(itemId);
    if(!pProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
        SetSentErrorMessage(true);
        return false;
    }

    // odobratie itemu (zaporny pocet)
    if (count < 0)
    {
        plTarget->DestroyItemCount(itemId, -count, true, false);
        PSendSysMessage(LANG_REMOVEITEM, itemId, -count, GetNameLink(plTarget).c_str());
        return true;
    }

    // Pridanie itemu
    uint32 noSpaceForCount = 0;

    // kontrola miesta v bagu a najdenie miesta
    ItemPosCountVec dest;
    uint8 msg = plTarget->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount );
    if( msg != EQUIP_ERR_OK )                               // convert to possible store amount
        count -= noSpaceForCount;

    if( count == 0 || dest.empty())                         // can't add any
    {
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount );
        SetSentErrorMessage(true);
        return false;
    }

    Item* item = plTarget->StoreNewItem( dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

    // Odstranenie bindovania ak gm si da item sam aby ho mohol tradeovat
	// 
	// TODO ma to vyznam?
	/*
    if(pl==plTarget)
        for(ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
            if(Item* item1 = pl->GetItemByPos(itr->pos))
                item1->SetBinding( false );
	*/

	// Pridanie itemu
    if(count > 0 && item)
    {
        pl->SendNewItem(item,count,false,true);
        if(pl!=plTarget)
            plTarget->SendNewItem(item,count,true,false);
    }

	// Chyba pri nedostatku miesta
    if(noSpaceForCount > 0)
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

    return true;
}

bool ChatHandler::HandleHonorMarkCommand(char* args)
{
    uint32 itemId = 10662;
	int32 count;

	// ak nieje parameter, pocet je defaultne 1
    if (!*args)
        count = 1;
	else
        count = strtol(args, NULL, 10);

	// ak je vlozena hodnota chybna, vrati 0
    if (count == 0)
        return false;

	// Ziskanie targetu hraca
    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
	// Ak nieje target, da Eventerovi samotnemu.
	//TODO: ma to vobec vyznam?
    if(!plTarget)
        plTarget = pl;

	// Sprava do Logu (len pri detajlnom log-ovani)
    sLog.outDetail(GetMangosString(LANG_ADDITEM), itemId, count);

	// Snaha vytvorit objekt (malo by sa podarit ale aj motyka vystreli...)
    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(itemId);
    if(!pProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
        SetSentErrorMessage(true);
        return false;
    }

    // odobratie itemu (zaporny pocet)
    if (count < 0)
    {
        plTarget->DestroyItemCount(itemId, -count, true, false);
        PSendSysMessage(LANG_REMOVEITEM, itemId, -count, GetNameLink(plTarget).c_str());
        return true;
    }

    // Pridanie itemu
    uint32 noSpaceForCount = 0;

    // kontrola miesta v bagu a najdenie miesta
    ItemPosCountVec dest;
    uint8 msg = plTarget->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount );
    if( msg != EQUIP_ERR_OK )                               // convert to possible store amount
        count -= noSpaceForCount;

    if( count == 0 || dest.empty())                         // can't add any
    {
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount );
        SetSentErrorMessage(true);
        return false;
    }

    Item* item = plTarget->StoreNewItem( dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

    // Odstranenie bindovania ak gm si da item sam aby ho mohol tradeovat
	// 
	// TODO ma to vyznam?
	/*
    if(pl==plTarget)
        for(ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
            if(Item* item1 = pl->GetItemByPos(itr->pos))
                item1->SetBinding( false );
	*/

	// Pridanie itemu
    if(count > 0 && item)
    {
        pl->SendNewItem(item,count,false,true);
        if(pl!=plTarget)
            plTarget->SendNewItem(item,count,true,false);
    }

	// Chyba pri nedostatku miesta
    if(noSpaceForCount > 0)
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

    return true;
}

bool ChatHandler::HandleFreezeCommand(char *args)
{
    std::string name;
    Player* player;
    char* TargetName = strtok((char*)args, " "); //get entered #name
    if (!TargetName) //if no #name entered use target
    {
        player = getSelectedPlayer();
		if (player) //prevent crash with creature as target
        {   
           name = player->GetName();
           normalizePlayerName(name);
        }
    }
    else // if #name entered
    {
        name = TargetName;
        normalizePlayerName(name);
        player = sObjectMgr.GetPlayer(name.c_str()); //get player by #name
    }


    //effect
    if ((player) && (!(player==m_session->GetPlayer())))
    {
        PSendSysMessage(LANG_COMMAND_FREEZE,name.c_str());

        //stop combat + unattackable + duel block + stop some spells
        player->setFaction(35);
        player->CombatStop();
		if(player->IsNonMeleeSpellCasted(true))
        player->InterruptNonMeleeSpells(true);
        player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        player->SetUInt32Value(PLAYER_DUEL_TEAM, 1);

		//if player class = hunter || warlock remove pet if alive
        if((player->getClass() == CLASS_HUNTER) || (player->getClass() == CLASS_WARLOCK))
        {
            if(Pet* pet = player->GetPet())
            {
                pet->SavePetToDB(PET_SAVE_AS_CURRENT);
                // not let dismiss dead pet
                if(pet && pet->isAlive())
                player->RemovePet(pet,PET_SAVE_NOT_IN_SLOT);
            }
        }

        //stop movement and disable spells
        //uint32 spellID = 9454;
        //m_session->GetPlayer()->CastSpell(player,spellID,false);
        uint32 spellID = 9454;
        SpellEntry const *spellInfo = sSpellStore.LookupEntry( spellID );
        if(spellInfo) //TODO: Change the duration of the aura to -1 instead of 5000000
        {
            for(uint32 i = 0;i<3;i++)
            {
                uint8 eff = spellInfo->Effect[i];
                if (eff>=TOTAL_SPELL_EFFECTS)
                    continue;
                if( eff == SPELL_EFFECT_APPLY_AREA_AURA_PARTY || eff == SPELL_EFFECT_APPLY_AURA ||
                    eff == SPELL_EFFECT_PERSISTENT_AREA_AURA || eff == SPELL_EFFECT_APPLY_AREA_AURA_FRIEND || 
                    eff == SPELL_EFFECT_APPLY_AREA_AURA_ENEMY)
                {
                    SpellEffectIndex si;
                    switch(i)
                    {
                        case 0: si=EFFECT_INDEX_0;break;
                        case 1: si=EFFECT_INDEX_1;break;
                        default:si=EFFECT_INDEX_2;
					}
                    Aura *Aur = CreateAura(spellInfo, si, NULL, player);
                    player->AddAura(Aur);
                }
            }
        }


        //save player
        player->SaveToDB();
    }

    if (!player)
    {
        SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
        return true;
    }

    if (player==m_session->GetPlayer())
    {
        SendSysMessage(LANG_COMMAND_FREEZE_ERROR);
        return true;
    }

    return true;
}

bool ChatHandler::HandleUnFreezeCommand(char *args)
{
    std::string name;
    Player* player;
    char* TargetName = strtok((char*)args, " "); //get entered #name
    if (!TargetName) //if no #name entered use target
    {
        player = getSelectedPlayer();
		if (player) //prevent crash with creature as target
        {   
           name = player->GetName();
        }
    }

    else // if #name entered
    {
        name = TargetName;
        normalizePlayerName(name);
        player = sObjectMgr.GetPlayer(name.c_str()); //get player by #name
    }

    //effect
    if (player)
    {
        PSendSysMessage(LANG_COMMAND_UNFREEZE,name.c_str());

        //Reset player faction + allow combat + allow duels
        player->setFactionForRace(player->getRace());
        player->RemoveFlag (UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

        //allow movement and spells
        uint32 spellID = 9454;
        player->RemoveAurasDueToSpell(spellID);

        //save player
        player->SaveToDB();
    }

    if (!player)
    {
        if (TargetName)
        {        
            //check for offline players
		    QueryResult *result = CharacterDatabase.PQuery("SELECT characters.guid FROM `characters` WHERE characters.name = '%s'",name.c_str());
            if(!result)
		    {
			    SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
                return true;
		    }
		    //if player found: delete his freeze aura
		    Field *fields=result->Fetch();
            uint64 pguid = fields[0].GetUInt64();
		    delete result;
            CharacterDatabase.PQuery("DELETE FROM `character_aura` WHERE character_aura.spell = 9454 AND character_aura.guid = '%u'",pguid);
            PSendSysMessage(LANG_COMMAND_UNFREEZE,name.c_str());
            return true;
		}
		else
        {
	        SendSysMessage(LANG_COMMAND_FREEZE_WRONG);
            return true;
		}
    }

    return true;
}

bool ChatHandler::HandleListFreezeCommand(char* args)
{
    //Get names from DB
    QueryResult *result = CharacterDatabase.PQuery("SELECT characters.name FROM `characters` LEFT JOIN `character_aura` ON (characters.guid = character_aura.guid) WHERE character_aura.spell = 9454");
    if(!result)
    {
        SendSysMessage(LANG_COMMAND_NO_FROZEN_PLAYERS);
        return true;
    }
    //Header of the names
    PSendSysMessage(LANG_COMMAND_LIST_FREEZE);
    
    //Output of the results
	do
    {
        Field *fields = result->Fetch();
        std::string fplayers = fields[0].GetCppString();
        PSendSysMessage(LANG_COMMAND_FROZEN_PLAYERS,fplayers.c_str());
    } while (result->NextRow());

    delete result;
    return true;
}

bool ChatHandler::HandleGoCheaterCommand(char* args)
{
    if (!*args)
        return false;

    int id = atoi((char*)args);
   
    //query na polohu
    QueryResult *result = CharacterDatabase.PQuery("SELECT x, y, z, Map FROM cheaters WHERE entry = %d", id);

    // je niejaky vysledok?
    if(!result)
    {
        SendSysMessage("Neexistujuce ID!");
        SetSentErrorMessage(true);
        return false;
    }
   
    float x,y,z;
    uint16 mapid;

    // ziskanie jedneho riadku (a vlastne aj jedineho)
    Field *fields = result->Fetch();

    // ziskanie hodnot z db
    x= fields[0].GetFloat();
    y= fields[1].GetFloat();
    z= fields[2].GetFloat();
    mapid= fields[3].GetUInt16();

    // Uvolnenie pamate
    delete result;

    // teleport
    m_session->GetPlayer()->TeleportTo(mapid,x,y,z,0.0f);

    return true;
}

bool ChatHandler::HandleAddItemToAllCommand(char* args)
{
    if (!*args)
        return false;

    uint32 itemId = 0;

    // Vyextrahovanie mena itemu
    if(args[0]=='[')                                        // [name] manual form
    {
        char* citemName = citemName = strtok((char*)args, "]");

        if(citemName && citemName[0])
        {
            std::string itemName = citemName+1;
            WorldDatabase.escape_string(itemName);
            QueryResult *result = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE name = '%s'", itemName.c_str());
            if (!result)
            {
                PSendSysMessage(LANG_COMMAND_COULDNOTFIND, citemName+1);
                SetSentErrorMessage(true);
                return false;
            }
            itemId = result->Fetch()->GetUInt16();
            delete result;
        }
        else
            return false;
    }
    else                                                    // item_id or [name] Shift-click form |color|Hitem:item_id:0:0:0|h[name]|h|r
    {
        char* cId = ExtractKeyFromLink(&args,"Hitem");
        if(!cId)
            return false;
        itemId = atol(cId);
    }

	// Ziskanie poctu itemov (nepovinny parameter, defaultne 1)
    char* ccount = strtok(NULL, " ");

    int32 countproto = 1;

    if (ccount)
        countproto = strtol(ccount, NULL, 10);

    if (countproto == 0)
        countproto = 1;

	//Odrstranenie itemov sa nepovoluje
    if (countproto < 0)
    {
        // TODO bodol by aj vypis
        return false;
    }

    // Ziskanie typu itemu
    ItemPrototype const *pProto = sObjectMgr.GetItemPrototype(itemId);
    if(!pProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
        SetSentErrorMessage(true);
        return false;
    }

	Player* pl = m_session->GetPlayer();

    // Prechod vsetkymi hracmi servra
    HashMapHolder<Player>::MapType& m = ObjectAccessor::Instance().GetPlayers();
    for(HashMapHolder<Player>::MapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        int32 count = countproto;

        Player* plTarget = itr->second;

        //Adding items
        uint32 noSpaceForCount = 0;

        // check space and find places
        ItemPosCountVec dest;
        uint8 msg = plTarget->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount );
        if( msg != EQUIP_ERR_OK )                               // convert to possible store amount
            count -= noSpaceForCount;

        if( count == 0 || dest.empty())                         // can't add any
        {
            PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount );
            SetSentErrorMessage(true);
            continue;
        }

        Item* item = plTarget->StoreNewItem( dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

        if(count > 0 && item)
        {
            pl->SendNewItem(item,count,false,true);
            if(pl!=plTarget)
                plTarget->SendNewItem(item,count,true,false);
        }

        if(noSpaceForCount > 0)
            PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
    }

    return true;
}

/// Zobrazi vip hracov online
bool ChatHandler::HandleVIPOnlineCommand(char* args)
{
    uint32 count=0;

    HashMapHolder<Player>::MapType& m = sObjectAccessor.GetPlayers();
    for(HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
        if(itr->second->GetSession()->GetSecurity() == SECURITY_VIP)
        {            
            QueryResult* result = LoginDatabase.PQuery("SELECT username FROM account WHERE id = '%u'",itr->second->GetSession()->GetAccountId());
            if(result)
            {
                Field* fields = result->Fetch();
				
                if(count++ == 0)
                    SendSysMessage("VIP hraci online:");

                PSendSysMessage(" %s (Konto:'%s')", itr->second->GetName(), fields[0].GetString());

                delete result;
            }
        }

    if(count == 0)
        SendSysMessage("V hre nie je ziadne vip.");

    return true;
}

bool ChatHandler::HandleBanMailCommand(char* args)
{
    if(!args)
        return false;

    std::string mail = args;

    if(mail.length() == 0)
           return false;

    LoginDatabase.escape_string(mail);

    LoginDatabase.PExecute("INSERT INTO `mail_banned` (`mail`) VALUES ('%s')", mail.c_str());

    PSendSysMessage("E-mail '%s' pridany do zoznamu zabanovanych mailov", args);

    return true;
}

// global announce
bool ChatHandler::HandleAnnounceCommand(char* args)
{
    if(!*args)
        return false;

    if(m_session)
    {
        // Eventerov announce
        if(m_session->GetSecurity() == SECURITY_EVENTMASTER)
            sWorld.SendWorldText(LANG_EVENT_ANNOUNCE,m_session->GetPlayerName(),args);

        // Moderatorov announce
        else if(m_session->GetSecurity() == SECURITY_MODERATOR)
            sWorld.SendWorldText(LANG_MODERATOR_ANNOUNCE,m_session->GetPlayerName(),args);

        // GMkove announce
        else if(m_session->GetSecurity()== SECURITY_GAMEMASTER)
            sWorld.SendWorldText(LANG_SYSTEMMESSAGE,m_session->GetPlayerName(),args);
        
        // Developerov announce
        else if(m_session->GetSecurity() == SECURITY_DEVELOPER)
            sWorld.SendWorldText(LANG_DEVELOPER_ANNOUNCE,m_session->GetPlayerName(),args);

        // Adminove announce
        else
            sWorld.SendWorldText(LANG_ADMIN_ANNOUNCE,m_session->GetPlayerName(),args);
    }
    // Announce konzoly
    else
        sWorld.SendWorldText(LANG_CONSOLE_ANNOUNCE,args);

    return true;
}

bool ChatHandler::HandleReloadDBCCommand(char* args)
{
    sWorld.CustomizeDBCData();

    SendGlobalSysMessage("DB tables `DBC_*` reloaded.");

    return true;
}

//Play sound for all online players
bool ChatHandler::HandlePlaySoundToAllCommand(char* args)
{
    // USAGE: .debug playsound #soundid
    // #soundid - ID decimal number from SoundEntries.dbc (1st column)
    uint32 dwSoundId;
    if (!ExtractUInt32(&args, dwSoundId))
        return false;

    if (!sSoundEntriesStore.LookupEntry(dwSoundId))
    {
        PSendSysMessage(LANG_SOUND_NOT_EXIST, dwSoundId);
        SetSentErrorMessage(true);
        return false;
    }

    HashMapHolder<Player>::MapType& m = ObjectAccessor::Instance().GetPlayers();
    for(HashMapHolder<Player>::MapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        itr->second->PlayDirectSound(dwSoundId, itr->second);

    PSendSysMessage(LANG_YOU_HEAR_SOUND, dwSoundId);
    return true;
}

bool ChatHandler::HandleMmapPathCommand(char* args)
{
    if (!MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(m_session->GetPlayer()->GetMapId()))
    {
        PSendSysMessage("NavMesh not loaded for current map.");
        return true;
    }

    PSendSysMessage("mmap path:");

    // units
    Player* player = m_session->GetPlayer();
    Unit* target = getSelectedUnit();
    if (!player || !target)
    {
        PSendSysMessage("Invalid target/source selection.");
        return true;
    }

    char* para = strtok(args, " ");

    bool useStraightPath = false;
    if (para && strcmp(para, "true") == 0)
        useStraightPath = true;

    // unit locations
    float x, y, z;
    player->GetPosition(x, y, z);

    // path
    PathInfo path(target, x, y, z, useStraightPath);
    PointPath pointPath = path.getFullPath();
    PSendSysMessage("%s's path to %s:", target->GetName(), player->GetName());
    PSendSysMessage("Building %s", useStraightPath ? "StraightPath" : "SmoothPath");
    PSendSysMessage("length %i type %u", pointPath.size(), path.getPathType());

    PathNode start = path.getStartPosition();
    PathNode next = path.getNextPosition();
    PathNode end = path.getEndPosition();
    PathNode actualEnd = path.getActualEndPosition();

    PSendSysMessage("start      (%.3f, %.3f, %.3f)", start.x, start.y, start.z);
    PSendSysMessage("next       (%.3f, %.3f, %.3f)", next.x, next.y, next.z);
    PSendSysMessage("end        (%.3f, %.3f, %.3f)", end.x, end.y, end.z);
    PSendSysMessage("actual end (%.3f, %.3f, %.3f)", actualEnd.x, actualEnd.y, actualEnd.z);

    if (!player->isGameMaster())
        PSendSysMessage("Enable GM mode to see the path points.");

    // this entry visible only to GM's with "gm on"
    static const uint32 WAYPOINT_NPC_ENTRY = 1;
    Creature* wp = NULL;
    for (uint32 i = 0; i < pointPath.size(); ++i)
    {
        wp = player->SummonCreature(WAYPOINT_NPC_ENTRY, pointPath[i].x, pointPath[i].y, pointPath[i].z, 0, TEMPSUMMON_TIMED_DESPAWN, 9000);
        // TODO: make creature not sink/fall
    }

    return true;
}

bool ChatHandler::HandleMmapLocCommand(char* /*args*/)
{
    PSendSysMessage("mmap tileloc:");

    // grid tile location
    Player* player = m_session->GetPlayer();

    int32 gx = 32 - player->GetPositionX() / SIZE_OF_GRIDS;
    int32 gy = 32 - player->GetPositionY() / SIZE_OF_GRIDS;

    PSendSysMessage("%03u%02i%02i.mmtile", player->GetMapId(), gy, gx);
    PSendSysMessage("gridloc [%i,%i]", gx, gy);

    // calculate navmesh tile location
    const dtNavMesh* navmesh = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(player->GetMapId());
    const dtNavMeshQuery* navmeshquery = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMeshQuery(player->GetMapId(), player->GetInstanceId());
    if (!navmesh || !navmeshquery)
    {
        PSendSysMessage("NavMesh not loaded for current map.");
        return true;
    }

    const float* min = navmesh->getParams()->orig;

    float x, y, z;
    player->GetPosition(x, y, z);
    float location[VERTEX_SIZE] = {y, z, x};
    float extents[VERTEX_SIZE] = {2.f,4.f,2.f};

    int32 tilex = int32((y - min[0]) / SIZE_OF_GRIDS);
    int32 tiley = int32((x - min[2]) / SIZE_OF_GRIDS);

    PSendSysMessage("Calc   [%02i,%02i]", tilex, tiley);

    // navmesh poly -> navmesh tile location
    dtQueryFilter filter = dtQueryFilter();
    dtPolyRef polyRef = INVALID_POLYREF;
    navmeshquery->findNearestPoly(location, extents, &filter, &polyRef, NULL);

    if (polyRef == INVALID_POLYREF)
        PSendSysMessage("Dt     [??,??] (invalid poly, probably no tile loaded)");
    else
    {
        const dtMeshTile* tile;
        const dtPoly* poly;
        navmesh->getTileAndPolyByRef(polyRef, &tile, &poly);
        if (tile)
            PSendSysMessage("Dt     [%02i,%02i]", tile->header->x, tile->header->y);
        else
            PSendSysMessage("Dt     [??,??] (no tile loaded)");
    }

    return true;
}

bool ChatHandler::HandleMmapLoadedTilesCommand(char* /*args*/)
{
    uint32 mapid = m_session->GetPlayer()->GetMapId();

    const dtNavMesh* navmesh = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(mapid);
    const dtNavMeshQuery* navmeshquery = MMAP::MMapFactory::createOrGetMMapManager()->GetNavMeshQuery(mapid, m_session->GetPlayer()->GetInstanceId());
    if (!navmesh || !navmeshquery)
    {
        PSendSysMessage("NavMesh not loaded for current map.");
        return true;
    }

    PSendSysMessage("mmap loadedtiles:");

    for (int32 i = 0; i < navmesh->getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navmesh->getTile(i);
        if (!tile || !tile->header)
            continue;

        PSendSysMessage("[%02i,%02i]", tile->header->x, tile->header->y);
    }

    return true;
}

bool ChatHandler::HandleMmapStatsCommand(char* /*args*/)
{
    PSendSysMessage("mmap stats:");
    PSendSysMessage("  global mmap pathfinding is %sabled", sWorld.getConfig(CONFIG_BOOL_MMAP_ENABLED) ? "en" : "dis");

    MMAP::MMapManager *manager = MMAP::MMapFactory::createOrGetMMapManager();
    PSendSysMessage(" %u maps loaded with %u tiles overall", manager->getLoadedMapsCount(), manager->getLoadedTilesCount());

    const dtNavMesh* navmesh = manager->GetNavMesh(m_session->GetPlayer()->GetMapId());
    if (!navmesh)
    {
        PSendSysMessage("NavMesh not loaded for current map.");
        return true;
    }

    uint32 tileCount = 0;
    uint32 nodeCount = 0;
    uint32 polyCount = 0;
    uint32 vertCount = 0;
    uint32 triCount = 0;
    uint32 triVertCount = 0;
    uint32 dataSize = 0;
    for (int32 i = 0; i < navmesh->getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navmesh->getTile(i);
        if (!tile || !tile->header)
            continue;

        tileCount ++;
        nodeCount += tile->header->bvNodeCount;
        polyCount += tile->header->polyCount;
        vertCount += tile->header->vertCount;
        triCount += tile->header->detailTriCount;
        triVertCount += tile->header->detailVertCount;
        dataSize += tile->dataSize;
    }

    PSendSysMessage("Navmesh stats on current map:");
    PSendSysMessage(" %u tiles loaded", tileCount);
    PSendSysMessage(" %u BVTree nodes", nodeCount);
    PSendSysMessage(" %u polygons (%u vertices)", polyCount, vertCount);
    PSendSysMessage(" %u triangles (%u vertices)", triCount, triVertCount);
    PSendSysMessage(" %.2f MB of data (not including pointers)", ((float)dataSize / sizeof(unsigned char)) / 1048576);

    return true;
}

bool ChatHandler::HandleMmap(char* args)
{
    bool on;
    if (ExtractOnOff(&args, on))
    {
        if (on)
        {
            sWorld.setConfig(CONFIG_BOOL_MMAP_ENABLED, true);
            SendSysMessage("WORLD: mmaps are now ENABLED (individual map settings still in effect)");
        }
        else
        {
            sWorld.setConfig(CONFIG_BOOL_MMAP_ENABLED, false);
            SendSysMessage("WORLD: mmaps are now DISABLED");
        }
        return true;
    }

    on = sWorld.getConfig(CONFIG_BOOL_MMAP_ENABLED);
    PSendSysMessage("mmaps are %sabled", on ? "en" : "dis");

    return true;
}

bool ChatHandler::HandleMmapTestArea(char* args)
{/*
    float radius = 40.0f;
    ExtractFloat(&args, radius);

    CellPair pair(MaNGOS::ComputeCellPair( m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY()) );
    Cell cell(pair);
    cell.SetNoCreate();

    std::list<Creature*> creatureList;

    MaNGOS::AnyUnitInObjectRangeCheck go_check(m_session->GetPlayer(), radius);
    MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> go_search(creatureList, go_check);
    TypeContainerVisitor<MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck>, GridTypeMapContainer> go_visit(go_search);
    
    // Get Creatures
    cell.Visit(pair, go_visit, *(m_session->GetPlayer()->GetMap()), *(m_session->GetPlayer()), radius);
    
    if (!creatureList.empty())
    {
        PSendSysMessage("Found %i Creatures.", creatureList.size());

        uint32 paths = 0;
        uint32 uStartTime = WorldTimer::getMSTime();

        float gx,gy,gz;
        m_session->GetPlayer()->GetPosition(gx,gy,gz);
        for (std::list<Creature*>::iterator itr = creatureList.begin(); itr != creatureList.end(); ++itr)
        {
            PathInfo((*itr), gx, gy, gz);
            ++paths;
        }

        uint32 uPathLoadTime = WorldTimer::getMSTimeDiff(uStartTime, WorldTimer::getMSTime());
        PSendSysMessage("Generated %i paths in %i ms", paths, uPathLoadTime);
    }
    else
    {
        PSendSysMessage("No creatures in %f yard range.", radius);
    }
    return true;
    */

    PSendSysMessage("Not implemented yet.");

    return false;
}

void ChatHandler::dajVipBonus(Player *target)
{
    char param[15];

    // 100 000g 00s 00c
    strcpy(param, "1000000000");
    if(!HandleModifyMoneyCommand(param))
        SendSysMessage("Chyba pri penazi");

    // 4 x 21876 - Primal Mooncloth bag
    strcpy(param, "21876 4");
    if(!HandleAddItemCommand(param))
        SendSysMessage("Chyba pri pridavani bagov");

    // 70 level
    HandleCharacterLevel(target, target->GetGUID(), target->getLevel(), 70);

    SendSysMessage("VIP vybavene");

}

bool ChatHandler::HandleSetVIPCommmand(char* args)
{
    Player *target = getSelectedPlayer();
    if (!target || target->GetTypeId() != TYPEID_PLAYER)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    if(target->GetSession()->GetSecurity() != SECURITY_VIP)
    {
        SendSysMessage("Target nie je VIP");
        SetSentErrorMessage(true);
        return false;
    }

    QueryResult* result = LoginDatabase.PQuery("SELECT VIP_char_comment FROM account WHERE id = %u", target->GetSession()->GetAccountId());
    if(result)
    {
        Field* fields = result->Fetch();
        if(!fields[0].IsNULL())
        {
            SendSysMessage("Hrac uz ma vybavene VIP.");
            SetSentErrorMessage(true);
            return false;
        }        
        delete result;
    }
    else
    {
        SendSysMessage("Internal error. Toto by sa stat nemalo.");
        SetSentErrorMessage(true);
        return false;
    }

    dajVipBonus(target);

    // Zaznam do db
    LoginDatabase.PExecute("UPDATE account SET VIP_char_comment='%s' WHERE id = %u", target->GetName(), target->GetSession()->GetAccountId());

    return true;
}

bool ChatHandler::HandlePridajVIPCommmand(char* args)
{
    Player *target = getSelectedPlayer();
    if (!target || target->GetTypeId() != TYPEID_PLAYER)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    if(target->GetSession()->GetSecurity() != SECURITY_VIP)
    {
        SendSysMessage("Target nie je VIP");
        SetSentErrorMessage(true);
        return false;
    }

    QueryResult* result = LoginDatabase.PQuery("SELECT VIP_char_comment FROM account WHERE id = %u", target->GetSession()->GetAccountId());
    if(result)
    {
        Field* fields = result->Fetch();
        if(fields[0].IsNULL())
        {
            SendSysMessage("Hrac este nema vybavene ziadne VIP.");
            SetSentErrorMessage(true);
            return false;
        }        
        delete result;
    }
    else
    {
        SendSysMessage("Internal error. Toto by sa stat nemalo.");
        SetSentErrorMessage(true);
        return false;
    }

    dajVipBonus(target);

    // Zaznam do db
    LoginDatabase.PExecute("UPDATE account SET VIP_char_comment=CONCAT(VIP_char_comment, ' ', '%s') WHERE id = %u", target->GetName(), target->GetSession()->GetAccountId());

    return true;
}

/// Zobrazi nevybavenych vip hracov, ktori su online
bool ChatHandler::HandleNevybaveneVIPOnlineCommand(char* args)
{
    uint32 count=0;

    HashMapHolder<Player>::MapType& m = sObjectAccessor.GetPlayers();
    for(HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
        if(itr->second->GetSession()->GetSecurity() == SECURITY_VIP)
        {
            //                                                    0              1
            QueryResult* result = LoginDatabase.PQuery("SELECT username, VIP_char_comment FROM account WHERE id = '%u'",itr->second->GetSession()->GetAccountId());
            if(result)
            {
                Field* fields = result->Fetch();
				
                if(!fields[1].IsNULL())
                    continue;

                if(count++ == 0)
                    SendSysMessage("Nevybaveni VIP hraci online:");

                PSendSysMessage(" %s (Konto:'%s')", itr->second->GetName(), fields[0].GetString());

                delete result;
            }
        }

    if(count == 0)
        SendSysMessage("V hre nie je ziadne nevybavene vip.");

    return true;
}

bool ChatHandler::HandleSetGHCommand(char* args)
{
    // Vyhladanie postavy v db
    std::string strargs = args;
    CharacterDatabase.escape_string(strargs);
    
    //                                                        0      1
    QueryResult* result = CharacterDatabase.PQuery("SELECT account, guid FROM characters WHERE name = '%s'", strargs.c_str());
    if(!result)
    {
        SendSysMessage("Chyba: Hrac sa nenasiel.");
        SetSentErrorMessage(true);
        return false;
    }

    // Ziskanie ID accountu a guid postavy
    Field* fields = result->Fetch();
    uint32 account_id = fields[0].GetUInt32();
    uint32 guid = fields[1].GetUInt32();

    // Zmazanie dotazu na postavu
    delete result;

    // Udaje o accounte
    //                                                      0         1            2
    QueryResult* result2 = LoginDatabase.PQuery("SELECT username, gmlevel, GuildHouse_comment FROM account WHERE id = '%u'", account_id);
    if(!result2)
    {
        SendSysMessage("wtf: postava nema ziaden account ?!");
        SetSentErrorMessage(true);
        return false;
    }

    Field* fields2 = result2->Fetch();

    // Len pre VIP hracov
    if(fields2[1].GetUInt32() != SECURITY_VIP)
    {
        SendSysMessage("Chyba: Postava nie je na VIP accounte!");
        SetSentErrorMessage(true);
        return false;
    }

    // Len pre tych, ktori este nemaju gh na svojom acce
    if(!fields2[2].IsNULL())
    {
        PSendSysMessage("Chyba: Account '%s' uz ma nastaveny GH '%s' !", fields2[0].GetString(), fields2[2].GetString());
        SetSentErrorMessage(true);
        return false;
    }

    // Zmazanie dotazu na account
    delete result2;

    // Ziskanie guildy
    //                                                       0
    QueryResult* result3 = CharacterDatabase.PQuery("SELECT name FROM guild_member JOIN guild ON guild_member.guildid = guild.guildid WHERE guild_member.guid = %u", guid);
    if(!result3)
    {
        SendSysMessage("Chyba: Hrac nie je v guilde!");
        SetSentErrorMessage(true);
        return false;
    }

    // Nazov guildy
    Field* fields3 = result3->Fetch();
    std::string guildname = fields3[0].GetCppString();
    LoginDatabase.escape_string(guildname);

    // Zaznam do db
    LoginDatabase.PExecute("UPDATE account SET GuildHouse_comment='%s' WHERE id = %u", guildname.c_str(), account_id);

    PSendSysMessage("Hracovi '%s'(%u) bola nastavena guilda '%s'", strargs, guid, fields3[0].GetCppString());

    // Zmazanie dotazu na guildu
    delete result3;

    return true;
}
