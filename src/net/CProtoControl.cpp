/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CProtoControl.c
    Created: Thursday, March 2, 1995, 22:37
    Modified: Sunday, September 1, 1996, 19:10
*/

#include "CProtoControl.h"

#include "AvaraDefines.h"
#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CNetManager.h"
#include "CPlayerManager.h"
#include "CommDefs.h"
#include "CommandList.h"
#include "Preferences.h"
#include "Debug.h"

static Boolean ImmedProtoHandler(PacketInfo *thePacket, Ptr userData) {
    CProtoControl *theControl;

    theControl = (CProtoControl *)userData;
    // SDL_Log("HANDLING PACKET type=%d p1=%d p2=%d p3=%d\n", thePacket->command, thePacket->p1, thePacket->p2,
    // thePacket->p3);

    return theControl->PacketHandler(thePacket);
}
static Boolean DelayedProtoHandler(PacketInfo *thePacket, Ptr userData) {
    CProtoControl *theControl;

    theControl = (CProtoControl *)userData;
    // SDL_Log("HANDLING DELAYED PACKET type=%d p1=%d p2=%d p3=%d\n", thePacket->command, thePacket->p1, thePacket->p2,
    // thePacket->p3);

    return theControl->DelayedPacketHandler(thePacket);
}

void CProtoControl::IProtoControl(CCommManager *aManager, CAvaraGame *aGame) {
    theGame = aGame;

    immedReceiverRecord.handler = ImmedProtoHandler;
    immedReceiverRecord.userData = (Ptr)this;

    delayedReceiverRecord.handler = DelayedProtoHandler;
    delayedReceiverRecord.userData = (Ptr)this;

    Attach(aManager);
}

void CProtoControl::Attach(CCommManager *aManager) {
    itsManager = aManager;
    itsManager->AddReceiver(&immedReceiverRecord, true);
    itsManager->AddReceiver(&delayedReceiverRecord, false);
}

Boolean CProtoControl::DelayedPacketHandler(PacketInfo *thePacket) {
    CNetManager *theNet = theGame->itsNet;
    Boolean didHandle = true;

    switch (thePacket->command) {
        case kpKillConnection:
            theNet->HandleDisconnect(thePacket->p1, 0);
            break;
        //	case kpError:
        case kpDisconnect:
            theNet->HandleDisconnect(thePacket->sender, thePacket->p2);
            break;
        case kpLogin:
            itsManager->SendPacket(
                1 << thePacket->sender, kpServerOptions, 0, theGame->itsApp->Number(kServerOptionsTag), 0, 0, NULL);

            theNet->SwapPositions(0, 0); //	Broadcast order
            theNet->SendColorChange();
            break;
        case kpNameQuery:
            theGame->itsApp->BroadcastCommand(kReportNameCmd);
            theNet->SendRealName(thePacket->p1);
            break;
        case kpNameChange:
            theNet->RecordNameAndLocation(
                thePacket->sender, (StringPtr)thePacket->dataBuffer, thePacket->p2, *(Point *)&thePacket->p3);
            break;
        case kpOrderChange:
            theNet->PositionsChanged(thePacket->dataBuffer);
            break;
        case kpRosterMessage:
            theNet->ReceiveRosterMessage(thePacket->sender, thePacket->dataLen, thePacket->dataBuffer);
            break;
        case kpColorChange:
            theNet->ReceiveColorChange(thePacket->dataBuffer);
            break;
        case kpLoadLevel:
            // p2 is proxy for which slot originally sent the kpLoadLevel command
            theNet->ReceiveLoadLevel(thePacket->sender, thePacket->p2, thePacket->dataBuffer, thePacket->p3);
            break;
        case kpLevelLoaded:
            theNet->LevelLoadStatus(thePacket->sender, thePacket->p2, 0, std::string(thePacket->dataBuffer));
            break;
        case kpLevelLoadErr:
            theNet->LevelLoadStatus(thePacket->sender, 0, thePacket->p2, std::string(thePacket->dataBuffer));
            break;
        case kpKillNet:
            theGame->itsApp->BroadcastCommand(kNetChangedCmd);
            break;
        case kpStartLevel:
            theNet->ReceiveStartCommand(thePacket->p2, thePacket->sender, thePacket->p1);
            break;
        case kpResumeLevel:
            theNet->ReceiveResumeCommand(thePacket->p2, thePacket->sender, thePacket->p3, thePacket->p1);
            break;
        case kpReadySynch:
            theNet->readyPlayers |= 1 << thePacket->sender;
            SDL_Log("--- readyPlayers = 0x%02x\n", theNet->readyPlayers);
            break;
        case kpUnavailableSynch:
            theNet->ReceivedUnavailable(thePacket->sender, thePacket->p1);
            break;
        case kpUnavailableZero:
            theNet->unavailablePlayers = 0;
            break;
        case kpStartSynch:
            theNet->ConfigPlayer(thePacket->sender, thePacket->dataBuffer);
            theNet->ReceivePlayerStatus(thePacket->sender, thePacket->p2, thePacket->p3, -1);
            break;
        case kpPlayerStatusChange:
            theNet->ReceivePlayerStatus(
                thePacket->p1, thePacket->p2, thePacket->p3, *(long *)thePacket->dataBuffer);
            break;
        case kpJSON:
            theNet->ReceiveJSON(
                thePacket->p1, thePacket->p2, thePacket->p3, std::string(thePacket->dataBuffer));
            break;
        case kpKeyAndMouseRequest: {
            theGame->itsNet->playerTable[itsManager->myId]->ResendFrame(
                thePacket->p3, thePacket->sender, kpKeyAndMouse);
        } break;

        case kpGetMugShot:
            theNet->MugShotRequest(thePacket->sender, thePacket->p3);
            break;
        case kpMugShot:
            theNet->ReceiveMugShot(
                thePacket->sender, thePacket->p2, thePacket->p3, thePacket->dataLen, thePacket->dataBuffer);
            break;
        case kpZapMugShot:
            theNet->ZapMugShot(thePacket->sender);
            break;
        case kpServerOptions:
            theNet->serverOptions = thePacket->p2;
            break;
        case kpNewArrival:
            theNet->NewArrival(thePacket->p1);
            break;

        case kpKickClient:
            theNet->HandleDisconnect(itsManager->myId, kpKickClient);
            break;

        case kpLatencyVote:
            theNet->ReceiveLatencyVote(thePacket->sender, thePacket->p1, thePacket->p2, thePacket->p3);
            break;

        case kpResultsReport:
            theNet->ResultsReport(thePacket->dataBuffer);
            break;

        case kpRealName:
            theNet->RealNameReport(thePacket->sender, thePacket->p1, (StringPtr)thePacket->dataBuffer);
            break;
        case kpPacketProtocolRefusal:
            theNet->LoginRefused();
            break;

        default:
            didHandle = false;
    }

    return didHandle;
}

