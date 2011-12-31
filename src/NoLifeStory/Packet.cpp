////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

map<uint16_t, function<void(NLS::Packet&)>> NLS::Packet::Handlers;
map<NLS::Send::PacketType::Header, uint16_t> NLS::Send::SendHeaders;

void NLS::Packet::Send() {
	if (!Network::Connected or Profiling) return;
	uint16_t a = (Network::SendIV[3]<<8)+Network::SendIV[2];
	a ^= Network::Version;
	uint16_t b = a^(data.size()-4);
	data[0] = a%0x100;
	data[1] = a/0x100;
	data[2] = b%0x100;
	data[3] = b/0x100;
	cout << "Sent Packet: " << ToString() << endl;
	Encrypt();
	Network::Socket.Send((char*)data.data(), data.size());
}

#pragma region Encryption Stuff
#ifndef _WIN32
unsigned int _rotl8(uint8_t value, int shift)
{
	return (value<<shift)|(value>>(8-shift));
}

unsigned int _rotr8(uint8_t value, int shift)
{
	return (value>>shift)|(value<<(8-shift));
}
#endif //_WIN32

void NextIV(uint8_t *oldiv);

void NLS::Packet::Encrypt() {
	int32_t j;
	uint8_t a, c;
	uint8_t* buf = data.data()+4;
	size_t size = data.size()-4;
	
	for (uint8_t i = 0; i < 3; i++) {
		a = 0;
		for (j = size; j > 0; j--) {
			c = buf[size-j];
			c = _rotl8(c, 3);
			c += j;
			c ^= a;
			a = c;
			c = _rotr8(c, j);
			c = ~c;
			c += 0x48;
			buf[size-j] = c;
		}
		a = 0;
		for (j = size; j > 0; j--) {
			c = buf[j-1];
			c = _rotl8(c, 4);
			c += j;
			c ^= a;
			a = c;
			c ^= 0x13;
			c = _rotr8(c, 3);
			buf[j-1] = c;
		}
	}
	
	Crypto::TransformData(NLS::Network::SendIV, buf, size);
	NextIV(NLS::Network::SendIV);
}

void NLS::Packet::Decrypt() {
	uint8_t* buf = data.data(); // NO HEADERS HERE!!!
	size_t size = data.size();
	Crypto::TransformData(NLS::Network::RecvIV, buf, size);
	NextIV(NLS::Network::RecvIV);
	int32_t j;
	uint8_t a, b, c;
	for (uint8_t i = 0; i < 3; i++) {
		a = 0;
		b = 0;
		for (j = size; j > 0; j--) {
			c = buf[j-1];
			c = _rotl8(c, 3);
			c ^= 0x13;
			a = c;
			c ^= b;
			c -= j;
			c = _rotr8(c, 4);
			b = a;
			buf[j-1] = c;
		}
		a = 0;
		b = 0;
		for (j = size; j > 0; j--) {
			c = buf[size-j];
			c -= 0x48;
			c = ~c;
			c = _rotl8(c, j);
			a = c;
			c ^= b;
			c -= j;
			c = _rotr8(c, 3);
			b = a;
			buf[size-j] = c;
		}
	}
}

void NextIV(uint8_t *oldiv) {
	uint8_t newIV[4] = {0xF2, 0x53, 0x50, 0xC6};
	for (auto i = 0; i < 4; i++) {
		uint8_t input = oldiv[i];
		uint8_t tableInput = ShuffleKey[input];
		newIV[0] += ShuffleKey[newIV[1]]-input;
		newIV[1] -= newIV[2]^tableInput;
		newIV[2] ^= ShuffleKey[newIV[3]]+input;
		newIV[3] -= newIV[0]-tableInput;
		uint32_t& merged = *(uint32_t*)newIV;
		merged = merged<<3|merged>>0x1D;
	}
	for (auto i = 0; i < 4; i++) {
		oldiv[i] = newIV[i];
	}
}

#pragma endregion

#pragma region Packet Handlers

void NLS::Handle::Init() {
	Packet::Handlers.clear();
	Send::SendHeaders.clear();

	uint8_t locale = Network::Locale;
	uint16_t version = Network::Version;
	uint16_t subversion = toint(Network::Patch);

	if (locale == NLS::Locales::Global) {
		if (version == 75) {
			Packet::Handlers[0x11] = &NLS::Handle::Ping;
			Packet::Handlers[0x72] = &NLS::Handle::ChangeMap;
			Packet::Handlers[0x91] = &NLS::Handle::PlayerSpawn;
			Packet::Handlers[0x92] = &NLS::Handle::PlayerDespawn;
			Packet::Handlers[0x93] = &NLS::Handle::PlayerChat;
			Packet::Handlers[0xA7] = &NLS::Handle::PlayerMove;
			Packet::Handlers[0xAF] = &NLS::Handle::PlayerEmote;
			Packet::Handlers[0xE3] = &NLS::Handle::NpcSpawn;

			Send::SendHeaders[Send::PacketType::Login] = 0x01;
			Send::SendHeaders[Send::PacketType::Pong] = 0x19;
			Send::SendHeaders[Send::PacketType::LoadCharacter] = 0x14;
			Send::SendHeaders[Send::PacketType::PlayerChat] = 0x30;
			Send::SendHeaders[Send::PacketType::PlayerMove] = 0x28;
			Send::SendHeaders[Send::PacketType::NpcRequestTalk] = 0x39;
			Send::SendHeaders[Send::PacketType::RequestWorldBack] = 0x18;
			Send::SendHeaders[Send::PacketType::UsePortal] = 0x25;
			Send::SendHeaders[Send::PacketType::UsePortalScripted] = 0x63;
		}
		else if (version == 104) {
			Packet::Handlers[0x00] = &NLS::Handle::Login;
			Packet::Handlers[0x03] = &NLS::Handle::WorldSelectResult;
			Packet::Handlers[0x11] = &NLS::Handle::Ping;
			Packet::Handlers[0x0A] = &NLS::Handle::WorldListResult;
			Packet::Handlers[0x0B] = &NLS::Handle::WorldCharacters;
			Packet::Handlers[0xB2] = &NLS::Handle::SkippedObjects;

			Send::SendHeaders[Send::PacketType::Version] = 0x14;
			Send::SendHeaders[Send::PacketType::Login] = 0x15;
			Send::SendHeaders[Send::PacketType::ChannelSelectRequest] = 0x19;
			Send::SendHeaders[Send::PacketType::WorldSelectRequest] = 0x1A;
			Send::SendHeaders[Send::PacketType::RequestWorld] = 0x1F;
			Send::SendHeaders[Send::PacketType::RequestWorldBack] = 0x20;
			Send::SendHeaders[Send::PacketType::Pong] = 0x2E;
			Send::SendHeaders[Send::PacketType::HSStartup] = 0x37;
		}
	}

	cout << "[INFO] " << Packet::Handlers.size() << " handlers added for this MapleStory version." << endl;
}

