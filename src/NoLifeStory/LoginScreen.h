////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
namespace NLS {
	class UILoginScreen : public UI::Window {
	public:
		UILoginScreen();
		void Draw();

		UI::TextBox *uname, *passwd;
	};
	class UIWorldSelectScreen : public UI::Window {
	public:
		UIWorldSelectScreen();
		static int8_t selectedWorld;

		static void Init();
		static void Uninit();

		struct WorldInfo {
			WorldInfo(int8_t id, int8_t ribbon, int16_t exp, int16_t drop, string name, string eventmsg);
			struct ChannelInfo {
				int8_t id;
				int8_t worldID;
				int8_t state;
				int32_t population;
				string name;
			};

			int8_t id;
			int8_t ribbon;
			int16_t expRate;
			int16_t dropRate;
			string name;
			string eventMsg;

			Text worldDrawName;

			map<int8_t, ChannelInfo> channels;
		};

		static WorldInfo *selectedWorldInfo;

		void DrawWorlds();
		static void SelectedWorld(int8_t worldid);
		vector<WorldInfo*> worlds;

		pair<int32_t, int32_t> GetNextWorldPos(int32_t worldid = -1) {
			int32_t i = worldid == -1 ? worlds.size() : worldid;
			int32_t curx = ((i % 6) * 96);
			int32_t cury = ((i / 6) * 26);
			return pair<int32_t, int32_t>(curx, cury);
		}

		class ChannelSelect : public Window {
		public:
			ChannelSelect();
			static int8_t selectedChannel;
			static void SelectChannel(int8_t chid);
			map<int32_t, UI::Button *> channelButtons;
		} *ChannelSelectWindow;
	};
	class UICharacterSelectScreen : public UI::Window {
	public:
		UICharacterSelectScreen();
		
		vector<LoginPlayer *> worldCharacters;
		int8_t startSlot;
		int32_t effectFrame;
		int32_t selectedChar;
		AniSprite selection;
		void ChangeSelectedChar(int32_t id) {
			selectedChar = id;
			selection.frame = 0;
			effectFrame = 0;
		}
		void ChangeStartSlot(bool);
		void Draw();
	};
	class LoginErrorMessage : public UI::Window {
	public:
		LoginErrorMessage(int32_t textid, int32_t bgid, string button1, function<void()> button1click, string button2, function<void()> button2click) : 
		UI::Window(0, 0, 362, 219, false, true, true, true), textid(textid), bgid(bgid) {
			Add(new UI::Button(100, 100, 75, 34, WZ["UI"]["Login"]["Notice"]["Bt" + button1], button1click, u32(button1)));
			Add(new UI::Button(190, 100, 75, 34, WZ["UI"]["Login"]["Notice"]["Bt" + button2], button2click, u32(button2)));
		}
		LoginErrorMessage(int32_t textid, int32_t bgid, string button1, function<void()> button1click) : 
		UI::Window(0, 0, 362, 219, false, true, true, true), textid(textid), bgid(bgid) {
			Add(new UI::Button(100, 100, 75, 34, WZ["UI"]["Login"]["Notice"]["Bt" + button1], button1click, u32(button1)));
		}
		int32_t bgid, textid;
		void Draw();
	};
	extern LoginErrorMessage * LoginErrorMsg;
	extern UILoginScreen * LoginScreen;
	extern UIWorldSelectScreen * WorldSelectScreen;
	extern UICharacterSelectScreen * CharSelectScreen;
}