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

#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "UpdateMask.h"
#include "AuctionHouseMgr.h"
#include "Mail.h"
#include "Util.h"
#include "Chat.h"

// please DO NOT use iterator++, because it is slower than ++iterator!!!
// post-incrementation is always slower than pre-incrementation !

// void called when player click on auctioneer npc
void WorldSession::HandleAuctionHelloOpcode( WorldPacket & recv_data )
{
    ObjectGuid auctioneerGuid;                              // NPC guid
    recv_data >> auctioneerGuid;

    Creature *unit = GetPlayer()->GetNPCIfCanInteractWith(auctioneerGuid, UNIT_NPC_FLAG_AUCTIONEER);
    if (!unit)
    {
        DEBUG_LOG("WORLD: HandleAuctionHelloOpcode - %s not found or you can't interact with him.", auctioneerGuid.GetString().c_str());
        return;
    }

    // remove fake death
    if(GetPlayer()->hasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    SendAuctionHello(unit);
}

// this void causes that auction window is opened
void WorldSession::SendAuctionHello(Unit* unit)
{
    // always return pointer
    AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntry(unit);

    WorldPacket data( MSG_AUCTION_HELLO, 12 );
    data << unit->GetObjectGuid();
    data << uint32(ahEntry->houseId);
    SendPacket(&data);
}

// call this method when player bids, creates, or deletes auction
void WorldSession::SendAuctionCommandResult(uint32 auctionId, uint32 Action, uint32 ErrorCode, uint32 bidError )
{
    WorldPacket data( SMSG_AUCTION_COMMAND_RESULT, 16 );
    data << uint32(auctionId);
    data << uint32(Action);
    data << uint32(ErrorCode);
    if ( !ErrorCode && Action )
        data << uint32(bidError);                           // when bid, then send 0, once...
    SendPacket(&data);
}

// this function sends notification, if bidder is online
void WorldSession::SendAuctionBidderNotification( uint32 location, uint32 auctionId, ObjectGuid bidderGuid, uint32 bidSum, uint32 diff, uint32 item_template)
{
    WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, (8*4));
    data << uint32(location);
    data << uint32(auctionId);
    data << bidderGuid;
    data << uint32(bidSum);
    data << uint32(diff);
    data << uint32(item_template);
    data << uint32(0);
    SendPacket(&data);
}

// this void causes on client to display: "Your auction sold"
void WorldSession::SendAuctionOwnerNotification( AuctionEntry* auction)
{
    WorldPacket data(SMSG_AUCTION_OWNER_NOTIFICATION, (7*4));
    data << uint32(auction->Id);
    data << uint32(auction->bid);
    data << uint32(0);                                      // unk
    data << uint32(0);                                      // unk
    data << uint32(0);                                      // unk
    data << uint32(auction->item_template);
    data << uint32(0);                                      // unk
    SendPacket(&data);
}

// this function sends mail to old bidder
void WorldSession::SendAuctionOutbiddedMail(AuctionEntry *auction, uint32 newPrice)
{
    ObjectGuid oldBidder_guid = ObjectGuid(HIGHGUID_PLAYER, auction->bidder);
    Player *oldBidder = sObjectMgr.GetPlayer(oldBidder_guid);

    uint32 oldBidder_accId = 0;
    if(!oldBidder)
        oldBidder_accId = sObjectMgr.GetPlayerAccountIdByGUID(oldBidder_guid);

    // old bidder exist
    if(oldBidder || oldBidder_accId)
    {
        std::ostringstream msgAuctionOutbiddedSubject;
        msgAuctionOutbiddedSubject << auction->item_template << ":0:" << AUCTION_OUTBIDDED;

        if (oldBidder)
            oldBidder->GetSession()->SendAuctionBidderNotification(
                auction->GetHouseId(), auction->Id, _player->GetObjectGuid(),
                newPrice, auction->GetAuctionOutBid(), auction->item_template);

        MailDraft(msgAuctionOutbiddedSubject.str())
            .SetMoney(auction->bid)
            .SendMailTo(MailReceiver(oldBidder, oldBidder_guid), auction, MAIL_CHECK_MASK_COPIED);
    }
}