void NLS::Handle::Ping(Packet &p) {
	Send::Pong();
}

void NLS::Handle::Login(Packet &p) {
	uint16_t op = p.Read<uint16_t>();
	if (op != 0x00) {
		cout << "Uh oh! Login Errors! Error code: " << op << endl;
		return;
	}
	if (Network::Locale == Locales::Global) {
		if (Network::Version >= 104) {
			p.Read<int32_t>(); // Unknown
			int32_t userid = p.Read<int32_t>(); // User ID
			p.Read<int32_t>(); // More unk. Might contain admin byte lol
			p.Read<int8_t>(); // 0x95??
			string username = p.Read<string>();

			cout << "Logged in as '" << username << "' (userID: " << userid << ")" << endl;
			
			p.Read<int32_t>(); // 0x00000003 ??
			// strange stuff!

			Send::RequestWorlds(false);
		}
	}
}

int32_t peopleOnline = 0;
void NLS::Handle::WorldListResult(Packet &p) {
	int8_t id = p.Read<int8_t>();
	if (id == -1) {
		// Return. Or do something more like showing the world list lol
		cout << "People online: " << peopleOnline << endl;
		peopleOnline = 0;
		View::LoginStage(1); // World Select
		return;
	}


	string name = p.Read<string>();
	int8_t ribbon = p.Read<int8_t>();
	string eventMsg = p.Read<string>();
	int16_t expRate = p.Read<int16_t>(); // EXP rate (100)
	int16_t dropRate = p.Read<int16_t>(); // Drop rate (100)
	p.Read<int8_t>(); // 0... always?

	UIWorldSelectScreen::WorldInfo *wi = new UIWorldSelectScreen::WorldInfo(id, ribbon, expRate, dropRate, name, eventMsg);

	u32string clr;
	switch (wi->ribbon) {
	case 1: clr = Text::Color(0, 0, 255); break; // Event - blue
	case 2: clr = Text::Color(0, 255, 0); break; // New - green
	case 3: clr = Text::Color(255, 0, 0); break; // Hot - red
	default: clr = Text::Color(0, 0, 0); break; // None or invalid - White
	}
	wi->worldDrawName.Set(clr + u32(wi->name), 12);

	uint8_t channels = p.Read<uint8_t>(); // Fun fact: the change channel screen has images for 50 (!) channels.
	for (auto i = 0; i < channels; i++) {
		UIWorldSelectScreen::WorldInfo::ChannelInfo ci;
		ci.name = p.Read<string>();
		ci.population = p.Read<int32_t>();
		ci.worldID = p.Read<int8_t>(); // Still unknown why this is sent. It does a check though
		ci.id = p.Read<int8_t>(); // Thanks for seperating this!
		ci.state = p.Read<int8_t>(); // EMS uses this for multi-language purpose!
		wi->channels[i] = ci;
		peopleOnline += ci.population;
	}

	int16_t msgs = p.Read<int16_t>(); // World screen messages. Bit buggy lol afaik never used too. Starts at top-left of world 0 tab
	for (auto i = 0; i < msgs; i++) {
		int16_t x = p.Read<int16_t>();
		int16_t y = p.Read<int16_t>();
		string msg = p.Read<string>();
	}
	WorldSelectScreen->worlds.push_back(wi);
}

void NLS::Handle::WorldSelectResult(Packet &p) {
	int8_t icon = p.Read<int8_t>();
	int8_t msg = p.Read<int8_t>();
	// lol
	WorldSelectScreen->ChannelSelectWindow->visible = true;
}

