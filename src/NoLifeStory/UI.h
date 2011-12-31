////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////

namespace NLS {
	namespace UI {
		class Element;
		class Window {
		public:
			Window(int x, int y, int width, int height, bool focusable, bool stealsfocus, bool visible, bool login);
			virtual ~Window();
			virtual void Draw();
			void Focus();
			bool HandleKey(sf::Keyboard::Key);
			void Add(Element* e);
			vector<Element*> Elements;
			map<sf::Keyboard::Key, function<void()>> actions;
			int x, y;
			int width, height;
			bool focusable, stealsfocus, visible, login;
			static list<Window*> All;
			static list<Window*>::iterator begin() {return All.begin();}
			static list<Window*>::iterator end() {return All.end();}
		private:
			Window(const Window&);
		};
		class Element {
		public:
			Element(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}
			virtual ~Element() {}
			virtual void Draw();
			virtual void Click(sf::Mouse::Button) {}
			int CalcX() {return x+parent->x;}
			int CalcY() {return y+parent->y;}
			int x, y;
			int width, height;
			Window* parent;
			vector<Element*> children;
		//private:
			//Element(const Element&);
		};
		class Image: public Element {
		public:
			Image(int x, int y, Node n) : node(n), Element(x, y, 0, 0) {}
			void Draw();
			Node node;
		};
		class Label: public Element {
		public:
			Label(int x, int y, u32string text, int size = 14) : Element(x, y, 0, 0) {
				txt.Set(text, size);
			}
			void Draw() {
				txt.Draw(CalcX(), CalcY());
			}
			Text txt;
		};
		class Button : public Element {
		public:
			Button(int x, int y, int width, int height, Node n, function<void()> action, const u32string &text = u32(""));
			void Click(sf::Mouse::Button);
			void Draw();
			function<void()> action;
			NLS::Node node;
			bool disabled, pressed;
			Text text;
		};
		class CheckBox : public Element {
		public:
			CheckBox(int x, int y, Node n);
			void Draw();
			void Click(sf::Mouse::Button);
			NLS::Node node;
			bool checked;
		};
		class TextBox : public Element {
		public:
			TextBox(int x, int y, int width, function<void()> onEnter, function<void()> onTab, bool password = false, char passchar = '*')
				: index(0), size(0), sel1(0), sel2(0), selecting(false), Element(x, y, width, 14), password(password), passwordChar(passchar),
				actionEnter(onEnter), actionTab(onTab)
			{
				
			}
			static TextBox* Active;
			void Send();
			void Draw();
			void Click(sf::Mouse::Button);
			void HandleChar(char32_t);
			void HandleKey(sf::Event::KeyEvent);
			void UpdateText();
			void UpdateSelection();
			void Clear();
			u32string str;
			int index, size;
			Text text;
			int sel1, sel2;
			bool selecting, password;
			char passwordChar;
			function<void()> actionEnter, actionTab;
		};
		class ScrollBar : public Element {
		public:
			int LineHeight;
			int CurLine;
		};
		class LSPageButton : public Element {
		public:
			LSPageButton(int x, int y, int width, int height, Node n, function<void()> action, const u32string &text = u32(""));
			void Click(sf::Mouse::Button);
			void Draw();
			function<void()> action;
			NLS::Node node;
			bool pressed;
			Text text;
		};
		class StatusBar : public Window {
		public:
			StatusBar();
			void Draw();
			void OnChatBoxEnter();
			TextBox text;
			/*TextBox tChat;//Yuck

			Image sLevelBG;//Yuck
			Image sLevelCover;
			Image sGaugeBG;
			Image sGaugeCover;
			Image sNotice;
			Image sChatCover;
			Image sChatSpace;

			Button bCashshop;//Yuck
			Button bChattarget;
			Button bCharacter;
			Button bChat;
			Button bClaim;
			Button bEquip;
			Button bInvent;
			Button bKeySettings;
			Button bMenu;
			Button bMTS;
			Button bQuest;
			Button bSkill;
			Button bStat;
			Button bSystem;
			Button bChatclose;*/
		};
		class LoginDialog : public Window {
		public:
			LoginDialog();
			void Draw();
			/*
			TextBox tUsername;
			TextBox tPassword;
			CheckBox cbRemember;
			Button bRemember;//Yuck
			Button bLoginLost;
			Button bPassLost;
			Button bNew;
			Button bHomePage;
			Button bQuit;
			Button bLogin;
			*/
		};

		void Init();
		void Draw();
		extern bool Focused;
		enum StyleEnum {
			Beta,//The non-hand mouse and other cool stuff
			Classic,//Pre big-bang
			Modern,//Post big-bang garbage
			Clean//No images, just rectangles :D
		};
		extern StyleEnum Style;
	}
}
