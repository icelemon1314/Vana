/*
Copyright (C) 2008-2009 Vana Development Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "WorldServerAcceptHandler.h"
#include "WorldServerAcceptPacket.h"
#include "WorldServerAcceptPlayer.h"
#include "WorldServer.h"
#include "Channels.h"
#include "Players.h"
#include "PacketReader.h"
#include "PartyHandler.h"

void WorldServerAcceptHandler::groupChat(WorldServerAcceptPlayer *player, PacketReader &packet) {
	int32_t playerid = packet.get<int32_t>();
	int8_t type = packet.get<int8_t>(); // Buddy = 0 party = 1 guild = 2
	string message = packet.getString();
	uint8_t receivers = packet.get<int8_t>();
	string sender = Players::Instance()->getPlayer(playerid)->name;
	for (size_t i = 0; i < receivers; i++) {
		int32_t receiver = packet.get<int32_t>();
		WorldServerAcceptPlayer *channel = Channels::Instance()->getChannel(Players::Instance()->getPlayer(receiver)->channel)->player;
		WorldServerAcceptPacket::groupChat(channel, receiver, type, message, sender);
	}	
}

void WorldServerAcceptHandler::partyOperation(WorldServerAcceptPlayer *player, PacketReader &packet) {
	int8_t type = packet.get<int8_t>();
	int32_t playerid = packet.get<int32_t>();
	switch (type) {
		case 0x01: PartyHandler::createParty(player, playerid); break;
		case 0x02: PartyHandler::leaveParty(player, playerid); break;
		case 0x03: PartyHandler::joinParty(player, playerid, packet.get<int32_t>()); break;
		case 0x04: PartyHandler::invitePlayer(player, playerid, packet.getString()); break;
		case 0x05: PartyHandler::expelPlayer(player, playerid, packet.get<int32_t>()); break;
		case 0x06: PartyHandler::giveLeader(player, playerid, packet.get<int32_t>(), 0); break;
	}
}

void WorldServerAcceptHandler::playerChangeChannel(WorldServerAcceptPlayer *player, PacketReader &packet) {
	int32_t playerid = packet.get<int32_t>();
	Channel *chan = Channels::Instance()->getChannel(packet.get<int16_t>());

	if (chan) {
		WorldServerAcceptPacket::newConnectable(chan->id, playerid);
		WorldServerAcceptPacket::playerChangeChannel(player, playerid, chan->ip, chan->port);
	}
	else { // Channel doesn't exist (offline)
		WorldServerAcceptPacket::playerChangeChannel(player, playerid, "255.255.255.255", -1);
	}
}

void WorldServerAcceptHandler::findPlayer(WorldServerAcceptPlayer *player, PacketReader &packet) {
	int32_t finder = packet.get<int32_t>();
	string findee_name = packet.getString();

	Player *findee = Players::Instance()->getPlayerFromName(findee_name);
	if (findee->channel != 65535) // Thanks for changing the datatype, pawitp ;_;
		WorldServerAcceptPacket::findPlayer(player, finder, findee->channel, findee->name);
	else
		WorldServerAcceptPacket::findPlayer(player, finder, findee->channel, findee_name);
}

void WorldServerAcceptHandler::whisperPlayer(WorldServerAcceptPlayer *player, PacketReader &packet) {
	int32_t whisperer = packet.get<int32_t>();
	string whisperee_name = packet.getString();
	string message = packet.getString();

	Player *whisperee = Players::Instance()->getPlayerFromName(whisperee_name);
	if (whisperee->channel != 65535) { // Thanks for changing the datatype, pawitp ;_;
		WorldServerAcceptPacket::findPlayer(player, whisperer, -1, whisperee->name, 1);
		WorldServerAcceptPacket::whisperPlayer(Channels::Instance()->getChannel(whisperee->channel)->player, whisperee->id, Players::Instance()->getPlayer(whisperer)->name, player->getChannel(),  message);
	}
	else
		WorldServerAcceptPacket::findPlayer(player, whisperer, whisperee->channel, whisperee_name);
}

void WorldServerAcceptHandler::registerPlayer(WorldServerAcceptPlayer *player, PacketReader &packet) {
	int32_t id = packet.get<int32_t>();
	string name = packet.getString();
	int32_t map = packet.get<int32_t>();
	int32_t job = packet.get<int32_t>();
	int32_t level = packet.get<int32_t>();
	Players::Instance()->registerPlayer(id, name, player->getChannel(), map, job, level);
}

void WorldServerAcceptHandler::removePlayer(WorldServerAcceptPlayer *player, PacketReader &packet) {
	int32_t id = packet.get<int32_t>();
	Players::Instance()->remove(id, player->getChannel());
}

void WorldServerAcceptHandler::scrollingHeader(WorldServerAcceptPlayer *player, PacketReader &packet) {
	string message = packet.getString();
	WorldServer::Instance()->setScrollingHeader(message);
}

void WorldServerAcceptHandler::updateJob(WorldServerAcceptPlayer *player, PacketReader &packet) {
	int32_t id = packet.get<int32_t>();
	int32_t job = packet.get<int32_t>();
	Players::Instance()->getPlayer(id)->job = job;
	if (Players::Instance()->getPlayer(id)->party != 0) {
		PartyHandler::silentUpdate(id);
	}
}

void WorldServerAcceptHandler::updateLevel(WorldServerAcceptPlayer *player, PacketReader &packet) {
	int32_t id = packet.get<int32_t>();
	int32_t level = packet.get<int32_t>();
	Players::Instance()->getPlayer(id)->level = level;
	if (Players::Instance()->getPlayer(id)->party != 0) {
		PartyHandler::silentUpdate(id);
	}
}

void WorldServerAcceptHandler::updateMap(WorldServerAcceptPlayer *player, PacketReader &packet) {
	int32_t id = packet.get<int32_t>();
	int32_t map = packet.get<int32_t>();
	Players::Instance()->getPlayer(id)->map = map;
	if (Players::Instance()->getPlayer(id)->party != 0) {
		PartyHandler::silentUpdate(id);
	}
}