void NLS::Handle::WorldCharacters(Packet &p) {
	int8_t unk = p.Read<int8_t>();
	if (unk != 0) {
		cout << "Error on server side (worldchars): " << (int16_t)unk << endl;
		return;
	}
	int8_t characters = p.Read<int8_t>();
	
	for (auto i = 0; i < characters; i++) {
		cout << "Loop " << i << ". Pos: " << p.pos << endl;
		LoginPlayer *lp = new LoginPlayer();
		lp->charid = p.Read<int32_t>();
		
		lp->name = p.ReadStringLen(13);
		lp->gender = p.Read<int8_t>();
		lp->skin = p.Read<int8_t>();
		lp->face = p.Read<int32_t>();
		lp->hair = p.Read<int32_t>();

		p.Read<int64_t>(); // Taken out Pet ID's
		p.Read<int64_t>();
		p.Read<int64_t>();

		lp->level = p.Read<uint8_t>();

		LoginPlayer::Stats stats;
		stats.Job = p.Read<int16_t>(); // job
		stats.Str = p.Read<int16_t>(); // Strength
		stats.Dex = p.Read<int16_t>(); // Dex
		stats.Int = p.Read<int16_t>(); // Int
		stats.Luk = p.Read<int16_t>(); // Luk
		if (Network::Locale == Locales::Global && Network::Version < 81) {
			stats.HP = p.Read<int16_t>(); // HP
			stats.MaxHP = p.Read<int16_t>(); // MaxHP
			stats.MP = p.Read<int16_t>(); // MP
			stats.MaxMP = p.Read<int16_t>(); // MaxMP
		}
		else {
			stats.HP = p.Read<int32_t>(); // HP
			stats.MaxHP = p.Read<int32_t>(); // MaxHP
			stats.MP = p.Read<int32_t>(); // MP
			stats.MaxMP = p.Read<int32_t>(); // MaxMP
		}
		stats.AP = p.Read<int16_t>(); // AP
		cout << "0| Pos: " << p.pos << endl;
		if (is_extendsp_job(stats.Job)) {
			cout << "!!! Extended SP job: " << stats.Job << endl;
			int8_t spslots = p.Read<int8_t>();
			for (auto i = 0; i < spslots; i++) {
				int8_t slot = p.Read<int8_t>();
				int8_t amount = p.Read<int8_t>();
				if (slot < stats.SP.size()) {
					stats.SP[slot] = amount;
				}
			}
		}
		else {
			stats.SP[0] = p.Read<int16_t>();
		}
		stats.EXP = p.Read<int32_t>(); // EXP
		if (Network::Locale == Locales::Global && Network::Version > 102) {
			stats.Fame = p.Read<int32_t>(); // Fame
		}
		else {
			stats.Fame = p.Read<int16_t>(); // Fame
		}
		stats.GachaEXP = p.Read<int32_t>(); // GachaEXP
		lp->stats = stats;

		int32_t mapid = p.Read<int32_t>();
		int8_t mappos = p.Read<int8_t>();

		p.Read<int32_t>(); // Added around .62 (777 ?)
		p.Read<int8_t>(); // Buddylist length
		if (Network::Locale == Locales::Global && Network::Version > 102) {
			p.Read<int16_t>(); // 0?
			p.Read<int32_t>(); // Datetime? (2011121802 | 2011-12-18 02)
			
			p.Read<int64_t>(); // 0?
			p.Read<int64_t>(); // 0?
			p.Read<int32_t>(); // 0?
			p.Read<int64_t>(); // 2?
			p.Read<int64_t>(); // 0?
			p.Read<int64_t>(); // UNK RANDOM

			p.Read<int32_t>(); // ?
			p.Read<int16_t>(); // 
		}
		
		cout << "1| Pos: " << p.pos << endl;
		lp->gender = p.Read<int8_t>();
		lp->skin = p.Read<int8_t>();
		lp->face = p.Read<int32_t>();
		p.Read<int32_t>(); // 100?
		p.Read<int8_t>(); // hairslot
		lp->hair = p.Read<int32_t>();
		
		cout << "2| Pos: " << p.pos << endl;
		// Shown
		while (true) {
			int8_t slot = p.Read<int8_t>();
			if (slot == -1) break;
			int itemid = p.Read<int>(); // ItemID
			cout << "Itemid: " << itemid << " Slot: " << (int) slot << endl;
			lp->SetItemBySlot(slot, itemid);
		}
		cout << "3| Pos: " << p.pos << endl;
		// Hidden
		while (true) {
			int8_t slot = p.Read<int8_t>();
			if (slot == -1) break;
			p.Read<int32_t>(); // ItemID
		}
		p.Read<int32_t>(); // Special Cash Weapon

		p.Read<int32_t>(); // Pet IDs
		p.Read<int32_t>();
		p.Read<int32_t>();
		if (Network::Locale == Locales::Global && Network::Version > 81) {
			p.Read<int16_t>();
		}
		cout << "4| Pos: " << p.pos << endl;

		bool rank = p.Read<bool>();
		if (rank) {
			p.Read<int32_t>(); // 
			p.Read<int32_t>();
			p.Read<int32_t>();
			p.Read<int32_t>();
		}
		lp->nametag.Set(lp->name, NameTag::Normal);
		CharSelectScreen->worldCharacters.push_back(lp);
	}
	View::LoginStage(2); // Char select.
}

