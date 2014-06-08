// Copyright (C) 2004 Team Python
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "QuestHandler.h"
#include "GameClient.h"
#include "Opcodes.h"
#include "Log.h"
#include "WorldServer.h"
#include "Character.h"
#include "Quest.h"
#include "UpdateMask.h"


#define world WorldServer::getSingleton()

QuestHandler::QuestHandler()
{

}

QuestHandler::~QuestHandler()
{
   for( QuestMap::iterator i = mQuests.begin( ); i != mQuests.end( ); ++ i ) {
        delete i->second;
    }
    mQuests.clear( );

}


void QuestHandler::HandleMsg( wowWData & recv_data, GameClient *pClient )
{
    wowWData data;
    char f[256];
    sprintf(f, "WORLD: Quest Opcode 0x%.4X", recv_data.opcode);
    Log::getSingleton( ).outString( f );
    switch (recv_data.opcode)
    {
        case CMSG_QUESTGIVER_STATUS_QUERY:
            {
                Log::getSingleton( ).outString( "WORLD: Recieved CMSG_QUESTGIVER_STATUS_QUERY" );
                uint32 guid1, guid2;
                recv_data >> guid1 >> guid2;
                
                WPAssert( guid1 != 0 );
                //uint32 quest_status = world.mCreatures[guid1]->getQuestStatus(pClient->getCurrentChar());
				uint32 quest_status;
				Unit *tmpUnit;
				tmpUnit = world.GetValidCreature(guid1);
				if (!tmpUnit)
					return;
				quest_status = tmpUnit->getQuestStatus(pClient->getCurrentChar());
                data.Initialise( 12, SMSG_QUESTGIVER_STATUS );
                data << guid1 << guid2 << quest_status;
                pClient->SendMsg( &data );
                Log::getSingleton( ).outString( "WORLD: Sent SMSG_QUESTGIVER_STATUS" );
            }break;
        case CMSG_QUESTGIVER_HELLO:
            {
                Log::getSingleton( ).outString( "WORLD: Recieved CMSG_QUESTGIVER_HELLO" );
                uint32 guid1, guid2;
                recv_data >> guid1 >> guid2;

                WPAssert( guid1 != 0 );
                uint32 qg_status = world.mCreatures[guid1]->getQuestStatus(pClient->getCurrentChar());
                uint32 quest_id = world.mCreatures[guid1]->getCurrentQuest(pClient->getCurrentChar());
                
                if (qg_status == 0) break;
                Quest *pQuest = world.getQuest(quest_id);

                if(qg_status == QUEST_STATUS_INCOMPLETE)
                {
                    if (pClient->getCurrentChar()->checkQuestStatus(quest_id) || pQuest->m_targetGuid == guid1)
                    {
                        char *title = world.mCreatures[guid1]->getQuestTitle(quest_id);
                        char *details = world.mCreatures[guid1]->getQuestCompleteText(quest_id);
                        
                        uint16 length = 8+4+strlen(title)+1 + strlen(details)+1 + 32;
                        length += pQuest->m_choiceRewards*12;
                        length += pQuest->m_itemRewards*12;
                        data.Initialise( length, SMSG_QUESTGIVER_OFFER_REWARD );
                        data << guid1 << guid2 << quest_id;
                        data.writeData( title, strlen(title)+1 );
                        data.writeData( details, strlen(details)+1 );
                    
                        data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                        data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                        data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                        data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);

                        data << uint32(pQuest->m_choiceRewards);
                        for (uint16 i=0; i < pQuest->m_choiceRewards; i++){
                            data << uint32(pQuest->m_choiceItemId[i]) << uint32(pQuest->m_choiceItemCount[i]);
                            data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                        }
                    
                        data << uint32(pQuest->m_itemRewards);
                        for (uint16 i=0; i < pQuest->m_itemRewards; i++){
                            data << uint32(pQuest->m_rewardItemId[i]) << uint32(pQuest->m_rewardItemCount[i]);
                            data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                        }

                        data << uint32(pQuest->m_rewardGold);
                        data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);

                        pClient->SendMsg( &data );
                        Log::getSingleton( ).outString( "WORLD: Sent SMSG_QUESTGIVER_OFFER_REWARD" );
                    } else {
                        // Incomplete Quest
                        char *title = world.mCreatures[guid1]->getQuestTitle(quest_id);
                        char *incompleteText = world.mCreatures[guid1]->getQuestIncompleteText(quest_id);

                        uint16 length = 8 + 4 + strlen(title)+1 + strlen(incompleteText)+1 + 28;

                        data.Initialise( length, SMSG_QUESTGIVER_REQUEST_ITEMS);
                        data << guid1 << guid2 << quest_id;
                        data.writeData( title, strlen(title)+1 );
                        data.writeData( incompleteText, strlen(incompleteText)+1 );

                        data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                        data << uint8(0x06) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                        data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                        data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                        data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00); // setting this to anything...
                        data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00); // with this set to anything, enables "continue" to comlete quest
                        data << uint8(0x01) << uint8(0x00) << uint8(0x00) << uint8(0x00);

                        pClient->SendMsg( &data );
 //                       pClient->getCurrentChar()->setQuestStatus(quest_id, QUEST_STATUS_CHOOSE_REWARD);
                        Log::getSingleton( ).outString( "WORLD: Sent SMSG_QUESTGIVER_REQUEST_ITEMS" );
                    }               
                }
                else if(qg_status == QUEST_STATUS_AVAILABLE)
                {
                    // Send quest details
                    char *title = world.mCreatures[guid1]->getQuestTitle(quest_id);
                    char *details = world.mCreatures[guid1]->getQuestDetails(quest_id);
                    char *objectives = world.mCreatures[guid1]->getQuestObjectives(quest_id);
                    Quest *pQuest = world.getQuest(quest_id);


                    uint16 rewardSize = 52;
                    rewardSize += pQuest->m_choiceRewards*12;
                    rewardSize += pQuest->m_itemRewards*12;

                    data.Initialise( 8 + 4 + strlen(title)+1 + strlen(details)+1 + strlen(objectives)+1 + rewardSize, SMSG_QUESTGIVER_QUEST_DETAILS );
                    data << guid1 << guid2 << quest_id;
                    data.writeData( title, strlen(title)+1 );
                    data.writeData( details, strlen(details)+1 );
                    data.writeData( objectives, strlen(objectives)+1 );

                    data << uint32(1);
                    data << uint32(pQuest->m_choiceRewards);

                    for (int i=0; i < pQuest->m_choiceRewards; i++){
                        data << pQuest->m_choiceItemId[i] << pQuest->m_choiceItemCount[i];
                        data << uint32(0);
                    }

                    data << uint32(pQuest->m_itemRewards);
                    for (int i=0; i < pQuest->m_itemRewards; i++){
                        data << uint32(pQuest->m_rewardItemId[i]) << uint32(pQuest->m_rewardItemCount[i]);
                        data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                    }

                    data << uint32(pQuest->m_rewardGold);
                    data << uint32(0) << uint32(0);
                    data << uint32(0) << uint32(0);
                    data << uint32(0) << uint32(0);
                    data << uint32(0) << uint32(0) << uint32(0);

                    pClient->SendMsg( &data );
                    Log::getSingleton( ).outString( "WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS" );
                }

            }break;
        case CMSG_QUESTGIVER_ACCEPT_QUEST:
            {
                Log::getSingleton( ).outString( "WORLD: Recieved CMSG_QUESTGIVER_ACCEPT_QUEST" );
                uint32 guid1, guid2, quest_id;
                recv_data >> guid1 >> guid2 >> quest_id;
                WPAssert( guid1 != 0 );
                Quest *pQuest = world.getQuest(quest_id);

                uint16 log_slot = pClient->getCurrentChar()->getOpenQuestSlot();
                if (log_slot == 0)
                {
                    // TODO:  Send log full message
                    break;
                }
                
                pClient->getCurrentChar()->setUpdateValue(log_slot, quest_id);
                pClient->getCurrentChar()->setUpdateValue(log_slot+1, uint32(0x337));

//                pClient->SendMsg( &data );
                Log::getSingleton( ).outString( "WORLD: Sent Quest Acceptance 0xA9" );

                if (pQuest->m_targetGuid != 0)
                {
                    SetNpcFlagsForTalkToQuest(pClient, guid1, pQuest->m_targetGuid);
                }

                pClient->getCurrentChar()->setQuestStatus(quest_id, QUEST_STATUS_INCOMPLETE);
            }break;
        case CMSG_QUEST_QUERY:
            {
                Log::getSingleton( ).outString( "WORLD: Recieved CMSG_QUEST_QUERY" );
                uint32 quest_id=0, guid1=0;
                recv_data >> quest_id;

                if (quest_id > 10){ // assuming there are no more than 10 quests for now
                    Log::getSingleton().outString("Bad Quest Query requested\n");   
                    return;
                }

                for( std::map<uint32, Unit*>::iterator i = world.mCreatures.begin( ); i != world.mCreatures.end( ); ++ i ) {
                    if(i->second != NULL) {
						if(i->second->hasQuest(quest_id))
						    guid1 = i->second->getGUID();
					}
                }
                WPAssert( guid1 != 0 );

                char *title = world.mCreatures[guid1]->getQuestTitle(quest_id);
                char *details = world.mCreatures[guid1]->getQuestDetails(quest_id);
                char *objectives = world.mCreatures[guid1]->getQuestObjectives(quest_id);
                Quest *pQuest = world.getQuest(quest_id);


                uint16 length = 140 + strlen(title)+1 + strlen(details)+1 + strlen(objectives)+1 + 69;
                data.Initialise( 4 + length, SMSG_QUEST_QUERY_RESPONSE );
                data << quest_id;

                // tdata
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                data << uint32(pQuest->m_zone);

                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00); // reputation faction?
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);

                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                data << uint32(pQuest->m_rewardGold);
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);

                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);

                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);
                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);

                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);

                for (int i=0; i < 5; i++){
                    data << uint32(pQuest->m_choiceItemId[i]) << uint32(pQuest->m_choiceItemCount[i]);
                }

                data << uint32(0);
                data << uint32(0x01);
                data << uint32(0xFF);
                data << uint32(0);
                data << uint32(0);

                data << uint8(0x00) << uint8(0x00) << uint8(0x00) << uint8(0x00);

                data.writeData( title, strlen(title)+1 );
                data.writeData( objectives, strlen(objectives)+1 );
                data.writeData( details, strlen(details)+1 );
                
                // quest requirements
                data << uint8(0);
                data << uint32(pQuest->m_questMobId[0])  << uint32(pQuest->m_questMobCount[0]);
                data << uint32(pQuest->m_questItemId[0]) << uint32(pQuest->m_questItemCount[0]);
                data << uint32(pQuest->m_questMobId[1])  << uint32(pQuest->m_questMobCount[1]);
                data << uint32(pQuest->m_questItemId[1]) << uint32(pQuest->m_questItemCount[1]);
                data << uint32(pQuest->m_questMobId[2])  << uint32(pQuest->m_questMobCount[2]);
                data << uint32(pQuest->m_questItemId[2]) << uint32(pQuest->m_questItemCount[2]);
                data << uint32(pQuest->m_questMobId[3])  << uint32(pQuest->m_questMobCount[3]);
                data << uint32(pQuest->m_questItemId[3]) << uint32(pQuest->m_questItemCount[3]);
                data << uint32(0);

                pClient->SendMsg( &data );
                Log::getSingleton( ).outString( "WORLD: Sent SMSG_QUEST_QUERY_RESPONSE" );

            }break;
        case CMSG_QUESTGIVER_CHOOSE_REWARD:
            {
                Log::getSingleton( ).outString( "WORLD: Recieved CMSG_QUESTGIVER_CHOOSE_REWARD" );
                uint32 guid1,guid2,quest_id,rewardid;
                recv_data >> guid1 >> guid2 >> quest_id >> rewardid;

                Quest *pQuest = world.getQuest(quest_id);

                data.Initialise( 16 + 4, SMSG_QUESTGIVER_QUEST_COMPLETE );
                data << quest_id;
                data.writeData(uint32(0x03));  // unsure
                data.writeData(uint32(pQuest->m_questXp));
                data.writeData(uint32(pQuest->m_rewardGold));
                data << uint32(0x00);
                pClient->SendMsg( &data );

                pClient->getCurrentChar()->setQuestStatus(quest_id, QUEST_STATUS_COMPLETE);

                if (pQuest->m_targetGuid != 0 && pQuest->m_originalGuid != 0){
                    // Do some special actions for "Talk to..." quests
                    UpdateMask npcMask;
                    npcMask.setCount(UNIT_BLOCKS);
                    npcMask.setBit(OBJECT_FIELD_GUID );
                    npcMask.setBit(OBJECT_FIELD_GUID+1 );
                    npcMask.setBit(UNIT_NPC_FLAGS );
                    // added code to buffer flags and set back so other players don't see the change -RG
                    // note that this buffering is *not* thread safe and should be only temporary

                    uint32 orig = pQuest->m_originalGuid;
                    WPAssert( orig != 0 );
                    uint32 valuebuffer = world.mCreatures[ orig ]->getUpdateValue( UNIT_NPC_FLAGS  );
                    world.mCreatures[orig]->setUpdateValue(UNIT_NPC_FLAGS , uint32(2));
                    world.mCreatures[orig]->UpdateObject(&npcMask, &data);
                    pClient->SendMsg(&data);
                    world.mCreatures[orig]->setUpdateValue(UNIT_NPC_FLAGS , valuebuffer);

                    uint32 targGuid = pQuest->m_targetGuid;
                    WPAssert( targGuid != 0 );
                    valuebuffer = world.mCreatures[targGuid]->getUpdateValue(UNIT_NPC_FLAGS );
                    world.mCreatures[targGuid]->setUpdateValue(UNIT_NPC_FLAGS , uint32(2));
                    world.mCreatures[targGuid]->UpdateObject(&npcMask, &data);
                    pClient->SendMsg(&data);
                    world.mCreatures[targGuid]->setUpdateValue(UNIT_NPC_FLAGS , valuebuffer);
                }
              
                Log::getSingleton( ).outString( "WORLD: Sent SMSG_QUESTGIVER_QUEST_COMPLETE" );

                uint16 log_slot = pClient->getCurrentChar()->getQuestSlot(quest_id);

                // Set player object with rewards!
                Character *chr = pClient->getCurrentChar();
                
                uint32 guid = chr->getGUID();

                chr->giveXP(pQuest->m_questXp);
                if (pQuest->m_rewardGold > 0){
                    uint32 newCoinage = chr->getUpdateValue(PLAYER_FIELD_COINAGE ) + pQuest->m_rewardGold;
                    chr->setUpdateValue(PLAYER_FIELD_COINAGE , newCoinage);
                }

                chr->setUpdateValue(log_slot, 0);
                chr->setUpdateValue(log_slot+1, 0);
                chr->setUpdateValue(log_slot+2, 0);
                chr->setUpdateValue(log_slot+3, 0);
            }break;

        case CMSG_QUESTGIVER_REQUEST_REWARD :
            {
                /*  Not really sure what this is all about.  SEnt from a SMSG_QUESTGIVER_REQUEST_ITEMS

                uint32 guid1, guid2, quest_id;
                recv_data >> guid1 >> guid2 >> quest_id;

                char *title = world.mCreatures[guid1]->getQuestTitle(quest_id);
                char *details = world.mCreatures[guid1]->getQuestCompleteText(quest_id);

                
                unsigned char tdata[] = 
                { 
                    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 
                };

                data.Initialise( 8+4+strlen(title)+1 + strlen(details)+1 +sizeof(tdata), SMSG_QUESTGIVER_OFFER_REWARD );
                data << guid1 << guid2 << uint32( 0x1 );
                data.writeData( title, strlen(title)+1 );
                data.writeData( details, strlen(details)+1 );
                data.writeData( tdata, sizeof(tdata) );
                pClient->SendMsg( &data );
                Log::getSingleton( ).outString( "WORLD: Sent SMSG_QUESTGIVER_OFFER_REWARD" );
                pClient->getCurrentChar()->setQuestStatus(quest_id, QUEST_STATUS_CHOOSE_REWARD);
                */
            }break;
    }
}

