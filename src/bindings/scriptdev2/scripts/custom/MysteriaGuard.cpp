/* ScriptData
SDName: npc_mcguard
SD%Complete: 100
SDComment: Mysteria Centrum Guard
SDCategory: Custom
Authors: Casso
Year: 2010
EndScriptData */

#include "precompiled.h"

bool GossipHello_MCGuard(Player *player, Creature *_Creature)
{
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   "House of Professions", GOSSIP_SENDER_MAIN, 1011);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    "T6, T5 and T4 vendors", GOSSIP_SENDER_MAIN, 1012);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    "Mounts and Riding", GOSSIP_SENDER_MAIN, 1001);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    "Arena S3 and S4 vendors", GOSSIP_SENDER_MAIN, 1002);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE,    "Arena organizators", GOSSIP_SENDER_MAIN, 1003);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE,    "Battleground organizators", GOSSIP_SENDER_MAIN, 1004);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_DOT,       "Training dummy", GOSSIP_SENDER_MAIN, 1005);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT,      "Bar and barber", GOSSIP_SENDER_MAIN, 1006);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "Auction", GOSSIP_SENDER_MAIN, 1010);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "Bank", GOSSIP_SENDER_MAIN, 1009);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "Guild Bank", GOSSIP_SENDER_MAIN, 1007);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK,      "Guild and Tabard Masters", GOSSIP_SENDER_MAIN, 1008);
    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK,      "Teleporter", GOSSIP_SENDER_MAIN, 1013);

    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,_Creature->GetGUID());
    return true;
}

void SendDefaultMenu_MCGuard(Player *player, Creature *_Creature, uint32 action )
{
    switch(action)
    {
        case 1001:                    // Mounti
            player->SEND_POI(7309.65f, -1549.0f, ICON_POI_REDHORSE, 6, 0, "Mounts");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
            break;
        case 1002:                    // Arena S3 a S4
            player->SEND_POI(7300.7f, -1519.7f, ICON_POI_REDFLAG, 6, 0, "S3 and S4");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
            break;
        case 1003:                    // Arena organizatori
            player->SEND_POI(7306.4f, -1505.8f, ICON_POI_TOMBSTONE, 6, 0, "Arena");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
            break;
        case 1004:                    // Battleground organizatori
            player->SEND_POI(7306.4f, -1505.8f, ICON_POI_REDTOWER, 6, 0, "Bg");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
            break;
        case 1005:                    // Training dummy
            player->SEND_POI(7320.0f, -1497.2f, ICON_POI_TOMBSTONE, 6, 0, "Dummy");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
            break;
        case 1006:                    // Bar
            player->SEND_POI(7351.2f, -1485.3f, ICON_POI_BLUEHOUSE, 6, 0, "Bar");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
			break;
        case 1007:                    // Guild Bank
            player->SEND_POI(7340.8f, -1544.2f, ICON_POI_SMALL_HOUSE, 6, 0, "GB");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
            break;
        case 1008:                    // Guild a Tabard Master
            player->SEND_POI(7342.8f, -1533.9f, ICON_POI_SMALL_HOUSE, 6, 0, "Guild");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
            break;
        case 1009:                    // Bankar
            player->SEND_POI(7336.1f, -1531.6f, ICON_POI_SMALL_HOUSE, 6, 0, "Bank");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
            break;
        case 1010:                    // Aukcia
            player->SEND_POI(7336.1f, -1531.6f, ICON_POI_SMALL_HOUSE, 6, 0, "Auction");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
            break;
        case 1011:                    // Centrum profesii
            player->SEND_POI(7474.9f, -1581.9f, ICON_POI_BLUELOGS, 6, 0, "Professions");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
            break;
        case 1012:                    // T6 token, BoJ a Sunmote vendori
            player->SEND_POI(7546.3f, -1531.6f, ICON_POI_BLUETOWER, 6, 0, "Tier items");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
            break;
        case 1013:                    // Teleporter
			player->SEND_POI(7333.1f, -1592.4f, ICON_POI_REDFLAG, 6, 0, "Teleporter");
            player->SEND_GOSSIP_MENU(500000, _Creature->GetGUID());
            break;
    }
}

bool GossipSelect_MCGuard(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    // Main menu
    if (sender == GOSSIP_SENDER_MAIN)
        SendDefaultMenu_MCGuard( player, _Creature, action );

    return true;
}

void AddSC_MCGuard()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="npc_mcguard";
    newscript->pGossipHello = &GossipHello_MCGuard;
    newscript->pGossipSelect = &GossipSelect_MCGuard;
    newscript->RegisterSelf();
}