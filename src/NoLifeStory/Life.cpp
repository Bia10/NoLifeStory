////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

map <uint32_t, NLS::Mob *> NLS::Life::Mobs;
map <uint32_t, NLS::Npc *> NLS::Life::Npcs;

uint32_t NLS::Life::NpcStart = 100;
uint32_t NLS::Life::MobStart = 0;

void NLS::Life::Load() {
	NpcStart = 100;
	MobStart = 0;

	for (auto it = Mobs.begin(); it != Mobs.end(); it++) {
		delete it->second;
	}
	Mobs.clear();
	for (auto it = Npcs.begin(); it != Npcs.end(); it++) {
		delete it->second;
	}
	Npcs.clear();

	Node data = Map::node["life"];
	if (!data || Network::Connected) return;
	
	for (auto it = data.begin(); it != data.end(); it++) {
		Node rn = it->second;
		Life* r;
		string type = (string) rn["type"];
		if (type == "n")	{
			r = new Npc;
		}
		else if (type == "m") {
			r = new Mob;
		}
		else {
			cerr << "[WARN] Loading unknown 'life'! Map: " << Map::curmap << ", Life list ID: " << it->first << ", Type: " << r->type << endl;
			// Safe mode, Erwin style
			//  delete r;
			//  continue;
			// Rage mode, Peter style
			throw(273); // Goodbye cruel world.
		}
		r->id = (string)rn["id"];
		r->x = rn["x"];
		r->y = rn["y"];
		r->cx = rn["cx"];
		r->cy = rn["cy"];
		r->rx0 = rn["rx0"];
		r->rx1 = rn["ry1"];
		r->f = (int)rn["f"];
		r->time = rn["mobTime"];
		r->Init();
		r->Reset(r->x, r->y);
		if (r->type == "n") Npcs[NpcStart++] = (NLS::Npc*)r;
		else if (r->type == "m") Mobs[MobStart++] = (NLS::Mob*)r;
	}
}

void NLS::Life::Init() {
	if (type == "m") {
		data = WZ["Mob"][id];
		name = (string)WZ["String"]["Mob"][id]["name"];
		speedMin = (double)abs((int)data["info"]["speed"]) + 10;
	}
	else if (type == "n") {
		data = WZ["Npc"][id];
		speedMin = 30;
		auto str =  WZ["String"]["Npc"][id];
		name = (string)str["name"];
		string scriptname = (string)data["info"]["script"]["0"]["script"];
		((Npc*)this)->function = (string)str["func"] + (scriptname.empty() ? "" : " (" + scriptname + ")");
		((Npc*)this)->functiontag.Set(((Npc*)this)->function, NameTag::Life);
		((Npc*)this)->cb.Set("Welkom op NoLifeStory! Praat tegen mij als je dood wilt. Lol.", "npc");
		((Npc*)this)->showcb = true;
	}
	down = false;
	up = false;
	notAPlayer = true;
	nametag.Set(name, NameTag::Life);
	if (data["info"]["link"]) {
		data = data[".."][data["info"]["link"]];
	}
	defaultState = data["info"]["flySpeed"] ? "fly" : "stand";
	ChangeState(defaultState);
}

void NLS::Life::ChangeState(const string &newState) {
	if (!data[newState]) {
		return;
	}
	if (newState == currentState) return;
	currentAnimation.Set(data[newState]);
	currentState = newState;
}

void NLS::Life::Draw() {
	Update();
	currentAnimation.Draw(x, y, data["info"]["noFlip"] ? 0 : f);
	if (!data["info"]["hideName"]) {
		nametag.Draw(x, y);
	}
}

