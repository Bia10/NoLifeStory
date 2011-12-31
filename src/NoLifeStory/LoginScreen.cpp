////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

NLS::LoginErrorMessage * NLS::LoginErrorMsg;
NLS::UILoginScreen * NLS::LoginScreen = nullptr;
NLS::UIWorldSelectScreen * NLS::WorldSelectScreen = nullptr;
NLS::UICharacterSelectScreen * NLS::CharSelectScreen = nullptr;
int8_t NLS::UIWorldSelectScreen::selectedWorld = -1;
int8_t NLS::UIWorldSelectScreen::ChannelSelect::selectedChannel = -1;
NLS::UIWorldSelectScreen::WorldInfo *NLS::UIWorldSelectScreen::selectedWorldInfo = nullptr;

NLS::UILoginScreen::UILoginScreen() : Window(-100, -50, 200, 100, false, false, true, true) {
	auto loginbtn = WZ["UI"]["Login"]["Title"]["BtLogin"];
	Add(new UI::Button(140, 45, 89, 42, loginbtn, [this]() {
		cout << "Pressed login btn" << endl;
		Send::Login(u8(uname->str), u8(passwd->str));
	}, u32("Login!")));

	auto sw = [this] { UI::TextBox::Active = passwd; };

	Add(uname = new UI::TextBox(100, 10, 80, sw, sw));
	Add(passwd = new UI::TextBox(100, 30, 80, [this]{cout << "Trying to login with: " << u8(uname->str) << endl; }, [this]{ UI::TextBox::Active = uname; }, true, '*'));
	Add(new UI::Label(10, 8, Text::Color(255,255,255) + u32("Username:")));
	Add(new UI::Label(10, 28, Text::Color(255,255,255) + u32("Password:")));
}

void NLS::UILoginScreen::Draw() {
	glColor4f(0.2, 0.2, 1, 1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x+width, y);
	glVertex2i(x+width, y+height);
	glVertex2i(x, y+height);
	glEnd();
	Window::Draw();
}

NLS::UIWorldSelectScreen::UIWorldSelectScreen() : Window(-250, -865, 560, 500, false, false, true, true) {
	ChannelSelectWindow = new ChannelSelect();
}

NLS::UIWorldSelectScreen::WorldInfo::WorldInfo(int8_t id, int8_t ribbon, int16_t exp, int16_t drop, string name, string eventmsg) : 
id(id), ribbon(ribbon), expRate(exp), dropRate(drop), name(name), eventMsg(eventmsg)
{
	auto p = WorldSelectScreen->GetNextWorldPos();
	WorldSelectScreen->Add(new UI::Button(p.first, p.second, 80, 24, WZ["UI"]["Login"]["WorldSelect"]["BtWorld"][id], [id]() {
		NLS::UIWorldSelectScreen::SelectedWorld(id);
		cout << "Clicked buttan " << (int16_t)NLS::UIWorldSelectScreen::selectedWorld << endl;
	}, u32(name)));
}

NLS::UIWorldSelectScreen::ChannelSelect::ChannelSelect() : Window(-250 + 93, -865 + 180, 400, 400, false, false, true, true) {
	Add(new UI::Button(230, 55, 118, 23, WZ["UI"]["Login"]["WorldSelect"]["BtGoworld"], []() {
		cout << "Selected Channel!" << endl;
	}, u32("Select World >>")));
	for (auto i = 0; i < 20; i++) {
		int32_t bx = 25 + (i % 5) * 66;
		int32_t by = 90 + (i / 5) * 29;
		auto button = new UI::Button(bx, by, 60, 21, WZ["UI"]["Login"]["WorldSelect"]["channel"][i], [i]() {
			NLS::UIWorldSelectScreen::ChannelSelect::SelectChannel(i);
		}, u32("Ch. " + i));
		channelButtons[i] = button;
		Add(button);
	}
}

void NLS::UIWorldSelectScreen::Init() {
	LoginScreen = new NLS::UILoginScreen();
	WorldSelectScreen = new NLS::UIWorldSelectScreen();
	CharSelectScreen = new NLS::UICharacterSelectScreen();
	LoginErrorMsg = new NLS::LoginErrorMessage(14, 1, "Yes", [](){ delete NLS::LoginErrorMsg; NLS::LoginErrorMsg = nullptr; });
}

void NLS::UIWorldSelectScreen::Uninit() {
	for (auto i = WorldSelectScreen->worlds.begin(); i != WorldSelectScreen->worlds.end(); i++) {
		delete *i;
	}
	WorldSelectScreen->worlds.clear();
	delete WorldSelectScreen;
	delete CharSelectScreen;
	delete LoginScreen;
}

void NLS::UIWorldSelectScreen::ChannelSelect::SelectChannel(int8_t chid) {
	cout << "Selected channel " << (int16_t)chid << endl;
	if (NLS::UIWorldSelectScreen::ChannelSelect::selectedChannel == chid) {
		// Select channel.
		Send::ChannelSelectRequest();
	}
	else {
		NLS::UIWorldSelectScreen::ChannelSelect::selectedChannel = chid;
	}
}