void QuestHandler::addQuest(Quest *pQuest)
{
    mQuests[pQuest->m_questId] = pQuest;
}

Quest* QuestHandler::getQuest(uint32 quest_id)
{
    assert( mQuests.find( quest_id ) != mQuests.end( ) );
    return mQuests[quest_id];
}


void QuestHandler::SetNpcFlagsForTalkToQuest(GameClient* pClient, uint32 guid1, uint32 targetGuid)
{
    // Do some special actions for "Talk to..." quests
    wowWData data;
    UpdateMask npcMask;

    npcMask.setCount(UNIT_BLOCKS);
    npcMask.setBit(UNIT_NPC_FLAGS );

    Unit* pGiver = world.GetValidCreature(guid1);
    uint32 valuebuffer = pGiver->getUpdateValue( UNIT_NPC_FLAGS  );
    pGiver->setUpdateValue(UNIT_NPC_FLAGS , uint32(0));
    pGiver->UpdateObject(&npcMask, &data);
    pClient->SendMsg(&data);
    pGiver->setUpdateValue(UNIT_NPC_FLAGS , valuebuffer);

    Unit* pTarget = world.GetValidCreature(targetGuid);

    valuebuffer = pTarget->getUpdateValue(UNIT_NPC_FLAGS );
    pTarget->setUpdateValue(UNIT_NPC_FLAGS , uint32(2));
    pTarget->UpdateObject(&npcMask, &data);
    pClient->SendMsg(&data);
    pTarget->setUpdateValue(UNIT_NPC_FLAGS , valuebuffer);
}