void NLS::Handle::ChangeMap(Packet &p) {
	int32_t channel = p.Read<int32_t>();
	uint8_t portalCount = p.Read<uint8_t>();
	ThisPlayer->currentPortal = portalCount;
	bool channelConnect = p.Read<bool>();
	// Special stuff begin
	int16_t topMsgs = p.Read<int16_t>();
	if (topMsgs > 0) {
		p.Read<string>(); // Title
		for (auto i = 0; i < topMsgs; i++) {
			p.Read<string>(); // Lines
		}
	}
	// Special stuff end

	if (channelConnect) {
		// RNG's
		uint32_t s1 = p.Read<uint32_t>();
		uint32_t s2 = p.Read<uint32_t>();
		uint32_t s3 = p.Read<uint32_t>();
		ThisPlayer->damageRNG.reset(s1, s2, s3);

		// Flag. Just ignore and load everything xd
		// Info: real client actually uses this!!!
		p.Read<uint64_t>(); // Most likely FF FF FF FF FF FF FF FF for every flag.

		ThisPlayer->charid = p.Read<uint32_t>();
		ThisPlayer->name = p.ReadStringLen(13);
		ThisPlayer->gender = p.Read<int8_t>();
		ThisPlayer->skin = p.Read<int8_t>();
		ThisPlayer->face = p.Read<int32_t>();
		ThisPlayer->hair = p.Read<int32_t>();

		p.Read<int64_t>(); // Taken out Pet ID's
		p.Read<int64_t>();
		p.Read<int64_t>();

		ThisPlayer->level = p.Read<uint8_t>();

		Player::Stats stats;
		stats.Job = p.Read<int16_t>(); // job
		stats.Str = p.Read<int16_t>(); // Strength
		stats.Dex = p.Read<int16_t>(); // Dex
		stats.Int = p.Read<int16_t>(); // Int
		stats.Luk = p.Read<int16_t>(); // Luk
		if (Network::Locale == Locales::Global && Network::Version < 81) {
			stats.HP = p.Read<int16_t>(); // HP
			stats.MaxHP = p.Read<int16_t>(); // MaxHP
			stats.MP = p.Read<int16_t>(); // MP
			stats.MaxMP = p.Read<int16_t>(); // MaxMP
		}
		else {
			stats.HP = p.Read<int32_t>(); // HP
			stats.MaxHP = p.Read<int32_t>(); // MaxHP
			stats.MP = p.Read<int32_t>(); // MP
			stats.MaxMP = p.Read<int32_t>(); // MaxMP
		}
		stats.AP = p.Read<int16_t>(); // AP
		if (is_extendsp_job(stats.Job)) {
			int8_t spslots = p.Read<int8_t>();
			for (auto i = 0; i < spslots; i++) {
				int8_t slot = p.Read<int8_t>();
				int8_t amount = p.Read<int8_t>();
				if (slot < stats.SP.size()) {
					stats.SP[slot] = amount;
				}
			}
		}
		else {
			stats.SP[0] = p.Read<int16_t>();
		}
		stats.EXP = p.Read<int32_t>(); // EXP
		stats.Fame = p.Read<int16_t>(); // Fame
		stats.GachaEXP = p.Read<int32_t>(); // GachaEXP
		ThisPlayer->stats = stats;

		int32_t mapid = p.Read<int32_t>();
		int8_t mappos = p.Read<int8_t>();

		p.Read<int32_t>(); // Added around .62
		p.Read<int8_t>(); // Buddylist length

		Player::Inventory inventory;
		inventory.mesos = p.Read<int32_t>();
		
		for (int8_t i = 0; i < 5; i++) {
			inventory.inventorySlots[i] = p.Read<int8_t>();
		}


		while (true) {
			int8_t slot = p.Read<int8_t>();
			if (slot == 0) break;
			auto item = DecodeItem(p);
			inventory.shownEquips[slot] = item;
			ThisPlayer->SetItemBySlot(slot, item->m_id);
		}

		while (true) {
			int8_t slot = p.Read<int8_t>();
			if (slot == 0) break;
			auto item = DecodeItem(p);
			ThisPlayer->inventory.hiddenEquips[slot] = item;
		}

		while (true) {
			int8_t slot = p.Read<int8_t>();
			if (slot == 0) break;
			auto item = DecodeItem(p);
			//ThisPlayer->inventory.hiddenEquips[slot] = item;
		}

		for (auto i = 0; i < 5; i++) {
			while (true) {
				int8_t slot = p.Read<int8_t>();
				if (slot == 0) break;
				auto item = DecodeItem(p);
				ThisPlayer->inventory.inventoryItems[i][slot] = item;
			}
		}

		ThisPlayer->inventory = inventory; // For DecodeItem.

		ThisPlayer->nametag.Set(ThisPlayer->name, NameTag::Normal);
		Map::nextmap = tostring(mapid);
		Map::nextportalID = mappos;
	}
	else {
		int32_t mapid = p.Read<int32_t>();
		int8_t mappos = p.Read<int8_t>();
		ThisPlayer->stats.HP = p.Read<int16_t>();
		p.Read<int8_t>(); // unk
		p.Read<int64_t>(); // Filetime time time
		Map::nextmap = tostring(mapid);
		Map::nextportalID = mappos;
	}
	Map::Load();
}

