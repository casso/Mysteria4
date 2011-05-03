/* ScriptData
SDName: npc_mysteria_changer
SD%Complete: 100
SDComment: PVP token changer
SDCategory: Custom
Author: Casso
Year: 2010
EndScriptData */

#include "precompiled.h"

#define ITEM_ENTRY_HONOR_MARK 10662
#define HONOR_PRICE 1500

bool GossipHello_TokenChanger(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu(_Creature->GetGUID());

    player->ADD_GOSSIP_ITEM( GOSSIP_ICON_TALK, "Vymen 1500 honoru za 1 honor marku", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);  
    player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,_Creature->GetGUID());

    return true;
}

bool GossipSelect_TokenChanger(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
        {
            if(player->GetHonorPoints() < HONOR_PRICE )
                player->SEND_GOSSIP_MENU(500002, _Creature->GetGUID());
            else
            {
                if (Item* pItem = player->StoreNewItemInInventorySlot(ITEM_ENTRY_HONOR_MARK, 1))
                {
                    player->ModifyHonorPoints(-HONOR_PRICE);
                    player->SEND_GOSSIP_MENU(500004, _Creature->GetGUID());
                    player->SendNewItem(pItem, 1, true, false);
                }
                else
                    player->SEND_GOSSIP_MENU(500003, _Creature->GetGUID());

                return true;
            }
            break;
        }
    }

    return false;
}

void AddSC_TokenChanger()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="npc_mysteria_changer";
    newscript->pGossipHello = &GossipHello_TokenChanger;
    newscript->pGossipSelect = &GossipSelect_TokenChanger;
    newscript->RegisterSelf();
}
