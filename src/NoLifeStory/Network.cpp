////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

bool NLS::Network::Online = true;
bool NLS::Network::Connected = false;
bool NLS::Network::IsLogin = true;
uint16_t NLS::Network::Version;
string NLS::Network::Patch;
uint8_t NLS::Network::SendIV[4];
uint8_t NLS::Network::RecvIV[4];
uint8_t NLS::Network::Locale;
string NLS::Network::IP;
uint16_t NLS::Network::Port;
vector<NLS::Packet> ToSend;
sf::TcpSocket NLS::Network::Socket;

void SendHandshakeOK();

void NLS::Network::Init() {
	Socket.SetBlocking(false);
	Connected = false;
}

void NLS::Network::Loop() {
	static Packet p;
	static bool ghead = true;
	static bool initial = true;
	static bool connecting = false;
	static size_t pos = 0;
	static uint16_t len = 0;
	static uint8_t header[4];
	static uint8_t data[0x10000];
	static double timeout = 0;
	auto Receive = [&](uint8_t* data, size_t len) -> bool{
		size_t received = 0;
		sf::Socket::Status err = Socket.Receive((char *)data+pos, len-pos, received);
		pos += received;
		switch (err) {
		case sf::Socket::Disconnected:
			if (!connecting) {
				cerr << "Disconnected from the server" << endl;
				Connected = false;
				Online = false;
				//TODO - Pop up message saying they got disconnected and ask if they want to play offline, or login again.
				Map::Load("0", "sp");
			} else {
				cerr << "Failed to connect to the server" << endl;
#ifdef _WIN32
				cerr << "WSAGetLastError(): " << WSAGetLastError() << endl;
#endif
				Connected = false;
				Online = false;
				//TODO - Pop up message saying unable to connect and ask if they want to play offline, or retry to connect.
				Map::Load("0", "sp");
			}
			return false;
		case sf::Socket::Error:
			cerr << "Network error occured " << endl;
			Connected = false;
			Online = false;
			Map::Load("0", "sp");
			return false;
		case sf::Socket::NotReady:
			if (connecting and timeout > 5000) {
				cerr << "Connection to the server timed out" << endl;
				Connected = false;
				Online = false;
				//TODO - Pop up message saying unable to connect and ask if they want to play offline, or retry to connect.
				Map::Load("0", "sp");
			}
			return false;
		case sf::Socket::Done:
			connecting = false;
			return pos == len;
		default:
			cerr << "Wat wat" << endl;
			return false;
		}
	};
	if (!Online) return;
	if (!Connected and !connecting) {
		cout << "Trying to connect to " << IP << ":" << Port << endl;
		Socket.SetBlocking(true);
		if (Socket.Connect(IP, Port, 2000) == sf::Socket::Done) {
			cout << "Connected to server at " << IP << ":" << Port << endl;
			Socket.SetBlocking(false);
			Connected = true;
			initial = true;
			ghead = true;
			pos = 0;
		}
		else {
			cout << "Could not connect. Switching to offline mode." << endl;
			Online = false;
			Map::Load("0", "sp");
		}
	}
	timeout += Time::delta;
	if (!Connected) return;
	while (Connected) {
		if (initial) {
			if (ghead) {
				if (!Receive(header, 2)) return;
				len = *(uint16_t*)header;
				ghead = false;
				pos = 0;
			} else {
				if (!Receive(data, len)) return;
				Packet p(data, len);
				Version = p.Read<uint16_t>();
				Patch = p.Read<string>();
				uint32_t siv = p.Read<uint32_t>();
				uint32_t riv = p.Read<uint32_t>();
				Locale = p.Read<uint8_t>();
				cout << "Server version: " << Version << endl;
				cout << "Minor version: " << Patch << endl;
				cout << "Locale: " << (uint16_t)Locale << endl;
				cout << "SendIV: " << siv << endl;
				cout << "RecvIV: " << riv << endl;
				memcpy(SendIV, &siv, 4);
				memcpy(RecvIV, &riv, 4);
				Handle::Init();
				Send::Handshake();
				Send::Pang();
				initial = false;
				ghead = true;
				pos = 0;
			}
		} else {
			if (ghead) {
				if (!Receive(header, 4)) return;
				uint32_t llen = *(uint32_t*)header;
				len = (llen >> 16) ^ llen;
				ghead = false;
				pos = 0;
			} else {
				if (!Receive(data, len)) return;
				Packet p(data, len);
				p.Decrypt();
				uint16_t opcode = p.Read<uint16_t>();
				auto& f = p.Handlers[opcode];
				if (f) f(p);
				else {
					cerr << "No packet handler for opcode: " << hex << uppercase << setw(4) << setfill('0') << opcode << endl;
					cout << "Received Packet: " << p.ToString() << endl;
				}
				ghead = true;
				pos = 0;
			}
		}
	}
}

void NLS::Network::Connect(string IP, uint16_t port) {
	if (Connected)
		Socket.Disconnect();
	Socket.SetBlocking(false);
	Connected = false;
	Network::IP = IP;
	Network::Port = port;
}

void NLS::Network::Unload() {

}

map<string, vector<string>> NLS::Network::RequestLogin(const string &user, const string &pass) {
	sf::TcpSocket sock;
	map<string, vector<string>> ret;
	sock.SetBlocking(true);
	
	if (sock.Connect("www.nexon.net", 80, 2000) == sf::Socket::Done) {
		sock.SetBlocking(false);
		string content = "userID=" + user + "&password=" + pass;

		
		stringstream out;
		out << "POST " << "/api/v001/account/login" << " HTTP/1.0" << endl;
		out << "Host: www.nexon.net" << endl;
		out << "Content-Length: " << content.size() << endl;
		out << "Content-Type: application/x-www-form-urlencoded" << endl;
		out << "User-Agent: Diamondos-Network-Code-LOL/1.3.3.7" << endl;
		out << endl;
		out << content;
		string data = out.str();
		auto senderr = sock.Send(data.c_str(), data.size());
		
		sock.SetBlocking(true);

		// :D
		char *recvData = new char[2048];
		int curpos = 0, lastpos = 0;
		size_t received = 0;
		while (true) {
			auto ans = sock.Receive(recvData + curpos, (size_t)1, received);
			if (ans != sf::Socket::Done) {
				break;
			}
			if (curpos >= 2) {
				if (recvData[curpos - 1] == '\r' && recvData[curpos] == '\n') {
					int len = curpos - lastpos;
					// Got data. Now push it to the list
					data = string(recvData + lastpos, len - 1);
					if (data.substr(0, 4) != "HTTP") {
						size_t dubbelepunt = data.find_first_of(':', 0);
						if (dubbelepunt == string::npos) {
							// No data. Break and try to read content next.
							break;
						}
					
						ret[data.substr(0, dubbelepunt)].push_back(data.substr(dubbelepunt + 2));
					}
					lastpos = curpos + 1;
				}
			}
			curpos += received;
		}

		if (ret.find("Content-Length") != ret.end()) {
			int length = toint(ret["Content-Length"][0]);
			sock.Receive(recvData, (size_t)length, received);
			ret["Content"].push_back(string(recvData, length));
		}
	}
	return ret;
}