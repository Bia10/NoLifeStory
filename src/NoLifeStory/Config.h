////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////

namespace NLS {
	namespace Config {
		void Load();
		void LoadDefault();
		void Save();
		extern int32_t LoadCharid;
	}
	extern bool Profiling;
	extern vector<string> ProfileMaps;
}