void NLS::Handle::PlayerSpawn(Packet &p) {
	// Spawn player
	uint32_t id = p.Read<uint32_t>();
	if (Map::Players.find(id) != Map::Players.end()) return; // We don't want to spawn someone twice!

	Player *player = new Player;
	player->charid = id;
	player->name = p.Read<string>();
	player->guildname = p.Read<string>();
	
	p.Read<int16_t>(); // Guild Emblem BG
	p.Read<int8_t>(); // Guild Emblem BG Color
	p.Read<int16_t>(); // Guild Emblem
	p.Read<int8_t>(); // Guild Emblem Color


	// OK THIS IS MADNESS. BRB GUYS

	// ok.. there we go. Started 1-12-2011
	using namespace NLS::BuffValueTypes;

	uint32_t buffFlags = p.Read<uint32_t>();
	{
		//using namespace NLS::BuffValueTypes::ValuesInt4;
		// Not yet used... lol
	}
	buffFlags = p.Read<uint32_t>();
	{
		using namespace NLS::BuffValueTypes::ValuesInt3;
		
	}

	buffFlags = p.Read<uint32_t>();
	{
		using namespace NLS::BuffValueTypes::ValuesInt2;
		
	}

	buffFlags = p.Read<uint32_t>();
	{
		using namespace NLS::BuffValueTypes::ValuesInt1;
		if (buffFlags & GetFlagFromBit(Speed)) {
			p.Read<int8_t>(); // Value
		}
		if (buffFlags & GetFlagFromBit(ComboAttack)) {
			p.Read<int8_t>(); // Value
		}
		if (buffFlags & GetFlagFromBit(Charges)) {
			p.Read<int32_t>(); // SkillID
		}
		if (buffFlags & GetFlagFromBit(Stun)) {
			p.Read<int32_t>(); // SkillID
		}
		if (buffFlags & GetFlagFromBit(Darkness)) {
			p.Read<int32_t>(); // SkillID
		}
		if (buffFlags & GetFlagFromBit(Seal)) {
			p.Read<int32_t>(); // SkillID
		}
		if (buffFlags & GetFlagFromBit(Weakness)) {
			p.Read<int32_t>(); // SkillID
		}
		if (buffFlags & GetFlagFromBit(Curse)) {
			p.Read<int32_t>(); // SkillID
		}
		if (buffFlags & GetFlagFromBit(Poison)) {
			p.Read<int16_t>(); // 'value'?
			p.Read<int32_t>(); // SkillID
		}
		if (buffFlags & GetFlagFromBit(ShadowPartner)) {
			
		}
		if (buffFlags & GetFlagFromBit(DarkSight)) {
			
		}
		if (buffFlags & GetFlagFromBit(SoulArrow)) {
			
		}
	}
	
	p.Read<int32_t>();
	p.Read<int32_t>();
	p.Read<int16_t>();
	p.Read<int32_t>(); // Static value?
	
	p.Read<int16_t>();
	p.Read<int8_t>();
	p.Read<int32_t>();
	p.Read<int32_t>();
	p.Read<int32_t>(); // Static value?
	
	p.Read<int16_t>();
	p.Read<int8_t>();
	p.Read<int32_t>();
	p.Read<int32_t>();
	p.Read<int32_t>(); // Static value?
	
	p.Read<int16_t>();
	p.Read<int8_t>();
	p.Read<int32_t>(); // Mount ID
	p.Read<int32_t>(); // Mount Skill
	p.Read<int32_t>(); // Static value?

	p.Read<int8_t>();
	p.Read<int32_t>();
	p.Read<int32_t>();
	p.Read<int32_t>(); // Static value?
	
	p.Read<int32_t>();
	p.Read<int32_t>();
	p.Read<int32_t>();
	p.Read<int32_t>();
	p.Read<int32_t>(); // Static value?

	p.Read<int8_t>();
	p.Read<int32_t>();
	p.Read<int32_t>();
	p.Read<int32_t>();
	p.Read<int32_t>(); // Static value?
	
	p.Read<int16_t>();
	p.Read<int8_t>();

	
	// PLAYER INFO. FINALLY
	{
		p.Read<int16_t>(); // Player Job
		player->gender = p.Read<int8_t>();
		player->skin = p.Read<int8_t>();
		player->face = p.Read<int32_t>();
		p.Read<int8_t>(); // Actually slot 0 for hair!!!
		player->hair = p.Read<int32_t>();
		// Shown
		while (true) {
			int8_t slot = p.Read<int8_t>();
			if (slot == -1) break;
			int32_t itemid = p.Read<int32_t>(); // ItemID
			player->SetItemBySlot(slot, itemid);
		}
		// Hidden
		while (true) {
			int8_t slot = p.Read<int8_t>();
			if (slot == -1) break;
			p.Read<int32_t>(); // ItemID
		}
		p.Read<int32_t>(); // Special Cash Weapon

		p.Read<int32_t>(); // Pet IDs
		p.Read<int32_t>();
		p.Read<int32_t>();
	}

	p.Read<int32_t>();
	p.Read<int32_t>(); // Item Effect
	p.Read<int32_t>(); // Chair
	player->x = p.Read<int16_t>();
	player->y = p.Read<int16_t>();
	int8_t stance = p.Read<int8_t>();
	player->state = player->StanceToString(stance);
	player->f = !(stance % 2);
	int16_t foothold = p.Read<int16_t>();
	auto it = find_if(Foothold::begin(), Foothold::end(), [&](Foothold* fh){return fh->id == foothold;});
	if (it != Foothold::end()) {
		player->fh = *it;
		player->r = pdis(player->x, player->y, player->fh->x1, player->fh->y1);
	} else {
		player->fh = nullptr;
	}
	//Ladders? Velocity?

	// More to come...

	player->guildtag.Set(player->guildname, NameTag::Normal);
	player->nametag.Set(player->name, NameTag::Normal);
	Map::Players[id] = player;
}

void NLS::Handle::PlayerDespawn(Packet &p) {
	// Spawn player
	uint32_t id = p.Read<uint32_t>();
	Player *player = Map::GetPlayer(id);
	if (player == nullptr) return;
	Map::Players.erase(id);
	delete player;
	// TODO: delete other stuff too
}

void NLS::Handle::PlayerChat(Packet &p) {
	uint32_t id = p.Read<uint32_t>();
	Player *player = Map::GetPlayer(id);
	if (player == nullptr) return;
	bool gmmsg = p.Read<bool>();
	string msg = p.Read<string>();
	bool shout = p.Read<bool>();
	player->balloon.Set(player->name + " : " + msg, "1");
	player->balloonRun = 500;
}

void NLS::Handle::PlayerMove(Packet &p) {
	uint32_t id = p.Read<uint32_t>();
	Player *player = Map::GetPlayer(id);
	if (player == nullptr) return;
	DecodeMovement(p, player);
}

void NLS::Handle::PlayerEmote(Packet &p) {
	uint32_t id = p.Read<uint32_t>();
	Player *player = Map::GetPlayer(id);
	if (player == nullptr) return;

	int32_t emoteID = p.Read<int32_t>();
	player->emote = player->GetEmoteNameByID(emoteID);
}

