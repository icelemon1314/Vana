/*
Copyright (C) 2008 Vana Development Team

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
#include "Shops.h"
#include "Inventory.h"
#include "ItemDataProvider.h"
#include "MapleSession.h"
#include "PacketCreator.h"
#include "Player.h"
#include "SendHeader.h"

unordered_map<int32_t, ShopInfo> Shops::shops;
vector<int32_t> Shops::rechargables;

void Shops::addShop(int32_t id, ShopInfo shop) {
	shops[id] = shop;
}

void Shops::showShop(Player *player, int32_t id) {
	player->setShop(id);
	if (shops.find(id) == shops.end())
		return;
	PacketCreator packet;
	packet.addShort(SEND_SHOP_OPEN);
	packet.addInt(shops[id].npc);
	packet.addShort(shops[id].items.size() + rechargables.size());
	for (size_t i = 0; i < shops[id].items.size(); i++) {
		packet.addInt(shops[id].items[i]);
		packet.addInt(shops[id].prices[shops[id].items[i]]);
		if (ISRECHARGEABLE(shops[id].items[i])) {
			packet.addShort(0);
			packet.addInt(0);
			packet.addShort((uint16_t) shops[id].prices[shops[id].items[i]]);
		}
		else {
			packet.addShort(1);
		}
		if (ISSTAR(shops[id].items[i]))
			packet.addShort(ItemDataProvider::Instance()->getMaxslot(shops[id].items[i]) + player->getSkills()->getSkillLevel(4100000) * 10);
		else if (ISBULLET(shops[id].items[i]))
			packet.addShort(ItemDataProvider::Instance()->getMaxslot(shops[id].items[i]) + player->getSkills()->getSkillLevel(5200000) * 10);
		else if (ItemDataProvider::Instance()->itemExists(shops[id].items[i]))
			packet.addShort(ItemDataProvider::Instance()->getMaxslot(shops[id].items[i])); 
		else
			packet.addShort(1000);
	}

	for (size_t i = 0; i < rechargables.size(); i++) {
		packet.addInt(rechargables[i]);
		packet.addInt(0);
		packet.addShort(0);
		packet.addInt(0);
		packet.addShort(1);
		packet.addShort(ItemDataProvider::Instance()->getMaxslot(rechargables[i]) + (ISSTAR(rechargables[i]) ? player->getSkills()->getSkillLevel(4100000) * 10 : player->getSkills()->getSkillLevel(5200000) * 10));
	}

	player->getSession()->send(packet);
}

int32_t Shops::getPrice(int32_t shopid, int32_t itemid) {
	return shops[shopid].prices.find(itemid) != shops[shopid].prices.end() ? shops[shopid].prices[itemid] : 0;
}