// this function sends mail, when auction is cancelled to old bidder
void WorldSession::SendAuctionCancelledToBidderMail( AuctionEntry* auction )
{
    ObjectGuid bidder_guid = ObjectGuid(HIGHGUID_PLAYER, auction->bidder);
    Player *bidder = sObjectMgr.GetPlayer(bidder_guid);

    uint32 bidder_accId = 0;
    if(!bidder)
        bidder_accId = sObjectMgr.GetPlayerAccountIdByGUID(bidder_guid);

    // bidder exist
    if(bidder || bidder_accId)
    {
        std::ostringstream msgAuctionCancelledSubject;
        msgAuctionCancelledSubject << auction->item_template << ":0:" << AUCTION_CANCELLED_TO_BIDDER;

        MailDraft(msgAuctionCancelledSubject.str())
            .SetMoney(auction->bid)
            .SendMailTo(MailReceiver(bidder, bidder_guid), auction, MAIL_CHECK_MASK_COPIED);
    }
}

AuctionHouseEntry const* WorldSession::GetCheckedAuctionHouseForAuctioneer(ObjectGuid guid)
{
    Unit* auctioneer = NULL;

    // GM case
    if (guid == GetPlayer()->GetObjectGuid())
    {
        // command case will return only if player have real access to command
        // using special access modes (1,-1) done at mode set in command, so not need recheck
        if (GetPlayer()->GetAuctionAccessMode()==0 && !ChatHandler(GetPlayer()).FindCommand("auction"))
        {
            DEBUG_LOG("%s attempt open auction in cheating way.", guid.GetString().c_str());
            return NULL;
        }

        auctioneer = GetPlayer();
    }
    // auctioneer case
    else
    {
        auctioneer = GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER);
        if (!auctioneer)
        {
            DEBUG_LOG("Auctioneeer %s accessed in cheating way.", guid.GetString().c_str());
            return NULL;
        }
    }

    // always return pointer
    return AuctionHouseMgr::GetAuctionHouseEntry(auctioneer);
}