static std::string FormatDist(uint16_t distribution) {
    std::bitset<kMaxAvaraPlayers> bits{distribution};
    return bits.to_string();
}

Boolean CProtoControl::PacketHandler(PacketInfo *thePacket) {
    Boolean didHandle = true;
    CNetManager *theNet = theGame->itsNet;

    // SDL_Log("CProtoControl::PacketHandler cmd=%d sender=%d\n", thePacket->command, thePacket->sender);
    switch (thePacket->command) {
        case kpLogin: //	Only servers see this
        {
            short senderDistr = 1 << thePacket->sender;
            DBG_Log("login", "kpLogin received from = %d\n", thePacket->sender);
            DBG_Log("login", "sending kpLoginAck to = %s\n", FormatDist(senderDistr).c_str());
            itsManager->SendPacket(senderDistr, kpLoginAck, thePacket->sender, 0, 0, 0, NULL);
            DBG_Log("login", "sending kpNameQuery(%d) to = %s\n", thePacket->sender, FormatDist(kdEveryone).c_str());
            itsManager->SendPacket(kdEveryone, kpNameQuery, thePacket->sender, 0, 0, 0, NULL);
            DBG_Log("login", "sending kpNewArrival(%d) to = %s\n", thePacket->sender, FormatDist(~senderDistr).c_str());
            itsManager->SendPacket(~senderDistr, kpNewArrival, thePacket->sender, 0, 0, 0, NULL);
            didHandle = false;
        } break;
        case kpLoginAck:
            itsManager->myId = thePacket->p1;
            DBG_Log("login", "kpLoginAck received with myId = %d\n", itsManager->myId);
            itsManager->SendPacket(kdEveryone - (1 << thePacket->p1), kpPing, 0, 0, 32, 0, NULL);
            // kpLoginAck is called when anyone joins/exits, so good place to check where the "local" player is
            for (int i = 0; i < kMaxAvaraPlayers; i++) {
                theNet->playerTable[i]->SetLocal(); // reset which player is "local"
            }
            break;
        case kpKeyAndMouseRequest:

            didHandle = false;
            //	Fall through to kpKeyAndMouse!
        case kpKeyAndMouse: {
            short playerIndex;

            playerIndex = thePacket->sender;
            if (playerIndex >= 0 && playerIndex < kMaxAvaraPlayers)
                theNet->playerTable[playerIndex]->ProtocolHandler(thePacket);
        } break;
        case kpAskLater: //	Packet was not lost, so ask again later.
            theNet->playerTable[thePacket->sender]->IncrementAskAgainTime(300);
            break;

            /*
            case kpFastTrack:
                theNet->fastTrack.addresses[thePacket->sender].value = thePacket->p3;
                break;
            */

        case kpRemoveMeFromGame:
            theNet->activePlayersDistribution &= ~(1 << thePacket->sender);
            break;

        case kpPing:
            if (!theNet->isPlaying && thePacket->p3) {
                itsManager->SendPacket(1 << thePacket->sender, kpPing, 0, 0, thePacket->p3 - 1, 0, NULL);
            }
            break;

        default:
            didHandle = false;
    }

    return didHandle;
}

void CProtoControl::Detach() {
    if (itsManager) {
        itsManager->RemoveReceiver(&immedReceiverRecord, true);
        itsManager->RemoveReceiver(&delayedReceiverRecord, false);
        itsManager = NULL;
    }
}

void CProtoControl::Dispose() {
    CDirectObject::Dispose();
}