void NLS::Life::Update() {
	if (!currentAnimation.repeat) {
		if (currentAnimation.done) {
			if (timeToNextAction >= 90000) {
				timeToNextAction = 700;
			}
		}
	}
	if (timeToNextAction-- <= 0) {
		if (isNPC) {
			bool walk = rand()%2 == 1;
			if (walk) {
				((Npc*)this)->showcb = false;
				currentAnimation.repeat = true;
				switch (rand()%4) {
				case 0:
					if (data["move"]) {
						left = true;
						right = false;
						timeToNextAction = (isNPC ? 0 : 100) + rand() % (type == "n" ? 90 : 100);
						break;
					}
				case 1:
					if (data["move"]) {
						left = false;
						right = true;
						timeToNextAction = (isNPC ? 0 : 100) + rand() % (type == "n" ? 90 : 100);
						break;
					}
				case 2: // Just stand.
					left = false;
					right = false;
					timeToNextAction = rand() % 500;
					break;
				}
			}
			else {
				currentAnimation.repeat = false;
				bool found = false;
				Node chosenNode;
				for (auto i = 0; i < 4; i++) {
					for_each(data.begin(), data.end(), [&](pair<string, Node>node) {
						Node realnode = node.second;
						if (!found && realnode["speak"] && rand() % 5 == 2) {
							chosenNode = realnode;
							if (node.first == "info") {
								ChangeState("stand");
							}
							else {
								ChangeState(node.first);
							}
							found = true;
						}
					});
					if (found) break;
				}
				if (found) {
					Node speakLines = chosenNode["speak"];
					string line = "";
					found = false;
					for (auto i = 0; i < 4; i++) {
						for_each(speakLines.begin(), speakLines.end(), [&](pair<string, Node>node) {
							Node realnode = node.second;
							if (!found && rand() % 5 == 2) {
								found = true;
								line = node.second;
							}
						});
						if (found) break;
					}
					if (!line.empty()) {
						((Npc*)this)->showcb = true;
						((Npc*)this)->cb.Set((string)WZ["String"]["Npc"][tostring(toint(id))][line], "npc");
						timeToNextAction = 929999; // ...
					}
					else {
						((Npc*)this)->showcb = false;
						timeToNextAction = rand() % 500;
					}
				}
				else {
					((Npc*)this)->showcb = false;
					ChangeState("stand");
					timeToNextAction = rand() % 500;
				}
				left = false;
				right = false;
			}
		}
		else {
			if (data["move"]) {
				switch (rand()%3) {
				case 0:
					left = true;
					right = false;
					timeToNextAction = (isNPC ? 0 : 100) + rand() % (type == "n" ? 90 : 100);
					break;
				case 1:
					left = false;
					right = true;
					timeToNextAction = (isNPC ? 0 : 100) + rand() % (type == "n" ? 90 : 100);
					break;
				case 2:
					left = false;
					right = false;
					timeToNextAction = rand() % (type == "n" ? 5000 : 1000);
					break;
				}
			}
		}
	}
	Physics::Update();
	
	string state = defaultState;
	if (fh) {
		if (left^right) {
			state = "move";
			ChangeState(state);
		} 
		else if (isNPC && !((Npc*)this)->showcb) {
			state = defaultState;
			ChangeState(state);
		}
		else if (!isNPC) {
			state = defaultState;
			ChangeState(state);
		}
	}
	
	currentAnimation.Step();
}

void NLS::Npc::Draw() {
	NLS::Life::Draw();

	if (!function.empty()) {
		functiontag.Draw(x, y+15);
	}

	if (showcb) {
		Sprite sh = currentAnimation.f;
		cb.Draw(x, y - sh.data->height - 10);
	}

	if (data["info"]["MapleTV"] && (int)data["info"]["MapleTV"] == 1) {
		int32_t mx = x + (int)data["info"]["MapleTVadX"];
		int32_t my = y + (int)data["info"]["MapleTVadY"];

		NLS::Sprite sprite = WZ["UI"]["MapleTV"]["TVmedia"]["1"]["0"];
		my += sprite.data->height;

		if (!hasMapleTVAnim) {
			int32_t ad = rand() % 3;
			hasMapleTVAnim = true;
			mapleTVanim.Set(WZ["UI"]["MapleTV"]["TVmedia"][tostring(ad)]);
		}
		mapleTVanim.Step();

		mapleTVanim.Draw(mx, my, f);

		sprite = WZ["UI"]["MapleTV"]["TVbasic"]["0"];

		my = y + (int)data["info"]["MapleTVmsgY"] + sprite.data->height;
		mx = x + (int16_t)(int)data["info"]["MapleTVmsgX"];
		
		sprite.Draw(mx, my, f);
	}
}

void NLS::Mob::Draw() {
	NLS::Life::Draw();
}