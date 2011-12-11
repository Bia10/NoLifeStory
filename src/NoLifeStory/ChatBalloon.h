////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////

namespace NLS {
	class ChatBalloon {
	public:
		void Set(string msg, string type);
		void Draw(int32_t x, int32_t y);
		string type;
		vector<Text> text;
		Sprite arrow, c, e, n, ne, nw, s, se, sw, w;
	};
	inline void ChatBalloon::Set(string msg, string type) {
		text.clear();
		this->type = type;
		// Set ALL sprites!
		Node dataNode = WZ["UI"]["ChatBalloon"][type];
		arrow = dataNode["arrow"];
		c = dataNode["c"];
		e = dataNode["e"];
		n = dataNode["n"];
		w = dataNode["w"];
		s = dataNode["s"];
		ne = dataNode["ne"];
		nw = dataNode["nw"];
		sw = dataNode["sw"];
		se = dataNode["se"];

		uint32_t color = (uint32_t)(int32_t)dataNode["clr"];
		uint8_t r = color & 0xFF;
		uint8_t g = (color << 8) & 0xFF;
		uint8_t b = (color << 16) & 0xFF;
		//uint8_t a = (color << 24) & 0xFF;
		const int32_t maxchars = 16;
		for (auto i = 0; i < msg.length();) {
			size_t characters = maxchars;
			if (msg.length() <= i + characters) {
				characters = msg.length() - i;
			}
			else if (msg[characters] != ' ') {
				// Go back and see if we have a space
				auto curpos = characters + i;
				bool found = false;
				for (auto j = curpos; j > curpos - 3; j--) {
					if (j < 0) break;
					if (msg.size() <= j) break;
					if (msg[j] == ' ') {
						characters = j - i;
						found = true;
						break;
					}
				}
				if (!found) {
					for (auto j = curpos; j < curpos + 3; j++) {
						if (msg.length() - 1 == j) break;
						if (msg[j] == ' ') {
							characters = j - i;
							found = true;
							break;
						}
					}
				}
			}
			if (characters == 0) break;
			Text txt;
			txt.Set(Text::Color(r, g, b/*, a*/) + u32(msg.substr(i, characters)), 12);
			text.push_back(txt);
			i += characters;
		}
	}

	inline void ChatBalloon::Draw(int32_t x, int32_t y) {
		if (text.size() == 0) return; //...

		int32_t width = 0, height = 0;
		for (auto i = 0; i < text.size(); i++) {
			if (width < text[i].Width()) width = text[i].Width();
			height += text[i].Height();
		}
		// Draw topline
		if (width == 0) {
			width = 10;
		}

		int32_t left = x - width/2, right = left + width;
		int32_t top = y - height, bottom = y;
		int32_t realright = left;

		for (auto i = left; i < right; i += c.data->width)
			realright += c.data->width;

		for (auto i = left; i < right; i += n.data->width)
			n.Draw(i, top);
		nw.Draw(left, top);
		ne.Draw(realright, top);
		
		int32_t counter = top;
		for (auto i = 0; i < text.size(); i++) {
			for (auto j = left; j < right; j += c.data->width) 
				c.Draw(j, counter);
			w.Draw(left, counter);
			e.Draw(realright, counter);
			text[i].Draw(x - text[i].Width()/2, counter);
			counter += text[i].Height();
		}
		
		for (auto i = left; i < right; i += s.data->width) {
			s.Draw(i, bottom);
		}
		sw.Draw(left, bottom);
		se.Draw(realright, bottom);
		arrow.Draw(x, bottom);
	}

}