void NLS::Handle::DecodeMovement(Packet &p, Physics*player) {
	p.Read<int32_t>(); // Unknown, should be X and Y...?

	uint8_t actions = p.Read<uint8_t>();
	for (auto i = 0; i < actions; i++) {
		uint8_t type = p.Read<uint8_t>();
		Movement move(type);
		if (move.type == 0 || move.type == 5) {
			move.x = p.Read<int16_t>();
			move.y = p.Read<int16_t>();
			move.vx = p.Read<int16_t>();
			move.vy = p.Read<int16_t>();
			move.fh = p.Read<int16_t>();
			move.stance = p.Read<int8_t>();
			move.t = p.Read<int16_t>();
		}
		else if (move.type == 1 || move.type == 2 || move.type == 6 || move.type == 13) {
			move.vx = p.Read<int16_t>();
			move.vy = p.Read<int16_t>();
			move.stance = p.Read<int8_t>();
			move.fh = p.Read<int16_t>();
		}
		else if (move.type == 15) {
			move.x = p.Read<int16_t>();
			move.y = p.Read<int16_t>();
			move.vx = p.Read<int16_t>();
			move.vy = p.Read<int16_t>();
			p.Read<int16_t>(); // TODO find out
			move.fh = p.Read<int16_t>();
			move.stance = p.Read<int8_t>();
			move.t = p.Read<int16_t>();
		}
		else if (move.type == 11) {
			move.x = p.Read<int16_t>();
			move.y = p.Read<int16_t>();
			move.fh = p.Read<int16_t>();
			move.stance = p.Read<int8_t>();
			move.t = p.Read<int16_t>(); // Not sure
		}
		else if (move.type == 10) {
			p.Read<int8_t>(); // TODO find out
			continue;
		}
		else if (move.type == 16) {
			move.x = p.Read<int16_t>();
			move.y = p.Read<int16_t>();
			move.fh = p.Read<int16_t>();
			move.stance = p.Read<int8_t>(); // I guess
		}
		else if (move.type == 17) {
			move.x = p.Read<int16_t>();
			move.y = p.Read<int16_t>();
			move.fh = p.Read<int16_t>();
			move.stance = p.Read<int8_t>();
			p.Read<int32_t>();
			move.t = p.Read<int16_t>();
		}
		else if (move.type == 12) {
			move.x = p.Read<int16_t>();
			move.y = p.Read<int16_t>();
			move.fh = p.Read<int16_t>();
			move.t = p.Read<int16_t>(); // NOT SURE
		}
		else if (move.type == 14) { // NOT SURE
			move.x = p.Read<int16_t>();
			move.y = p.Read<int16_t>();
			move.vx = p.Read<int16_t>();
			move.vy = p.Read<int16_t>();
			move.t = p.Read<int16_t>();
		}
		else if (move.type == 3 || move.type == 4 || (move.type >= 7 && move.type <= 9)) { // NOT SURE
			move.x = p.Read<int16_t>();
			move.y = p.Read<int16_t>();
			move.vx = p.Read<int16_t>();
			move.vy = p.Read<int16_t>();
			move.stance = p.Read<int8_t>();
		}
		player->moves.push_back(move);
	}
}

NLS::Item * NLS::Handle::DecodeItem(Packet &p) {
	Item *item = new Item();
	int8_t type = p.Read<int8_t>();
	int32_t itemid = p.Read<int32_t>();
	item->m_id = itemid;
	bool isCash = p.Read<bool>();
	if (isCash) {
		item->m_cashId = p.Read<int64_t>();
	}
	else {
		item->m_cashId = 0;
	}
	item->m_expiration = p.Read<int64_t>();

	if (type == Items::PacketTypes::Item) {
		item->m_amount = p.Read<int16_t>();
		item->m_name = p.Read<string>();
		item->m_flags = p.Read<int16_t>();
		// Rechargable load
		int32_t i = item->m_id / 10000;
		if (i == 207 || i == 233) {
			p.Read<int64_t>();
		}
	}
	else if (type == Items::PacketTypes::Equip) {
		item->m_slots = p.Read<int8_t>();
		item->m_scrolls = p.Read<int8_t>();
		item->m_str = p.Read<int16_t>();
		item->m_dex = p.Read<int16_t>();
		item->m_int = p.Read<int16_t>();
		item->m_luk = p.Read<int16_t>();
		item->m_hp = p.Read<int16_t>();
		item->m_mp = p.Read<int16_t>();
		item->m_watk = p.Read<int16_t>();
		item->m_matk = p.Read<int16_t>();
		item->m_wdef = p.Read<int16_t>();
		item->m_mdef = p.Read<int16_t>();
		item->m_acc = p.Read<int16_t>();
		item->m_avo = p.Read<int16_t>();
		item->m_hands = p.Read<int16_t>();
		item->m_speed = p.Read<int16_t>();
		item->m_jump = p.Read<int16_t>();
		item->m_name = p.Read<string>();
		item->m_flags = p.Read<int16_t>();
		if (item->m_cashId != 0) {
			p.Read<int32_t>();
			p.Read<int16_t>();
			p.Read<int32_t>();
		}
		else {
			p.Read<int8_t>();
			p.Read<int8_t>();
			p.Read<int16_t>();
			p.Read<int16_t>();
			item->m_hammers = p.Read<int32_t>();
			p.Read<int64_t>();
		}
		p.Read<int64_t>();
		p.Read<int32_t>();
	}
	else if (type == Items::PacketTypes::Pet) {

	}
	return item;
}

void NLS::Handle::MobSpawn(Packet &packet) {
	uint32_t oid = packet.Read<uint32_t>();
	if (Life::Mobs.find(oid) != Life::Mobs.end()) return;
}