void NLS::UIWorldSelectScreen::SelectedWorld(int8_t worldid) {
	if (selectedWorld == worldid)
		selectedWorld = -1;
	else
		selectedWorld = worldid;

	if (selectedWorld != -1) {
		// Get channels.
		auto found = find_if(WorldSelectScreen->worlds.begin(), WorldSelectScreen->worlds.end(), [&](WorldInfo *wi){return wi->id == WorldSelectScreen->selectedWorld; });
		WorldInfo *wi = *found;
		WorldSelectScreen->selectedWorldInfo = wi;
		WorldSelectScreen->ChannelSelectWindow->selectedChannel = -1;
		bool autoselect = true;
		for (auto i = 0; i < 20; i++) {
			auto t = wi->channels.find(i);
			if (t == wi->channels.end()) {
				WorldSelectScreen->ChannelSelectWindow->channelButtons[i]->disabled = true;
			}
			else {
				WorldSelectScreen->ChannelSelectWindow->channelButtons[i]->disabled = false;
				if (autoselect && (*t).second.population < 800) {
					autoselect = false;
					WorldSelectScreen->ChannelSelectWindow->selectedChannel = i;
				}
			}
		}
		WorldSelectScreen->ChannelSelectWindow->visible = false; // Till we get a response.
		Send::WorldSelectRequest();
	}
}

void NLS::UIWorldSelectScreen::DrawWorlds() {
	if (UI::Style == UI::Clean) {
		Draw();
		int32_t viewx = this->x;
		int32_t viewy = this->y;

		for (auto i = worlds.size(); i < (6 * 7); i++) {
			auto p = WorldSelectScreen->GetNextWorldPos(i);
			Sprite spr = WZ["UI"]["Login"]["WorldSelect"]["BtWorld"]["e"]["disabled"][0];
			spr.Draw(viewx + p.first, viewy + p.second);
		}

		if (selectedWorld != -1) {
			WorldInfo *wi = selectedWorldInfo;
			viewx = this->x + 95;
			viewy = this->y + 180;
			
			Sprite spr = WZ["UI"]["Login"]["WorldSelect"]["world"][tostring(selectedWorld)];
			spr.Draw(viewx + 20, viewy + 33);
			if (ChannelSelectWindow->selectedChannel != -1) {
				auto btn = ChannelSelectWindow->channelButtons[ChannelSelectWindow->selectedChannel];
				spr = WZ["UI"]["Login"]["WorldSelect"]["channel"]["chSelect"][0];
				spr.Draw(btn->CalcX(), btn->CalcY());
			}
			ChannelSelectWindow->Draw();

			spr = WZ["UI"]["Login"]["WorldSelect"]["channel"]["chgauge"];
			for (auto i = 0; i < 20; i++) {
				auto btn = ChannelSelectWindow->channelButtons[i];
				if (btn->disabled) continue;
				auto data = wi->channels[i];
				int w = spr.data->width;
				if (data.population >= 1000) { }
				else {
					w *= (data.population / 1000.0);
				}
				spr.DrawX(btn->CalcX() + 2, btn->CalcY() + 13, w);
			}
		}
	}
}

NLS::UICharacterSelectScreen::UICharacterSelectScreen() : Window(-250, -1450, 650, 400, false, false, true, true) {
	startSlot = 0;
	Add(new UI::LSPageButton(80, 240, 86, 84, WZ["UI"]["Login"]["CharSelect"]["pageL"], []() {
		cout << "PageLeft!" << endl;
		CharSelectScreen->ChangeStartSlot(false);
	}, u32("Next Page >>")));
	Add(new UI::LSPageButton(490, 240, 86, 84, WZ["UI"]["Login"]["CharSelect"]["pageR"], []() {
		cout << "PageRight!" << endl;
		CharSelectScreen->ChangeStartSlot(true);
	}, u32("Next Page >>")));

	selection.Set(WZ["UI"]["Login"]["CharSelect"]["character"]["0"]);
	effectFrame = 0;
}

void NLS::UICharacterSelectScreen::ChangeStartSlot(bool up) {
	for (auto i = startSlot; i < worldCharacters.size(); i++) {
		LoginPlayer *lp = worldCharacters[i];
		lp->x = 0;
		lp->y = 0;
	}

	if (up) {
		if (startSlot + 3 >= worldCharacters.size()) {
			startSlot = 0;
		}
		else {
			startSlot += 3;
		}
	}
	else {
		if (startSlot - 3 < 0) {
			startSlot = (worldCharacters.size() / 3)*3;
		}
		else {
			startSlot -= 3;
		}
	}
}

void NLS::UICharacterSelectScreen::Draw() {

	Window::Draw();

	int32_t startx = -250 + 200;
	int32_t starty = -1250 + 110;
	int j = 1;
	for (auto i = startSlot; i < worldCharacters.size(); i++) {
		LoginPlayer *lp = worldCharacters[i];
		lp->x = startx;
		lp->y = starty;
		if (lp->charid == selectedChar) {
			Node n = WZ["UI"]["Login"]["CharSelect"]["effect"]["1"];
			if (n[effectFrame + 1]) {
				effectFrame++;
			}
			Sprite spr = n[effectFrame];
			spr.Draw(lp->x, lp->y - 400);
		}
		lp->Draw();
		startx += 100;
		if (lp->charid == selectedChar) {
			selection.Step();
			selection.Draw(lp->x, lp->y);
		}
		if (j++ == 3) break; // 3 max
	}
}

void NLS::LoginErrorMessage::Draw() {
	Sprite spr = WZ["UI"]["Login"]["Notice"]["backgrnd"][bgid];
	width = spr.data->width;
	height = spr.data->height;
	x = View::x + (window->GetWidth()/2 - width/2);
	y = View::y + (window->GetHeight()/2 - height/2);
	spr.Draw(x, y);

	spr = WZ["UI"]["Login"]["Notice"]["text"][bgid];
	spr.Draw(x + 16, y + 12);
	Window::Draw();
}