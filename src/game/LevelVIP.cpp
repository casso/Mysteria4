#include "Common.h"
#include "Player.h"
#include "WorldSession.h"
#include "Chat.h"
#include "Language.h"
#include "BattleGround.h"
#include "BattleGroundMgr.h"

bool ChatHandler::HandleWSGCommand(char* args)
{
	Player *_player = m_session->GetPlayer();

	
    if(_player->getLevel() < 70)
    {
        PSendSysMessage(LANG_LEVEL_MINREQUIRED,70);
        SetSentErrorMessage(true);
        return false;
    }

    // Zakaz prihlasovania za letu
    if(_player->IsFlying())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    // Zakaz prihlasovania v combate, bg a instanciach
    if(_player->isInCombat() || _player->GetMap()->Instanceable())
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->JoinIntoBattleground( BATTLEGROUND_WS, 0, 0);

    return true;
}
bool ChatHandler::HandleABCommand(char* args)
{
	Player *_player = m_session->GetPlayer();

	
    if(_player->getLevel() < 70)
    {
        PSendSysMessage(LANG_LEVEL_MINREQUIRED,70);
        SetSentErrorMessage(true);
        return false;
    }

    // Zakaz prihlasovania za letu
    if(_player->IsFlying())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    // Zakaz prihlasovania v combate, bg a instanciach
    if(_player->isInCombat() || _player->GetMap()->Instanceable())
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->JoinIntoBattleground( BATTLEGROUND_AB, 0, 0);

    return true;
}

bool ChatHandler::HandleAVCommand(char* args)
{
	Player *_player = m_session->GetPlayer();

	
    if(_player->getLevel() < 70)
    {
        PSendSysMessage(LANG_LEVEL_MINREQUIRED,70);
        SetSentErrorMessage(true);
        return false;
    }

    // Zakaz prihlasovania za letu
    if(_player->IsFlying())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    // Zakaz prihlasovania v combate, bg a instanciach
    if(_player->isInCombat() || _player->GetMap()->Instanceable())
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->JoinIntoBattleground( BATTLEGROUND_AV, 0, 0);

    return true;
}

bool ChatHandler::HandleEOSCommand(char* args)
{
	Player *_player = m_session->GetPlayer();

	
    if(_player->getLevel() < 70)
    {
        PSendSysMessage(LANG_LEVEL_MINREQUIRED,70);
        SetSentErrorMessage(true);
        return false;
    }

    // Zakaz prihlasovania za letu
    if(_player->IsFlying())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    // Zakaz prihlasovania v combate, bg a instanciach
    if(_player->isInCombat() || _player->GetMap()->Instanceable())
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }

    m_session->JoinIntoBattleground( BATTLEGROUND_EY, 0, 0);

    return true;
}

bool ChatHandler::HandleMCCommand(char* /*args*/)
{
    Player *chr = m_session->GetPlayer();

    // Osetrenie aby sa neportovali za letu
    if(chr->IsFlying())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    //Zakaz portovania
    if( chr->isInCombat()		// Pocas combatu
     || chr->InBattleGround()   // V BattleGrounde (aj arene)
     || chr->HasStealthAura()   // Pocas stealth-u
     || chr->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH)) // S Feign Death (hunter)
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }

    // Suradnice Mysteria centra
    chr->TeleportTo(1, 7333.01f, -1590.69f, 164.543f, 1.533f);

    // sicko tak jak ma byc
    return true;
}

bool ChatHandler::HandleGuildHouseCommand(char* /*args*/)
{
    Player *chr = m_session->GetPlayer();

    // Osetrenie aby sa neportovali za letu
    if(chr->IsFlying())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    //Zakaz portovania
    if( chr->isInCombat()		// Pocas combatu
     || chr->InBattleGround()   // V BattleGrounde (aj arene)
     || chr->HasStealthAura()   // Pocas stealth-u
     || chr->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH)) // S Feign Death (hunter)
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }

    if (!chr->TeleportToGuildHouse())
    {
        SendSysMessage("You have no guild, or your guild have no guildhouse!");

        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

bool ChatHandler::HandleFlyMountCommand(char* /*args*/)
{
    Player *chr = m_session->GetPlayer();

    // Osetrenie aby sa nemountovali za letu
    if(chr->IsFlying())
    {
        SendSysMessage(LANG_YOU_IN_FLIGHT);
        SetSentErrorMessage(true);
        return false;
    }

    // zakaz
    if( chr->isInCombat()		// Pocas combatu
     || chr->GetMap()->Instanceable()  // V instancii
     || !chr->CanFreeMove() )         // ak sa nemoze volne pohybovat (sap, taxi ...)
    {
        SendSysMessage(LANG_YOU_IN_COMBAT);
        SetSentErrorMessage(true);
        return false;
    }

    // smrt
    if(chr->isDead())
    {
        SendSysMessage("You are dead!");
        SetSentErrorMessage(true);
        return false;
    }

    // zakaz pre low lvl
    if(chr->getLevel() != 70)
    {
        SendSysMessage("Required 70 level!");
        SetSentErrorMessage(true);
        return false;
    }

    switch(chr->GetAreaId())
    {
        case 1637: // orgrimmar
        case 1519: // stormwind
        case 3487: // silvermoon
        case 168:  // Tirisfal glades sea
        case 1256: // Azshara sea
        case 4080: // ioqd
            SendSysMessage("Not allowed here!");
            SetSentErrorMessage(true);
            return false;
    }

    // cely ioqd a Diremaul
    if(chr->GetZoneId() == 4080 || chr->GetZoneId() == 2557)
    {
        SendSysMessage("Not allowed here!");
        SetSentErrorMessage(true);
        return false;
    }

    // ine mapy ako azeroth
    if(chr->GetMapId() != 0 && chr->GetMapId() != 1 && chr->GetMapId() != 530)
    {
        SendSysMessage("Not allowed here!");
        SetSentErrorMessage(true);
        return false;
    }

    Item * firstpositem = chr->GetItemByPos(INVENTORY_SLOT_BAG_0, 23);
    if(firstpositem && (        
        firstpositem->GetProto()->RequiredSkill == 762 && firstpositem->GetProto()->RequiredSkill > 150 || // 762 = riding skill
        firstpositem->GetProto()->ItemId == 34060 || // Flying Machine Control
        firstpositem->GetProto()->ItemId == 34061 ))  // Turbo-Charged Flying Machine Control
        chr->CastSpell(chr, firstpositem->GetProto()->Spells[0].SpellId, false);
    // Druidi dostanu switft flight form
    else if(chr->getClass() == CLASS_DRUID)
        chr->CastSpell(chr, 40120, false);
    // alici grifina
    else if (chr->GetTeam() == ALLIANCE)
        chr->CastSpell(chr, 32290, false);
    // horda netopiera
    else
        chr->CastSpell(chr, 32295, false);

    return true;
}