void NLS::Handle::NpcSpawn(Packet &packet) {
	uint32_t oid = packet.Read<uint32_t>();
	if (Life::Npcs.find(oid) != Life::Npcs.end()) return;

	Npc *npc = new Npc;
	npc->id = tostring(packet.Read<int32_t>());
	npc->Init();
	npc->x = npc->cx = packet.Read<int16_t>();
	npc->y = npc->cy = packet.Read<int16_t>() - 5;
	npc->f = !packet.Read<bool>();
	int16_t foothold = packet.Read<int16_t>();
	auto it = find_if(Foothold::begin(), Foothold::end(), [&](Foothold* fh){return fh->id == foothold;});
	if (it != Foothold::end()) {
		npc->fh = *it;
		npc->r = pdis(npc->x, npc->y, npc->fh->x1, npc->fh->y1);
	} else {
		npc->fh = nullptr;
	}
	npc->rx0 = packet.Read<int16_t>();
	npc->rx1 = packet.Read<int16_t>();
	npc->time = 0;
	npc->Reset(npc->x, npc->y);
	Life::Npcs[Life::NpcStart++] = npc;
}

void NLS::Handle::SkippedObjects(Packet &packet) {
	// contains a list of skipped objects.
	int8_t amount = packet.Read<int8_t>();
	for (auto i = 0; i < amount; i++) {
		string value = packet.Read<string>();
		Obj::skipTags.push_back(value);
	}
}

void NLS::Send::Pong() {
	Packet packet(SendHeaders[PacketType::Pong]);
	packet.Send();
}

void NLS::Send::Pang() {
	// Loading special character for local testserver.
	//NLS::Packet packet(0x14);
	//packet.Write<int32_t>(Config::LoadCharid); // Special Character!
	//packet.Send();
	//NLS::Packet packet(0x18);
	//packet.Send();
}

void NLS::Send::RequestWorlds(bool back) {
	for (auto iter = WorldSelectScreen->worlds.begin(); iter != WorldSelectScreen->worlds.end(); iter++) {
		delete *iter;
	}
	WorldSelectScreen->worlds.clear();
	Packet p(SendHeaders[back ? PacketType::RequestWorldBack : PacketType::RequestWorld]);
	p.Send();
}

void NLS::Send::WorldSelectRequest() {
	Packet p(SendHeaders[PacketType::WorldSelectRequest]);
	p.Write<int16_t>(WorldSelectScreen->selectedWorld);
	p.Send();
}

void NLS::Send::ChannelSelectRequest() {
	cout << "Channel Select. World: " << (int16_t)WorldSelectScreen->selectedWorld << " Channel: " << (int16_t)WorldSelectScreen->ChannelSelectWindow->selectedChannel << endl;
	Packet p(SendHeaders[PacketType::ChannelSelectRequest]);
	p.Write<int8_t>(2);
	p.Write<int8_t>(WorldSelectScreen->selectedWorld);
	p.Write<int8_t>(WorldSelectScreen->ChannelSelectWindow->selectedChannel);
	p.Write<uint16_t>(43200); // ???
	p.Write<int8_t>(0); // ???
	p.Write<int8_t>(11); // ???
	p.Send();
}

void NLS::Send::Handshake() {
	if (Network::Locale == 0x08 && Network::Version <= 100) return; // GMS V.100 and lower didn't have this.
	if (Network::Locale == 0x07 && Network::Version <= 111) return; // MSEA V.111 and lower didn't have this. |NOTSURE|

	uint16_t subversion = toint(Network::Patch);
	uint16_t header = 0;
	if (Network::Locale == Locales::Global) {
		if (Network::Version >= 101) header = 0x14;
	}
	else if (Network::Locale == Locales::Sea) {
		if (Network::Version >= 112) header = 0x01; // No shit!
	}

	Packet packet(header);
	packet.Write<uint8_t>(Network::Locale);
	packet.Write<uint16_t>(Network::Version);
	packet.Write<uint16_t>(subversion);
	packet.Send();
}

void NLS::Send::Login(const string &username, const string &password) {
	Packet packet(SendHeaders[PacketType::Login]);

	// TODO
	if (Network::Locale == Locales::Global) {
		if (Network::Version <= 88) {
			packet.Write<string>(username);
			packet.Write<string>(password);
			// Machine ID stuff. Just put it here :)
			packet.Write<int32_t>(0);
			packet.Write<int16_t>(0);
			packet.Write<int64_t>(2060196618);
			packet.Write<int32_t>(64976);
			packet.Write<int16_t>(0);
			packet.Write<int16_t>(2);
			packet.Write<int32_t>(0);
		}
		else {
			auto responses = Network::RequestLogin(username, password);

			if (responses.find("Set-Cookie") != responses.end()) {
				string device_id, npp, session, authToken;
				auto cookies = responses["Set-Cookie"];
				for (vector<string>::iterator iter = cookies.begin(); iter != cookies.end(); iter++) {
					string cookieData = *iter, 
						key = cookieData.substr(0, cookieData.find_first_of('=')),
						value = cookieData.substr(cookieData.find_first_of('=') + 1);
					value = value.substr(0, value.find_first_of(';'));
					if (key == "device_id") device_id = value;
					else if (key == "NPP") npp = value;
					else if (key == "session") session = value;
					else if (key == "authToken") authToken = value;
				}

				cout << "Device ID: " << device_id << endl;
				cout << "NPP: " << npp << endl;
				cout << "Session: " << session << endl;
				cout << "AuthToken: " << authToken << endl;
				cout << responses["Content"][0] << endl;
				packet.Write<string>(password);
				packet.Write<string>(npp);
				packet.Write<int32_t>(0);
				packet.Write<int16_t>(0);
				packet.Write<int64_t>(2060196618);
				packet.Write<int32_t>(64976);
				packet.Write<int16_t>(0);
				packet.Write<int16_t>(2);
				packet.Write<int32_t>(0);
				packet.Write<int16_t>(256);
				packet.Write<string>(""); // Base64 shit, w/ client start time at the beginning in GMT form: "�m42011 12 11 14:32"
				packet.Write<string>("2.80.1000"); // I have no fucking idea.
			}
			else {
				cout << "Failed logging in on website!" << endl;
				return;
			}
		}
	}
	packet.Send();
}

