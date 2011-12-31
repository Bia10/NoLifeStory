////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

array<NLS::AniSprite, NLS::Mouse::Total> Sprites;

int NLS::Mouse::x = 0;
int NLS::Mouse::y = 0;
NLS::Mouse::StateEnum NLS::Mouse::State = NLS::Mouse::Normal;
NLS::UI::Element* NLS::Mouse::over = nullptr;
int NLS::Mouse::cx = 0;
int NLS::Mouse::cy = 0;
int NLS::Mouse::checkx = 0;
int NLS::Mouse::checky = 0;

void NLS::Mouse::Init() {
	Node base = WZ["UI"]["Basic"]["Cursor"];
	for (int i = 0; i < Total; ++i) {
		Sprites[i].Set(base[i]);
	}
	State = Normal;
}

void NLS::Mouse::Draw() {
	sf::Vector2i p = sf::Mouse::GetPosition(*window);
	x = p.x;
	y = p.y;

	Sprites[State].Step();
	Sprites[State].Draw(x, y);

	if (x < 0 || x > 800 || y < 0 || y > 600) return;
	checkx = Map::Login ? View::x + x : x;
	checky = Map::Login ? View::y + y : y;

	over = nullptr;
	bool found = false;
	if (!found && Map::Login) {
		int32_t curchar = CharSelectScreen->selectedChar;
		int32_t width = 20, height = 35;
		int32_t hitx = View::x+x, hity = View::y+y;
		for_each(CharSelectScreen->worldCharacters.begin(), CharSelectScreen->worldCharacters.end(), [&](LoginPlayer *n) {
			if (!found) {
				if (sf::Mouse::IsButtonPressed(sf::Mouse::Left)) {
					if (n->x - width/2 <= hitx && n->x + width/2 >= hitx && n->y - height <= hity && n->y >= hity) {
						CharSelectScreen->ChangeSelectedChar(n->charid);
						n->state = "walk1";
						n->delay = 0;
						n->emote = "cheers";
						found = true;
					}
				}
			}
		});

		if (found) {
			if (curchar == CharSelectScreen->selectedChar) {
				// Select character and get ingame... or something
				cout << "[TODO] Send packet to server to login into channel!" << endl;
			}
			else {
				// Disable walk anim!
				auto find = find_if(CharSelectScreen->worldCharacters.begin(), CharSelectScreen->worldCharacters.end(), [&](LoginPlayer *n) { return n->charid == curchar; });
				if (find != CharSelectScreen->worldCharacters.end()) {
					(*find)->state = "stand1";
					(*find)->delay = 0;
					(*find)->emote = "blaze";
				}
			}
		}
	}
	if (!found) {
		for_each(UI::Window::begin(), UI::Window::end(), [&](UI::Window* w) {
			if (checkx > w->x and checkx < w->x+w->width and checky > w->y and checky < w->y+w->height) {
				for_each(w->Elements.begin(), w->Elements.end(), [&](UI::Element* e) {
					if (checkx > e->CalcX() and checkx < e->CalcX()+e->width and checky > e->CalcY() and checky < e->CalcY()+e->height) {
						over = e;
					}
				});
			}
		});
	}
	if (State != OnOverClickableLocked) {
		for_each(NLS::Life::Npcs.begin(), NLS::Life::Npcs.end(), [&found](pair<uint32_t, NLS::Npc*> n) {
			if(n.second->CheckPosition(View::x+x,View::y+y)) {
				if (sf::Mouse::IsButtonPressed(sf::Mouse::Left)) {
					State = OnOverClickableLocked;
					n.second->MouseFly();
				}
				else {
					State = OnOverClickable;
				}
				found = true;
			}
		});
		if (!found) {
			for_each(NLS::Life::Mobs.begin(), NLS::Life::Mobs.end(), [&found](pair<uint32_t, NLS::Mob*> n) {
				if (!found && n.second->CheckPosition(View::x+x,View::y+y)) {
					if (sf::Mouse::IsButtonPressed(sf::Mouse::Left)) {
						State = OnOverClickableLocked;
						n.second->MouseFly();
						found = true;
					}
				}
			});
		}
		if (!found) {
			State = Normal;
		}
	}
}

void NLS::Mouse::HandleEvent(sf::Event& e) {
	switch (e.Type) {
	case sf::Event::MouseButtonPressed: {
		if (x < 0 || x > 800 || y < 0 || y > 600) return;
		checkx = Map::Login ? View::x + x : x;
		checky = Map::Login ? View::y + y : y;
		State = OnOverClickableLocked;
		UI::TextBox::Active = nullptr;
		for_each(UI::Window::All.rbegin(), UI::Window::All.rend(), [&](UI::Window* w) {
			if (checkx > w->x and checkx < w->x+w->width and checky > w->y and checky < w->y+w->height) {
				w->Focus();
				sf::Mouse::Button button = e.MouseButton.Button;
				for_each(w->Elements.rbegin(), w->Elements.rend(), [&](UI::Element* m) {
					if (checkx > m->CalcX() and checkx < m->CalcX()+m->width and checky > m->CalcY() and checky < m->CalcY()+m->height) {
						m->Click(button);
					}
				});
			}
		});
		break;
	}
	case sf::Event::MouseButtonReleased:
		State = Normal;
		break;
	case sf::Event::MouseMoved:
		break;
	}
}