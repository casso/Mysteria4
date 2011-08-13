/* ScriptData
SDName: npc_teleporter
SD%Complete: 100
SDComment: City/Dungeon/arena Teleporter
SDCategory: Custom
Authors: Casso & Smrtokvitek
Year: 2010
EndScriptData */

#include "precompiled.h"

bool GossipHello_Teleporter(Player *player, Creature *_Creature)
{ 
    ////////////////// MAIN MENU ///////////////////////////
    player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TALK, "Cities",             GOSSIP_SENDER_MAIN, 1000);
    if(player->getLevel() >= 50) player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TALK, "Dungeons and Raids", GOSSIP_SENDER_MAIN, 2000);
    if(player->getLevel() >= 60) player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TALK, "Arenas",             GOSSIP_SENDER_MAIN, 3000);
    player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TALK, "Leveling zones",     GOSSIP_SENDER_MAIN, 4000);
    player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TALK, "Other",              GOSSIP_SENDER_MAIN, 5000);

    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,_Creature->GetGUID());

    return true;
}

void SendDefaultMenu_Teleporter(Player *player, Creature *_Creature, uint32 action )
{
    switch(action)
    {
        ////////////////// SUB MENUS ///////////////////////////
    case 1000: // Cities
        // Alliance
        if(player->GetTeam() == ALLIANCE)
        {
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Stormwind City",  GOSSIP_SENDER_MAIN, 1001);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Ironforge",       GOSSIP_SENDER_MAIN, 1002);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Darnassus",       GOSSIP_SENDER_MAIN, 1003);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Exodar",          GOSSIP_SENDER_MAIN, 1004);
            if(player->getLevel() >= 60) player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Honor Hold",      GOSSIP_SENDER_MAIN, 1005);
        }
        // Horde
        else if(player->GetTeam() == HORDE)
        {
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Orgrimmar",       GOSSIP_SENDER_MAIN, 1006);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Thunder Bluff",   GOSSIP_SENDER_MAIN, 1007);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Undercity",       GOSSIP_SENDER_MAIN, 1008);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Silvermoon City", GOSSIP_SENDER_MAIN, 1009);
            if(player->getLevel() >= 60) player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Thrallmar",       GOSSIP_SENDER_MAIN, 1010);
        }

        // Neutral
        if(player->getLevel() >= 60) player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Shattrath City",  GOSSIP_SENDER_MAIN, 1011);
        if(player->getLevel() >= 60) player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Area 52",         GOSSIP_SENDER_MAIN, 1012);
        if(player->getLevel() >= 30) player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Gadgetzan",       GOSSIP_SENDER_MAIN, 1013);
        if(player->getLevel() >= 30) player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Booty Bay",       GOSSIP_SENDER_MAIN, 1014);
        if(player->getLevel() >= 30) player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Ratchet",         GOSSIP_SENDER_MAIN, 1016);
        if(player->getLevel() >= 40) player->ADD_GOSSIP_ITEM( GOSSIP_ICON_VENDOR, "Everlook",        GOSSIP_SENDER_MAIN, 1015);


        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TALK,   "<- Back",  GOSSIP_SENDER_MAIN, 6000);

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,_Creature->GetGUID());
        break;

    case 2000: // Dungeons And Raids
        if(player->getLevel() >= 60)
        {
            // T6+/T6
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Sunwell Plateau",    GOSSIP_SENDER_MAIN, 2001);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Black Temple",       GOSSIP_SENDER_MAIN, 2002);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Hyjal Summit",       GOSSIP_SENDER_MAIN, 2003);
            // T5
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Tempest Keep",       GOSSIP_SENDER_MAIN, 2004);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Coilfang Reservoir", GOSSIP_SENDER_MAIN, 2005);
            // T5/T4
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Gruul's Lair",       GOSSIP_SENDER_MAIN, 2006);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Zul' Aman",          GOSSIP_SENDER_MAIN, 2007);
            // T4
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Karazhan",           GOSSIP_SENDER_MAIN, 2008);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Hellfire Citadel",   GOSSIP_SENDER_MAIN, 2009);
            // D3
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Auchindoun",         GOSSIP_SENDER_MAIN, 2010);
        }
        if(player->getLevel() >= 50)
        {
            // T3
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Naxxramas",          GOSSIP_SENDER_MAIN, 2011);
            // T2/T1
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Onyxia's Lair",      GOSSIP_SENDER_MAIN, 2012);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Blackrock Mountain", GOSSIP_SENDER_MAIN, 2013);
            // Other 60lvl
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Ahn' Qiraj",         GOSSIP_SENDER_MAIN, 2014);
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_MONEY_BAG, "Zul'Gurub",          GOSSIP_SENDER_MAIN, 2015);
        }
        //player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TALK,      "<- Back",     GOSSIP_SENDER_MAIN, 6000);

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,_Creature->GetGUID());
        break;

    case 3000: // Arenas
        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_BATTLE, "Gurubashi Arena",       GOSSIP_SENDER_MAIN, 3001);
        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_BATTLE, "Nagrand Arena",         GOSSIP_SENDER_MAIN, 3002);
        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_BATTLE, "Blade's Edge Arena",    GOSSIP_SENDER_MAIN, 3003);
        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_BATTLE, "Orgrimmar Arena",       GOSSIP_SENDER_MAIN, 3004);
        //player->ADD_GOSSIP_ITEM( GOSSIP_ICON_BATTLE, "Dire Maul Arena",       GOSSIP_SENDER_MAIN, 3005);

        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TALK,   "<- Back",               GOSSIP_SENDER_MAIN, 6000);

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,_Creature->GetGUID());
        break;

    case 4000: // Leveling
        // Alliance
        if(player->GetTeam() == ALLIANCE)
        {
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TRAINER, "1-50 level",  GOSSIP_SENDER_MAIN, 4001);
            if(player->getLevel() >= 45) player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TRAINER, "50-70 level", GOSSIP_SENDER_MAIN, 4002);
        }
        // Horde
        else if(player->GetTeam() == HORDE)
        {
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TRAINER, "1-50 level",  GOSSIP_SENDER_MAIN, 4003);
            if(player->getLevel() >= 45) player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TRAINER, "50-70 level", GOSSIP_SENDER_MAIN, 4004);
        }

        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TALK,     "<- Back",        GOSSIP_SENDER_MAIN, 6000);

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,_Creature->GetGUID());
        break;

    case 5000: // Other
        if(player->getLevel() >= 60)
        {
            // Horde
            if(player->GetTeam() == HORDE)
                player->ADD_GOSSIP_ITEM( GOSSIP_ICON_BATTLE, "Attack on Stormwind", GOSSIP_SENDER_MAIN, 5001);

            // Alliance
            if(player->GetTeam() == ALLIANCE)
                player->ADD_GOSSIP_ITEM( GOSSIP_ICON_BATTLE, "Attack on Orgrimmar", GOSSIP_SENDER_MAIN, 5002);

            // Neutral
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TAXI,    "Dark Portal",         GOSSIP_SENDER_MAIN, 5003);
        }
        // player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TAXI,    "Mysteria Centrum",    GOSSIP_SENDER_MAIN, 5004);

        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TABARD, "Guild House",      GOSSIP_SENDER_MAIN, 5005);

        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TALK,    "<- Back",             GOSSIP_SENDER_MAIN, 6000);

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,_Creature->GetGUID());
        break;

    case 6000: //<- Back
        GossipHello_Teleporter(player,_Creature);
        break;


        ////////////////// TELEPORTATIONS ///////////////////////////

        // Cities

    case 1001: // Stormwind City
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == ALLIANCE)
            player->TeleportTo(0, -8832.59f, 621.7f, 93.59f, 3.944f);
        break;

    case 1002: // Ironforge
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == ALLIANCE)
            player->TeleportTo(0, -4981.25f, -881.542f, 501.66f, 5.416f);
        break;    

    case 1003: // Darnassus
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == ALLIANCE)
            player->TeleportTo(1, 9948.55f, 2413.59f, 1327.23f, 1.619f);
        break;    

    case 1004: // Exodar
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == ALLIANCE)
            player->TeleportTo(530, -3902.58f, -11748.74f, -107.72f, 5.221f);
        break;    

    case 1005: // Honor Hold
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == ALLIANCE)
            player->TeleportTo(530, -705.713f, 2681.38f, 93.887f, 6.068f);
        break;

    case 1006: // Orgrimmar
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == HORDE)
            player->TeleportTo(1, 1586.07f, -4401.07f, 5.15365f, 0.570f);
        break;

    case 1007: // Thunder Bluff
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == HORDE)
            player->TeleportTo(1, -1273.01f, 122.61f, 131.17f, 5.137f);
        break;

    case 1008: // Undercity
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == HORDE)
            player->TeleportTo(0, 1568.29f, 241.27f, -44.30f, 0.00f);
        break;

    case 1009: // Silvermoon City
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == HORDE)
            player->TeleportTo(530, 9912.12f, -7149.29f, 45.607f,    4.010f);
        break;

    case 1010: // Thrallmar
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == HORDE)
            player->TeleportTo(530, 139.167f, 2672.55f, 85.441f, 3.545f);
        break;

    case 1011: // Shattrath City
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(530, -1886.71f, 5422.01f, -12.42f, 0.327f);
        break;

    case 1012: // Area 52
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(530, 3049.57f, 3686.92f, 142.34f, 0.522f);
        break;    

    case 1013: // Gadgetzan
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(1, -7170.16f, -3785.36f, 8.3698f, 0.000f);
        break;

    case 1014: // Booty Bay
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(0, -14293.5f, 533.7f, 8.79f, 4.294f);
        break;

    case 1015: // Everlook
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(1, 6727.00f, -4643.06f, 721.40f, 4.76f);
        break;

    case 1016: // Ratchet
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(1, -956.66f, -3754.70f, 5.34f, 0.99f);
        break;


        // Dungeons and Raids

    case 2001: // Sunwell Plateau
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(530, 12574.6f, -6774.81f, 15.09f, 3.120f);
        break;

    case 2002: // Black Temple
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(530, -3640.55f, 315.48f, 35.1f, 3.060f);
        break;

    case 2003: // Hyjal Summit
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(1, -8177.18f, -4179.92f, -167.18f, 1.050f);
        break;

    case 2004: // Tempest Keep
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(530, 3099.65f, 1526.38f, 190.3f, 4.660f);
        break;

    case 2005: // Coilfang Reservoir
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(530, 787.28f, 6867.18f, -65.35f, 0.094f);
        break;

    case 2006: // Gruul's Lair
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(530, 3527.78f, 5155.82f, -1.3f, 4.720f);
        break;

    case 2007: // Zul' Aman
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(530, 6851.29f, -7952.01f, 170.03f, 4.820f);
        break;

    case 2008: // Karazhan
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(0, -11127.9f, -2018.67f, 47.3f, 0.688f);
        break;

    case 2009: // Hellfire Citadel
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(530, -268.3f, 3109.79f, 31.66f, 2.162f);
        break;

    case 2010: // Auchindoun
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(530, -3325.73f, 4947.9f, -101.395f, 4.414f);
        break;

    case 2011: // Naxxramas
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(0, 3108.88f, -3717.06f, 133.6f, 5.730f);
        break;        

    case 2012: // Onyxia's Lair
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(1, -4708.14f, -3727.58f, 54.57f, 3.811f);
        break;

    case 2013: // Blackrock Mountain
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(0, -7493.71f, -1115.18f, 265.86f, 3.200f);
        break;

    case 2014: // Ahn' Qiraj
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(1, -8414.69f, 1503.07f, 30.54f, 2.754f);
        break;

    case 2015: // Zul' Gurub
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(0, -11916.2f, -1205.27f, 92.28f, 4.732f);
        break;


        // Arenas

    case 3001: // Gurubashi arena (Stranglethorn vale)
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(0, -13226.1f, 233.839f, 33.359f, 1.139f);
        break;

    case 3002: // Ring of Trials (Nagrand)
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(530, -2039.32f, 6658.62f, 13.05f, 5.273f);
        break;

    case 3003: // Circle of blood (Bladesedge mountains)
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(530, 2838.74f, 5928.16f, 11.06f, 2.419f);
        break;

    case 3004: // Ring of Valor (Orgrimmar)
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(1, 2179.908f, -4767.31f, 54.9f, 2.62f);
        break;


        // Leveling areas

    case 4001: // 1-50 alliance
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == ALLIANCE)
            player->TeleportTo(0, -4137.6f, -1058.59f, 178.638f, 4.791f);
        break;

    case 4002: // 50-70 alliance
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == ALLIANCE)
            player->TeleportTo(0, -3916.91f, -1457.46f, 147.31f, 5.150f);
        break;

    case 4003: // 1-50 horde
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == HORDE)
            player->TeleportTo(1, 5295.17f, -1937.22f, 1331.34f, 5.314f);
        break;

    case 4004: // 50-70 horde
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == HORDE)
            player->TeleportTo(1, 4584.91f, -2032.06f, 1148.02f, 1.438f);
        break;


        // Other

    case 5001: // Attack to Stormwind City
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == HORDE)
            player->TeleportTo(0, -9553.17f, -1372.71f, 51.29f, 0.0f);
        break;

    case 5002: // Attack to Orgrimmar
        player->CLOSE_GOSSIP_MENU();
        if(player->GetTeam() == ALLIANCE)
            player->TeleportTo(1, 2432.84f, -2953.04f, 123.75f, 3.263f);
        break;

    case 5003: // Dark Portal
        player->CLOSE_GOSSIP_MENU();
        player->TeleportTo(0, -11888.9f, -3206.47f, -15.12f, 3.323f);
        break;

    // case 5004: // Mysteria Centrum
    //     player->CLOSE_GOSSIP_MENU();
    //     player->TeleportTo(1, 7333.12f, -1592.5f, 164.542f, 1.521f);
    //     break;

    case 5005: // GuildHouse
        if(player->TeleportToGuildHouse())
            player->CLOSE_GOSSIP_MENU();
        else
            player->SEND_GOSSIP_MENU(500001, _Creature->GetGUID());
        break;


    default: // Something wrong?!
        player->CLOSE_GOSSIP_MENU();

    }
}

bool GossipSelect_Teleporter(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    // Main menu
    if (sender == GOSSIP_SENDER_MAIN)
        SendDefaultMenu_Teleporter( player, _Creature, action );

    return true;
}

void AddSC_Teleporter()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="npc_teleporter";
    newscript->pGossipHello = &GossipHello_Teleporter;
    newscript->pGossipSelect = &GossipSelect_Teleporter;
    newscript->RegisterSelf();
}