void NLS::Send::PlayerEmote(int32_t emote) {
	NLS::Packet packet(SendHeaders[PacketType::PlayerEmote]);
	packet.Write<int32_t>(emote);
	packet.Send();
}

void NLS::Send::UsePortal(const string &portalname) {
	NLS::Packet packet(SendHeaders[PacketType::UsePortal]);
	packet.Write<uint8_t>(ThisPlayer->currentPortal);
	packet.Write<int32_t>(-1);
	packet.Write<string>(portalname);
	packet.Send();
}

void NLS::Send::UsePortalScripted(const string &portalname) {
	NLS::Packet packet(SendHeaders[PacketType::UsePortalScripted]);
	packet.Write<uint8_t>(ThisPlayer->currentPortal);
	packet.Write<string>(portalname);
	packet.Send();
}

void NLS::Send::Revive() {
	NLS::Packet packet(SendHeaders[PacketType::UsePortal]);
	packet.Write<uint8_t>(ThisPlayer->currentPortal);
	packet.Write<int32_t>(0);
	packet.Write<string>("");
	packet.Write<int8_t>(0);
	packet.Write<bool>(false);
	packet.Send();
}

void NLS::Send::GmMapTeleport(int32_t mapid) {
	NLS::Packet packet(SendHeaders[PacketType::UsePortal]);
	packet.Write<uint8_t>(ThisPlayer->currentPortal);
	packet.Write<int32_t>(mapid);
	packet.Send();
}

void NLS::Send::NpcChatStart(int32_t npcid) {
	NLS::Packet packet(SendHeaders[PacketType::NpcRequestTalk]);
	packet.Write<uint8_t>(ThisPlayer->currentPortal);
	packet.Write<int32_t>(npcid);
	packet.Send();
}

void NLS::Send::Chat(const string &msg, bool shout) {
	Packet packet(SendHeaders[PacketType::PlayerChat]);
	packet.Write<string>(msg);
	packet.Write<bool>(shout);
	packet.Send();
}

void NLS::Send::PlayerMove() {
	NLS::Packet p(SendHeaders[PacketType::PlayerMove]);
	p.Write<int8_t>(ThisPlayer->currentPortal);
	p.Write<int32_t>(0); // Move X and Y??
	p.Write<int16_t>(ThisPlayer->x);
	p.Write<int16_t>(ThisPlayer->y);
	p.Write<int8_t>(ThisPlayer->moves.size());

	for (auto i = 0; i < ThisPlayer->moves.size(); i++) {
		Movement move = ThisPlayer->moves[i];
		p.Write<int8_t>(move.type);
		if (move.type == 0 || move.type == 5) {
			p.Write<int16_t>(move.x);
			p.Write<int16_t>(move.y);
			p.Write<int16_t>(move.vx);
			p.Write<int16_t>(move.vy);
			p.Write<int16_t>(move.fh);
			p.Write<int8_t>(move.stance);
			p.Write<int16_t>(move.t);
		}
		else if (move.type == 1 || move.type == 2 || move.type == 6 || move.type == 13) {
			p.Write<int16_t>(move.x);
			p.Write<int16_t>(move.y);
			p.Write<int8_t>(move.stance);
			p.Write<int16_t>(move.fh);
		}
		else if (move.type == 15) {
			p.Write<int16_t>(move.x);
			p.Write<int16_t>(move.y);
			p.Write<int16_t>(move.vx);
			p.Write<int16_t>(move.vy);
			p.Write<int16_t>(0); // TODO find out
			p.Write<int16_t>(move.fh);
			p.Write<int8_t>(move.stance);
			p.Write<int16_t>(move.t);
		}
		else if (move.type == 11) {
			p.Write<int16_t>(move.x);
			p.Write<int16_t>(move.y);
			p.Write<int16_t>(move.fh);
			p.Write<int8_t>(move.stance);
			p.Write<int16_t>(move.t); // Not sure
		}
		else if (move.type == 10) {
			p.Write<int8_t>(0); // TODO find out
		}
		else if (move.type == 16) {
			p.Write<int16_t>(move.x);
			p.Write<int16_t>(move.y);
			p.Write<int16_t>(move.fh);
			p.Write<int8_t>(move.stance); // I guess
		}
		else if (move.type == 17) {
			p.Write<int16_t>(move.x);
			p.Write<int16_t>(move.y);
			p.Write<int16_t>(move.fh);
			p.Write<int8_t>(move.stance);
			p.Write<int32_t>(0);
			p.Write<int16_t>(move.t);
		}
		else if (move.type == 12) {
			p.Write<int16_t>(move.x);
			p.Write<int16_t>(move.y);
			p.Write<int16_t>(move.fh);
			p.Write<int16_t>(move.t); // NOT SURE
		}
		else if (move.type == 14) { // NOT SURE
			p.Write<int16_t>(move.x);
			p.Write<int16_t>(move.y);
			p.Write<int16_t>(move.vx);
			p.Write<int16_t>(move.vy);
			p.Write<int16_t>(move.t);
		}
		else if (move.type == 3 || move.type == 4 || (move.type >= 7 && move.type <= 9)) { // NOT SURE
			p.Write<int16_t>(move.x);
			p.Write<int16_t>(move.y);
			p.Write<int16_t>(move.vx);
			p.Write<int16_t>(move.vy);
			p.Write<int8_t>(move.stance);
		}
	}
	p.Write<int64_t>(0);
	p.Write<int64_t>(0);
	p.Write<int64_t>(0);
	p.Send();
	ThisPlayer->moves.clear();
}
#pragma endregion