/*  Query Response Header
                uint8 tdata[] = 
                { 
                    // Flags of some sort?
                    0x02, 0x00, 0x00, 0x00, 
                    0x04, 0x00, 0x00, 0x00, 
                    0x09, 0x00, 0x00, 0x00,

                    0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, //<-- Reputation Faction!?  If I set above 0, CRASH!  Probably have to set factions first
                    0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 
                    0x05, 0x00, 0x00, 0x00, // Oooh, gold reward?

                    0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00,

                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

                    // Item Reward list
                    0xb0, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, // Item Entry ID and reward count
                    0xcc, 0x15, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                    0x89, 0x04, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0xcb, 0x15, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x87, 0x04, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,

                    0x00, 0x00, 0x00, 0x00, // setting any of these bytes to 1 makes it ignore the Item rewards Above. Ooook then.
                
                    0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                };
*/
/* another Query Response Header
                // rewards?
                uint8 tdata[] = 
                { 
                    // Flags of some sort?
                    0x2B, 0x02, 0x00, 0x00, 
                    0x3E, 0x03, 0x00, 0x00,  
                    0x90, 0x00, 0x00, 0x00, // zone id to store quest in log

                    0x02, 0x00, 0x00, 0x00, 

                    0x00, 0x00, 0x00, 0x00, //<-- Reputation Faction!?  If I set above 0, CRASH!  Probably have to set factions first

                    0x0B, 0x01, 0x00, 0x00, 

                    0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, // Oooh, gold reward
                    0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00,

                    0xF4, 0x01, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00,
                 
                    0x99, 0x0E, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, // Item rewards you always get, no choosing
                    0x91, 0x0E, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 
                
                    0x00, 0x00, 0x00, 0x00,

                    // Item Reward Choice list
                    0x82, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, // Item Entry ID and reward count
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

                    0x00, 0x00, 0x00, 0x00, // setting any of these bytes to 1 makes it ignore the Item rewards Above. Ooook then.
                
                    0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                };
*/



