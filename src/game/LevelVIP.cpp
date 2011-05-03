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