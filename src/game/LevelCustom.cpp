#include "Chat.h"
#include "SpellAuras.h"
#include "Language.h"
#include "World.h"

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

    SendGlobalSysMessage("DB table `DBC_Spell` reloaded.");

    return true;
}