/*
SMSG_QUESTGIVER_OFFER_REWARD
4 bytes
4 bytes
4 bytes
4 bytes
4 bytes - # of items to choose from
  for itemCount
    4 bytes - item entry id
    4 bytes - # in stack
    4 bytes - Item icon (?)

4 bytes - # of items being rewarded
  for itemCount
    4 bytes - item entry id
    4 bytes - # in stack
    4 bytes - Item icon (?)
4 bytes - Gold rewarded
4 bytes
*/

/*
SMSG_QUESTGIVER_QUEST_DETAILS

8 bytes - questgiver guid
4 bytes - quest id
string - quest title
string - quest description
string - quest objectives
4 bytes - ?
4 bytes - number of rewards to choose from
  for each item
    4 bytes - item name entry id
    4 bytes - # in stack
    4 bytes - picture id
4 bytes - number of item rewards always awarded
  for each item
    4 bytes - item name entry id
    4 bytes - # in stack
    4 bytes - picture id
4 bytes - reward gold
4 bytes ?
4 bytes ?
4 bytes ?
4 bytes ?
4 bytes ?
4 bytes ?
4 bytes ?
4 bytes ?

*/

/*
SMSG_QUEST_QUERY_RESPONSE

4 bytes - quest id
4 bytes - ?
4 bytes - ?
4 bytes - Quest Zone
4 bytes - ?
4 bytes - Faction (?)
4 bytes - ?
4 bytes - ?
4 bytes - ?
4 bytes - ?
4 bytes - Gold rewarded
4 bytes - ?
4 bytes - ?

4 bytes - ?
4 bytes - ?

4 bytes - ?
4 bytes - ?
4 bytes - ?
4 bytes - ?

4 bytes - ?

loop 5 times
  4 bytes - Item name entry id
  4 bytes - items tack count

4 bytes - ?
4 bytes - ?
4 bytes - ?
4 bytes - ?
4 bytes - ?
4 bytes - ?
string - quest title
string - objectives
string - decription
1 byte - ? some kind of flag ?
4 bytes - Creature to slay entry ID
4 bytes - # of those creature to slay
4 bytes - Item ID to collect
4 bytes - # of those items to collect
4 bytes - Creature to slay entry ID
4 bytes - # of those creature to slay
4 bytes - Item ID to collect
4 bytes - # of those items to collect
4 bytes - Creature to slay entry ID
4 bytes - # of those creature to slay
4 bytes - Item ID to collect
4 bytes - # of those items to collect
4 bytes - Creature to slay entry ID
4 bytes - # of those creature to slay
4 bytes - Item ID to collect
4 bytes - # of those items to collect
4 bytes - ?

*/

/*
SMSG_QUESTUPDATE_ADD_KILL[00000194]

4 bytes - Quest ID
4 bytes - Entry ID of Monster Killed
4 bytes - Number of kills to add?
4 bytes - Total kills required for quest?
8 bytes - Killed Monster GUID
*/
