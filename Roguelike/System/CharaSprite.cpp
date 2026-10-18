#include "CharaSprite.h"
#include<DxLib.h>
#include<array>
#include<vector>
#include<unordered_map>
#include<algorithm>

namespace
{
	struct Source
	{
		const wchar_t* path;
		int frames;
	};

	//CharaArt の じゅんばん
	constexpr Source kSources[static_cast<int>(CharaArt::Num)] =
	{
		{ L"data/gfx/chara/hero.png", 4 },
		{ L"data/gfx/chara/sword.png", 1 },
		{ L"data/gfx/chara/slime_blue.png", 4 },
		{ L"data/gfx/chara/slime_green.png", 4 },
		{ L"data/gfx/chara/king_slime.png", 5 },
		{ L"data/gfx/chara/bat.png", 1 },
		{ L"data/gfx/chara/mole.png", 4 },
		{ L"data/gfx/chara/drop.png", 4 },
		{ L"data/gfx/chara/rock.png", 1 },
		{ L"data/gfx/chara/flame.png", 4 },
		{ L"data/gfx/chara/wolf.png", 2 },
		{ L"data/gfx/chara/lion.png", 2 },
		{ L"data/gfx/chara/ghost.png", 4 },
		{ L"data/gfx/chara/golem.png", 3 },
		{ L"data/gfx/chara/orb.png", 4 },
		{ L"data/gfx/chara/dragon_green.png", 4 },
		{ L"data/gfx/chara/dragon_boss.png", 1 },
		{ L"data/gfx/chara/bird.png", 4 },
		{ L"data/gfx/chara/armor_dark.png", 1 },
		{ L"data/gfx/chara/armor_holy.png", 1 },
		{ L"data/gfx/chara/spirit_ice.png", 4 },
		{ L"data/gfx/chara/spirit_fire.png", 4 },
		{ L"data/gfx/chara/demon.png", 5 },
		{ L"data/gfx/chara/priest.png", 1 },
		{ L"data/gfx/chara/crab.png", 5 },
		{ L"data/gfx/chara/boar.png", 2 },
		{ L"data/gfx/chara/doll.png", 1 },
		{ L"data/gfx/chara/rat.png", 2 },
	};

	struct Sheet
	{
		int soft = -1;	//うすい かげを けした ピクセル(ぬりかえ用)
		CharaSprite::Info info;
		std::vector<int> frames;
	};

	std::array<Sheet, static_cast<int>(CharaArt::Num)> gSheets;
	std::unordered_map<unsigned long long, int> gTinted;

	constexpr int kAlphaCut = 128;	//これ より うすい ピクセル(そざいに ついた かげ)は けす

	unsigned long long TintKey(CharaArt art, int frame, const Game::Rgb& c, float strength)
	{
		const unsigned long long s = static_cast<unsigned long long>(std::clamp(strength, 0.0f, 1.0f) * 100.0f);
		return (static_cast<unsigned long long>(art) << 48) | (static_cast<unsigned long long>(frame) << 40) |
			(static_cast<unsigned long long>(c.r & 0xff) << 32) | (static_cast<unsigned long long>(c.g & 0xff) << 24) |
			(static_cast<unsigned long long>(c.b & 0xff) << 16) | s;
	}

	bool LoadSheet(const Source& src, Sheet& sheet)
	{
		const int raw = LoadSoftImage(src.path);
		if (raw == -1) return false;
		int w = 0, h = 0;
		GetSoftImageSize(raw, &w, &h);

		//うすい かげを けして、体の はんいを しらべる
		sheet.soft = MakeARGB8ColorSoftImage(w, h);
		CharaSprite::Info& info = sheet.info;
		info.frames = src.frames;
		info.frameW = w / src.frames;
		info.frameH = h;
		info.left = info.frameW;
		info.top = info.frameH;
		info.right = 0;
		info.bottom = 0;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				int r = 0, g = 0, b = 0, a = 0;
				GetPixelSoftImage(raw, x, y, &r, &g, &b, &a);
				if (a < kAlphaCut) { r = g = b = a = 0; }
				else a = 255;
				DrawPixelSoftImage(sheet.soft, x, y, r, g, b, a);
				if (a == 0) continue;
				const int fx = x % info.frameW;
				info.left = std::min(info.left, fx);
				info.right = std::max(info.right, fx + 1);
				info.top = std::min(info.top, y);
				info.bottom = std::max(info.bottom, y + 1);
			}
		}
		DeleteSoftImage(raw);

		sheet.frames.resize(src.frames);
		for (int i = 0; i < src.frames; i++)
		{
			sheet.frames[i] = CreateGraphFromRectSoftImage(sheet.soft, i * info.frameW, 0, info.frameW, info.frameH);
			if (sheet.frames[i] == -1) return false;
		}
		return true;
	}
}

bool CharaSprite::Init()
{
	for (int i = 0; i < static_cast<int>(CharaArt::Num); i++)
	{
		if (!LoadSheet(kSources[i], gSheets[i])) return false;
	}
	return true;
}

void CharaSprite::Terminate()
{
	for (auto& [key, handle] : gTinted) DeleteGraph(handle);
	gTinted.clear();
	for (Sheet& s : gSheets)
	{
		for (int h : s.frames) { if (h != -1) DeleteGraph(h); }
		s.frames.clear();
		if (s.soft != -1) DeleteSoftImage(s.soft);
		s.soft = -1;
	}
}

const CharaSprite::Info& CharaSprite::Get(CharaArt art)
{
	return gSheets[static_cast<int>(art)].info;
}

int CharaSprite::Frame(CharaArt art, int frame)
{
	const Sheet& s = gSheets[static_cast<int>(art)];
	if (s.frames.empty()) return -1;
	return s.frames[std::clamp(frame, 0, static_cast<int>(s.frames.size()) - 1)];
}

int CharaSprite::Tinted(CharaArt art, int frame, const Game::Rgb& color, float strength)
{
	const Sheet& s = gSheets[static_cast<int>(art)];
	if (s.frames.empty()) return -1;
	frame = std::clamp(frame, 0, static_cast<int>(s.frames.size()) - 1);
	if (strength <= 0.0f) return s.frames[frame];
	const unsigned long long key = TintKey(art, frame, color, strength);
	const auto found = gTinted.find(key);
	if (found != gTinted.end()) return found->second;

	const int fw = s.info.frameW;
	const int fh = s.info.frameH;
	const int img = MakeARGB8ColorSoftImage(fw, fh);
	for (int y = 0; y < fh; y++)
	{
		for (int x = 0; x < fw; x++)
		{
			int r = 0, g = 0, b = 0, a = 0;
			GetPixelSoftImage(s.soft, frame * fw + x, y, &r, &g, &b, &a);
			const float lum = (0.299f * r + 0.587f * g + 0.114f * b) / 255.0f;

			//ふちどり(くらい いろ)は そのまま
			if (a > 0 && lum >= 0.16f)
			{
				//あかるさの もようを のこして その いろに する
				const float k = 0.35f + 0.95f * lum;
				const float tr = std::min(255.0f, color.r * k);
				const float tg = std::min(255.0f, color.g * k);
				const float tb = std::min(255.0f, color.b * k);
				r = static_cast<int>(r + (tr - r) * strength);
				g = static_cast<int>(g + (tg - g) * strength);
				b = static_cast<int>(b + (tb - b) * strength);
			}
			DrawPixelSoftImage(img, x, y, r, g, b, a);
		}
	}
	const int handle = CreateGraphFromSoftImage(img);
	DeleteSoftImage(img);
	gTinted[key] = handle;
	return handle;
}