// this void creates new auction and adds auction to some auctionhouse
void WorldSession::HandleAuctionSellItem( WorldPacket & recv_data )
{
    DEBUG_LOG("WORLD: HandleAuctionSellItem");

    ObjectGuid auctioneerGuid;
    uint64 item;
    uint32 etime, bid, buyout;
    recv_data >> auctioneerGuid;
    recv_data >> item;
    recv_data >> bid;
    recv_data >> buyout;
    recv_data >> etime;

    if (!item || !bid || !etime)
        return;                                             // check for cheaters

    Player *pl = GetPlayer();

    AuctionHouseEntry const* auctionHouseEntry = GetCheckedAuctionHouseForAuctioneer(auctioneerGuid);
    if (!auctionHouseEntry)
        return;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // client send time in minutes, convert to common used sec time
    etime *= MINUTE;

    // client understand only 3 auction time
    switch(etime)
    {
        case 1*MIN_AUCTION_TIME:
        case 2*MIN_AUCTION_TIME:
        case 4*MIN_AUCTION_TIME:
            break;
        default:
            return;
    }

    // remove fake death
    if(GetPlayer()->hasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    Item *it = pl->GetItemByGuid( item );
    // do not allow to sell already auctioned items
    if(sAuctionMgr.GetAItem(GUID_LOPART(item)))
    {
        sLog.outError("AuctionError, player %s is sending item id: %u, but item is already in another auction", pl->GetName(), GUID_LOPART(item));
        SendAuctionCommandResult(0, AUCTION_SELL_ITEM, AUCTION_INTERNAL_ERROR);
        return;
    }
    // prevent sending bag with items (cheat: can be placed in bag after adding equipped empty bag to auction)
    if(!it)
    {
        SendAuctionCommandResult(0, AUCTION_SELL_ITEM, AUCTION_ITEM_NOT_FOUND);
        return;
    }

    if(!it->CanBeTraded())
    {
        SendAuctionCommandResult(0, AUCTION_SELL_ITEM, AUCTION_INTERNAL_ERROR);
        return;
    }

    if ((it->GetProto()->Flags & ITEM_FLAG_CONJURED) || it->GetUInt32Value(ITEM_FIELD_DURATION))
    {
        SendAuctionCommandResult(0, AUCTION_SELL_ITEM, AUCTION_INTERNAL_ERROR);
        return;
    }

    //we have to take deposit :
    uint32 deposit = AuctionHouseMgr::GetAuctionDeposit( auctionHouseEntry, etime, it );
    if ( pl->GetMoney() < deposit )
    {
        SendAuctionCommandResult(0, AUCTION_SELL_ITEM, AUCTION_NOT_ENOUGHT_MONEY);
        return;
    }

    if( GetSecurity() > SECURITY_VIP && sWorld.getConfig(CONFIG_BOOL_GM_LOG_TRADE) )
    {
        sLog.outCommand(GetAccountId(),"GM %s (Account: %u) create auction: %s (Entry: %u Count: %u)",
            GetPlayerName(), GetAccountId(), it->GetProto()->Name1, it->GetEntry(), it->GetCount());
    }

    pl->ModifyMoney( -int32(deposit) );

    uint32 auction_time = uint32(etime * sWorld.getConfig(CONFIG_FLOAT_RATE_AUCTION_TIME));

    AuctionEntry *AH = new AuctionEntry;
    AH->Id = sObjectMgr.GenerateAuctionID();
    AH->item_guidlow = GUID_LOPART(item);
    AH->item_template = it->GetEntry();
    AH->owner = pl->GetGUIDLow();
    AH->startbid = bid;
    AH->bidder = 0;
    AH->bid = 0;
    AH->buyout = buyout;
    AH->expire_time = time(NULL) + auction_time;
    AH->deposit = deposit;
    AH->auctionHouseEntry = auctionHouseEntry;

    DETAIL_LOG("selling item %u to auctioneer %s with initial bid %u with buyout %u and with time %u (in sec) in auctionhouse %u",
        GUID_LOPART(item), auctioneerGuid.GetString().c_str(), bid, buyout, auction_time, AH->GetHouseId());
    auctionHouse->AddAuction(AH);

    sAuctionMgr.AddAItem(it);
    pl->MoveItemFromInventory( it->GetBagSlot(), it->GetSlot(), true);

    CharacterDatabase.BeginTransaction();
    it->DeleteFromInventoryDB();
    it->SaveToDB();                                         // recursive and not have transaction guard into self, not in inventiory and can be save standalone
    AH->SaveToDB();
    pl->SaveInventoryAndGoldToDB();
    CharacterDatabase.CommitTransaction();

    SendAuctionCommandResult(AH->Id, AUCTION_SELL_ITEM, AUCTION_OK);
}

// this function is called when client bids or buys out auction
void WorldSession::HandleAuctionPlaceBid( WorldPacket & recv_data )
{
    DEBUG_LOG("WORLD: HandleAuctionPlaceBid");

    ObjectGuid auctioneerGuid;
    uint32 auctionId;
    uint32 price;
    recv_data >> auctioneerGuid;
    recv_data >> auctionId >> price;

    if (!auctionId || !price)
        return;                                             // check for cheaters

    AuctionHouseEntry const* auctionHouseEntry = GetCheckedAuctionHouseForAuctioneer(auctioneerGuid);
    if (!auctionHouseEntry)
        return;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // remove fake death
    if(GetPlayer()->hasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);


    AuctionEntry *auction = auctionHouse->GetAuction(auctionId);
    Player *pl = GetPlayer();

    if( !auction || auction->owner == pl->GetGUIDLow() )
    {
        // you cannot bid your own auction:
        SendAuctionCommandResult( 0, AUCTION_PLACE_BID, CANNOT_BID_YOUR_AUCTION_ERROR );
        return;
    }

    ObjectGuid ownerGuid = ObjectGuid(HIGHGUID_PLAYER, auction->owner);

    // impossible have online own another character (use this for speedup check in case online owner)
    Player* auction_owner = sObjectMgr.GetPlayer(ownerGuid);
    if (!auction_owner && sObjectMgr.GetPlayerAccountIdByGUID(ownerGuid) == pl->GetSession()->GetAccountId())
    {
        // you cannot bid your another character auction:
        SendAuctionCommandResult( 0, AUCTION_PLACE_BID, CANNOT_BID_YOUR_AUCTION_ERROR );
        return;
    }

    // cheating
    if(price <= auction->bid || price < auction->startbid)
        return;

    // price too low for next bid if not buyout
    if ((price < auction->buyout || auction->buyout == 0) &&
        price < auction->bid + auction->GetAuctionOutBid())
    {
        // auction has already higher bid, client tests it!
        return;
    }

    if (price > pl->GetMoney())
    {
        // you don't have enough money!, client tests!
        // SendAuctionCommandResult(auction->auctionId, AUCTION_PLACE_BID, ???);
        return;
    }

    if ((price < auction->buyout) || (auction->buyout == 0))
    {
        if (auction->bidder > 0)
        {
            if ( auction->bidder == pl->GetGUIDLow() )
            {
                pl->ModifyMoney( -int32(price - auction->bid));
            }
            else
            {
                // mail to last bidder and return money
                SendAuctionOutbiddedMail( auction , price );
                pl->ModifyMoney( -int32(price) );
            }
        }
        else
        {
            pl->ModifyMoney( -int32(price) );
        }
        auction->bidder = pl->GetGUIDLow();
        auction->bid = price;

        // after this update we should save player's money ...
        CharacterDatabase.PExecute("UPDATE auction SET buyguid = '%u',lastbid = '%u' WHERE id = '%u'", auction->bidder, auction->bid, auction->Id);

        SendAuctionCommandResult(auction->Id, AUCTION_PLACE_BID, AUCTION_OK, 0 );
    }
    else
    {
        // buyout:
        if (pl->GetGUIDLow() == auction->bidder )
        {
            pl->ModifyMoney(-int32(auction->buyout - auction->bid));
        }
        else
        {
            pl->ModifyMoney(-int32(auction->buyout));
            if ( auction->bidder )                          // buyout for bidded auction ..
            {
                SendAuctionOutbiddedMail( auction, auction->buyout );
            }
        }
        auction->bidder = pl->GetGUIDLow();
        auction->bid = auction->buyout;

        sAuctionMgr.SendAuctionSalePendingMail( auction );
        sAuctionMgr.SendAuctionSuccessfulMail( auction );
        sAuctionMgr.SendAuctionWonMail( auction );

        SendAuctionCommandResult(auction->Id, AUCTION_PLACE_BID, AUCTION_OK);

        sAuctionMgr.RemoveAItem(auction->item_guidlow);
        auctionHouse->RemoveAuction(auction->Id);
        auction->DeleteFromDB();

        delete auction;
    }
    CharacterDatabase.BeginTransaction();
    pl->SaveInventoryAndGoldToDB();
    CharacterDatabase.CommitTransaction();
}

// this void is called when auction_owner cancels his auction
void WorldSession::HandleAuctionRemoveItem( WorldPacket & recv_data )
{
    DEBUG_LOG("WORLD: HandleAuctionRemoveItem");

    ObjectGuid auctioneerGuid;
    uint32 auctionId;
    recv_data >> auctioneerGuid;
    recv_data >> auctionId;
    //DEBUG_LOG( "Cancel AUCTION AuctionID: %u", auctionId);

    AuctionHouseEntry const* auctionHouseEntry = GetCheckedAuctionHouseForAuctioneer(auctioneerGuid);
    if (!auctionHouseEntry)
        return;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // remove fake death
    if(GetPlayer()->hasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    AuctionEntry *auction = auctionHouse->GetAuction(auctionId);
    Player *pl = GetPlayer();

    if (auction && auction->owner == pl->GetGUIDLow())
    {
        Item *pItem = sAuctionMgr.GetAItem(auction->item_guidlow);
        if (pItem)
        {
            if (auction->bidder > 0)                        // If we have a bidder, we have to send him the money he paid
            {
                uint32 auctionCut = auction->GetAuctionCut();
                if ( pl->GetMoney() < auctionCut )          //player doesn't have enough money, maybe message needed
                    return;
                //some auctionBidderNotification would be needed, but don't know that parts..
                SendAuctionCancelledToBidderMail( auction );
                pl->ModifyMoney( -int32(auctionCut) );
            }
            // Return the item by mail
            std::ostringstream msgAuctionCanceledOwner;
            msgAuctionCanceledOwner << auction->item_template << ":0:" << AUCTION_CANCELED;

            // item will deleted or added to received mail list
            MailDraft(msgAuctionCanceledOwner.str())
                .AddItem(pItem)
                .SendMailTo(pl, auction, MAIL_CHECK_MASK_COPIED);
        }
        else
        {
            sLog.outError("Auction id: %u has nonexistent item (item guid : %u)!!!", auction->Id, auction->item_guidlow);
            SendAuctionCommandResult( 0, AUCTION_CANCEL, AUCTION_INTERNAL_ERROR );
            return;
        }
    }
    else
    {
        SendAuctionCommandResult( 0, AUCTION_CANCEL, AUCTION_INTERNAL_ERROR );
        //this code isn't possible ... maybe there should be ASSERT
        sLog.outError("CHEATER : %u, he tried to cancel auction (id: %u) of another player, or auction is NULL", pl->GetGUIDLow(), auctionId );
        return;
    }

    //inform player, that auction is removed
    SendAuctionCommandResult( auction->Id, AUCTION_CANCEL, AUCTION_OK );
    // Now remove the auction
    CharacterDatabase.BeginTransaction();
    auction->DeleteFromDB();
    pl->SaveInventoryAndGoldToDB();
    CharacterDatabase.CommitTransaction();
    sAuctionMgr.RemoveAItem( auction->item_guidlow );
    auctionHouse->RemoveAuction( auction->Id );
    delete auction;
}

//called when player lists his bids
void WorldSession::HandleAuctionListBidderItems( WorldPacket & recv_data )
{
    DEBUG_LOG("WORLD: HandleAuctionListBidderItems");

    ObjectGuid auctioneerGuid;                              // NPC guid
    uint32 listfrom;                                        // page of auctions
    uint32 outbiddedCount;                                  // count of outbidded auctions

    recv_data >> auctioneerGuid;
    recv_data >> listfrom;                                  // not used in fact (this list not have page control in client)
    recv_data >> outbiddedCount;
    if (recv_data.size() != (16 + outbiddedCount * 4 ))
    {
        sLog.outError("Client sent bad opcode!!! with count: %u and size : %lu (must be: %u)", outbiddedCount, (unsigned long)recv_data.size(),(16 + outbiddedCount * 4 ));
        outbiddedCount = 0;
    }

    AuctionHouseEntry const* auctionHouseEntry = GetCheckedAuctionHouseForAuctioneer(auctioneerGuid);
    if (!auctionHouseEntry)
        return;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // remove fake death
    if(GetPlayer()->hasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    WorldPacket data( SMSG_AUCTION_BIDDER_LIST_RESULT, (4+4+4) );
    Player *pl = GetPlayer();
    data << uint32(0);                                      // add 0 as count
    uint32 count = 0;
    uint32 totalcount = 0;
    while ( outbiddedCount > 0)                             // add all data, which client requires
    {
        --outbiddedCount;
        uint32 outbiddedAuctionId;
        recv_data >> outbiddedAuctionId;
        AuctionEntry * auction = auctionHouse->GetAuction( outbiddedAuctionId );
        if ( auction && auction->BuildAuctionInfo(data))
        {
            ++totalcount;
            ++count;
        }
    }

    auctionHouse->BuildListBidderItems(data, pl, count, totalcount);
    data.put<uint32>( 0, count );                           // add count to placeholder
    data << uint32(totalcount);
    data << uint32(300);                                    // unk 2.3.0 delay for next list request?
    SendPacket(&data);
}

// this void sends player info about his auctions
void WorldSession::HandleAuctionListOwnerItems( WorldPacket & recv_data )
{
    DEBUG_LOG("WORLD: HandleAuctionListOwnerItems");

    ObjectGuid auctioneerGuid;
    uint32 listfrom;

    recv_data >> auctioneerGuid;
    recv_data >> listfrom;                                  // not used in fact (this list not have page control in client)

    AuctionHouseEntry const* auctionHouseEntry = GetCheckedAuctionHouseForAuctioneer(auctioneerGuid);
    if (!auctionHouseEntry)
        return;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // remove fake death
    if(GetPlayer()->hasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    WorldPacket data( SMSG_AUCTION_OWNER_LIST_RESULT, (4+4+4) );
    data << (uint32) 0;                                     // amount place holder

    uint32 count = 0;
    uint32 totalcount = 0;

    auctionHouse->BuildListOwnerItems(data, _player, count, totalcount);
    data.put<uint32>(0, count);
    data << uint32(totalcount);
    data << uint32(0);                                      // 2.3.0 delay for next list request?
    SendPacket(&data);
}

//this void is called when player clicks on search button
void WorldSession::HandleAuctionListItems( WorldPacket & recv_data )
{
    DEBUG_LOG("WORLD: HandleAuctionListItems");

    ObjectGuid auctioneerGuid;
    std::string searchedname;
    uint8 levelmin, levelmax, usable;
    uint32 listfrom, auctionSlotID, auctionMainCategory, auctionSubCategory, quality;

    recv_data >> auctioneerGuid;
    recv_data >> listfrom;                                  // start, used for page control listing by 50 elements
    recv_data >> searchedname;

    recv_data >> levelmin >> levelmax;
    recv_data >> auctionSlotID >> auctionMainCategory >> auctionSubCategory;
    recv_data >> quality >> usable;

    recv_data.read_skip(16);                                // unknown 16 bytes: 00 07 01 00 00 01 05 00 06 00 09 01 08 00 03 00

    AuctionHouseEntry const* auctionHouseEntry = GetCheckedAuctionHouseForAuctioneer(auctioneerGuid);
    if (!auctionHouseEntry)
        return;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    // remove fake death
    if(GetPlayer()->hasUnitState(UNIT_STAT_DIED))
        GetPlayer()->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

    //DEBUG_LOG("Auctionhouse search %s list from: %u, searchedname: %s, levelmin: %u, levelmax: %u, auctionSlotID: %u, auctionMainCategory: %u, auctionSubCategory: %u, quality: %u, usable: %u",
    //  auctioneerGuid.GetString().c_str(), listfrom, searchedname.c_str(), levelmin, levelmax, auctionSlotID, auctionMainCategory, auctionSubCategory, quality, usable);

    WorldPacket data( SMSG_AUCTION_LIST_RESULT, (4+4+4) );
    uint32 count = 0;
    uint32 totalcount = 0;
    data << uint32(0);

    // converting string that we try to find to lower case
    std::wstring wsearchedname;
    if(!Utf8toWStr(searchedname,wsearchedname))
        return;

    wstrToLower(wsearchedname);

    auctionHouse->BuildListAuctionItems(data, _player,
        wsearchedname, listfrom, levelmin, levelmax, usable,
        auctionSlotID, auctionMainCategory, auctionSubCategory, quality,
        count, totalcount);

    data.put<uint32>(0, count);
    data << uint32(totalcount);
    data << uint32(300);                                    // 2.3.0 delay for next list request?
    SendPacket(&data);
}
