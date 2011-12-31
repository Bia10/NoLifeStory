////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
namespace NLS {
	class Packet {
	public:
		vector<uint8_t> data;
		size_t pos;
		Packet() : pos(0), data() {}
		Packet(uint16_t opcode) : pos(0), data() {
			Write<int32_t>(0);
			Write<uint16_t>(opcode);
		}
		Packet(uint8_t* data, uint16_t len) : pos(0), data(data, data+len) {}
		void Send();
		void Encrypt();
		void Decrypt();
		string ToString()  {
			std::stringstream out;
			for (int i = 0; i < data.size(); ++i) {
				out << hex << uppercase << setw(2) << setfill('0') << (uint16_t)data[i];
				out << ' ';
			}
			return out.str();
		}
		template <class T>
		T Read() {
			T& ret = *(T*)&data[pos];
			pos += sizeof(T);
			return ret;
		}

		string ReadStringLen(size_t size) {
			string s((char*)&data[pos], size);
			size_t nullstart = s.find_first_of('\0');
			if (nullstart != string::npos) {
				s = s.substr(0, nullstart);
			}
			pos += size;
			return s;
		}
		template <class T>
		void Write(T v) {
			data.insert(data.end(), (uint8_t*)&v, (uint8_t*)&v+sizeof(T));
		}
		static map<uint16_t, function<void(Packet&)>> Handlers;
	};
	template <>
	inline string Packet::Read<string>() {
		size_t size = Read<uint16_t>();
		string s((char*)&data[pos], size);
		pos += size;
		return s;
	}
	template <>
	inline bool Packet::Read<bool>() {
		return Read<uint8_t>();
	}
	template <>
	inline void Packet::Write<string>(string s) {
		Write<uint16_t>(s.size());
		data.insert(data.end(), s.begin(), s.end());
	}
	template <>
	inline void Packet::Write<bool>(bool v) {
		Write<uint8_t>(v);
	}
	namespace Handle {
		void Init();
		void Login(Packet&);
		void WorldListResult(Packet&);
		void WorldSelectResult(Packet&);
		void WorldCharacters(Packet&);
		void Ping(Packet&);
		void ChangeMap(Packet&);
		void PlayerSpawn(Packet&);
		void PlayerDespawn(Packet&);
		void PlayerChat(Packet&);
		void PlayerMove(Packet&);
		void PlayerEmote(Packet&);
		void DecodeMovement(Packet&, Physics*);
		Item * DecodeItem(Packet&);
		void MobSpawn(Packet&);
		void NpcSpawn(Packet&);
		void SkippedObjects(Packet&);
	}
	namespace Send {
		void Pong();
		void Pang();
		void RequestWorlds(bool);
		void WorldSelectRequest();
		void ChannelSelectRequest();
		void Handshake();
		void Login(const string &, const string &);
		void PlayerMove();
		void PlayerEmote(int32_t);
		void UsePortal(const string &);
		void UsePortalScripted(const string &);
		void Revive();
		void GmMapTeleport(int32_t);
		void NpcChatStart(int32_t);
		void Chat(const string &, bool shout = false);
		namespace PacketType {
			enum Header {
				Login,
				HSStartup,
				Version,
				RequestWorld,
				RequestWorldBack,
				WorldSelectRequest,
				ChannelSelectRequest,
				Pong,
				LoadCharacter,
				PlayerMove,
				PlayerEmote,
				PlayerChat,
				NpcRequestTalk,
				UsePortal,
				UsePortalScripted,
			};
		}
		extern map<PacketType::Header, uint16_t> SendHeaders;
	}
}
