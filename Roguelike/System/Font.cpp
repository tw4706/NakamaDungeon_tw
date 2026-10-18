#include "Font.h"
#include<DxLib.h>
#include<array>
#include<cwchar>
#include<algorithm>

namespace
{
	constexpr int kSizeTable[static_cast<int>(FontSize::Num)] = { 24, 36, 56, 96, 150 };
	constexpr int kEdgeTable[static_cast<int>(FontSize::Num)] = { 2, 3, 4, 5, 6 };

	std::array<int, static_cast<int>(FontSize::Num)> handles = { -1, -1, -1, -1, -1 };

	//ゲームに いれた フォント(M PLUS Rounded 1c / SIL OFL 1.1。data/font/MPLUSRounded1c_OFL.txt)
	constexpr const wchar_t* kFontFiles[] =
	{
		L"data/font/MPLUSRounded1c-Bold.ttf",
		L"data/font/MPLUSRounded1c-ExtraBold.ttf",
	};
	constexpr const wchar_t* kFaceBold = L"Rounded Mplus 1c Bold";
	constexpr const wchar_t* kFaceExtraBold = L"Rounded Mplus 1c ExtraBold";
	constexpr const wchar_t* kFaceFallback = L"メイリオ";	//フォントが よめない ときの かわり

	//大きい 文字は ふとい ほう
	constexpr bool kIsExtraBold[static_cast<int>(FontSize::Num)] = { false, false, true, true, true };

	//メイリオより 字が 大きいので、がめんの レイアウトに あわせて すこし 小さく つくる
	constexpr float kSizeRate = 0.92f;
	//おなじ いちに かくと メイリオより 字が 下に でるので、その ぶん 上げる
	constexpr int kOffsetTable[static_cast<int>(FontSize::Num)] = { -2, -2, -2, -4, -6 };

	bool isLoaded = false;

	/// <summary>
	/// 字の 上下の ずれ(倍率をかける まえ)
	/// </summary>
	float OffsetY(FontSize size)
	{
		return isLoaded ? static_cast<float>(kOffsetTable[static_cast<int>(size)]) : 0.0f;
	}

	/// <summary>
	/// 行の 高さ(レイアウトの きじゅん。フォントを 小さく つくっても かわらない)
	/// </summary>
	float LineHeight(FontSize size)
	{
		return static_cast<float>(kSizeTable[static_cast<int>(size)]);
	}
}

void Font::Init()
{
	isLoaded = true;
	for (const wchar_t* path : kFontFiles)
	{
		if (AddFontResourceExW(path, FR_PRIVATE, nullptr) == 0) isLoaded = false;
	}
	for (int i = 0; i < static_cast<int>(FontSize::Num); i++)
	{
		const wchar_t* face = isLoaded ? (kIsExtraBold[i] ? kFaceExtraBold : kFaceBold) : kFaceFallback;
		const int thick = isLoaded ? -1 : 9;
		const int pixel = isLoaded ? static_cast<int>(kSizeTable[i] * kSizeRate + 0.5f) : kSizeTable[i];
		handles[i] = CreateFontToHandle(face, pixel, thick, DX_FONTTYPE_ANTIALIASING_EDGE_8X8, -1, kEdgeTable[i]);
	}
}

void Font::Terminate()
{
	for (int& handle : handles)
	{
		DeleteFontToHandle(handle);
		handle = -1;
	}
	if (!isLoaded) return;
	for (const wchar_t* path : kFontFiles) RemoveFontResourceExW(path, FR_PRIVATE, nullptr);
	isLoaded = false;
}

int Font::Get(FontSize size)
{
	return handles[static_cast<int>(size)];
}

int Font::GetPixelSize(FontSize size)
{
	return kSizeTable[static_cast<int>(size)];
}

void Font::DrawTop(float x, float y, const wchar_t* text, unsigned int color, FontSize size, unsigned int edgeColor, float scale)
{
	const int font = Get(size);
	if (scale == 1.0f)
	{
		DrawStringToHandle(static_cast<int>(x), static_cast<int>(y + OffsetY(size)), text, color, font, edgeColor);
		return;
	}
	DrawExtendStringToHandle(static_cast<int>(x), static_cast<int>(y + OffsetY(size) * scale), scale, scale, text, color, font, edgeColor);
}

void Font::DrawCenter(float x, float y, const wchar_t* text, unsigned int color, FontSize size, unsigned int edgeColor)
{
	const int font = Get(size);
	const int width = GetDrawStringWidthToHandle(text, static_cast<int>(std::wcslen(text)), font);
	const float height = LineHeight(size);
	DrawStringToHandle(static_cast<int>(x) - width / 2, static_cast<int>(y - height * 0.5f + OffsetY(size)), text, color, font, edgeColor);
}

void Font::DrawCenterScaled(float x, float y, float scale, const wchar_t* text, unsigned int color, FontSize size, unsigned int edgeColor)
{
	if (scale <= 0.01f) return;
	const int font = Get(size);
	const int width = GetDrawStringWidthToHandle(text, static_cast<int>(std::wcslen(text)), font);
	const float height = LineHeight(size);
	DrawExtendStringToHandle(static_cast<int>(x - width * scale * 0.5f), static_cast<int>(y + (OffsetY(size) - height * 0.5f) * scale),
		scale, scale, text, color, font, edgeColor);
}

namespace
{
	//これより小さくは縮めない
	constexpr float kMinScale = 0.72f;

	/// <summary>
	/// 収まる倍率と、必要なら末尾を切った文字
	/// </summary>
	std::wstring FitText(const std::wstring& text, int font, float maxWidth, float& scale)
	{
		const int width = GetDrawStringWidthToHandle(text.c_str(), static_cast<int>(text.size()), font);
		scale = 1.0f;
		if (width <= maxWidth || width <= 0) return text;
		scale = maxWidth / width;
		if (scale >= kMinScale) return text;

		scale = kMinScale;
		std::wstring cut = text;
		while (cut.size() > 1)
		{
			cut.pop_back();
			const std::wstring t = cut + L"…";
			if (GetDrawStringWidthToHandle(t.c_str(), static_cast<int>(t.size()), font) * kMinScale <= maxWidth) return t;
		}
		return cut + L"…";
	}
}

void Font::DrawFit(float x, float y, const std::wstring& text, unsigned int color, FontSize size, unsigned int edgeColor, float maxWidth)
{
	const int font = Get(size);
	float scale = 1.0f;
	const std::wstring t = FitText(text, font, maxWidth, scale);
	if (scale >= 1.0f)
	{
		DrawStringToHandle(static_cast<int>(x), static_cast<int>(y + OffsetY(size)), t.c_str(), color, font, edgeColor);
		return;
	}
	//縮めた分だけ下げて、行の中央をそろえる
	const float h = LineHeight(size);
	DrawExtendStringToHandle(static_cast<int>(x), static_cast<int>(y + h * (1.0f - scale) * 0.5f + OffsetY(size) * scale), scale, scale, t.c_str(), color, font, edgeColor);
}

void Font::DrawCenterFit(float x, float y, const std::wstring& text, unsigned int color, FontSize size, unsigned int edgeColor, float maxWidth)
{
	const int font = Get(size);
	float scale = 1.0f;
	const std::wstring t = FitText(text, font, maxWidth, scale);
	const float w = static_cast<float>(GetDrawStringWidthToHandle(t.c_str(), static_cast<int>(t.size()), font)) * scale;
	const float h = LineHeight(size) * scale;
	DrawExtendStringToHandle(static_cast<int>(x - w * 0.5f), static_cast<int>(y - h * 0.5f + OffsetY(size) * scale), scale, scale, t.c_str(), color, font, edgeColor);
}

std::vector<std::wstring> Font::Wrap(const std::wstring& text, FontSize size, float maxWidth)
{
	const int font = Get(size);
	auto width = [font](const std::wstring& s, size_t len) { return GetDrawStringWidthToHandle(s.c_str(), static_cast<int>(len), font); };

	std::vector<std::wstring> out;
	std::wstring rest = text;
	while (!rest.empty())
	{
		if (width(rest, rest.size()) <= maxWidth)
		{
			out.push_back(rest);
			break;
		}
		size_t cut = 1;
		while (cut < rest.size() && width(rest, cut + 1) <= maxWidth) cut++;
		//ぜんかくの くうはく(「さいだいHP +8　かいしん +5」の あいだ など)で きれれば そこ、なければ はんかくの くうはく・くとうてん
		size_t space = rest.find_last_of(L"　", cut - 1);
		if (space != std::wstring::npos && space > cut / 3) cut = space + 1;
		else
		{
			space = rest.find_last_of(L" ・、", cut - 1);
			if (space != std::wstring::npos && space > cut / 2) cut = space + 1;
		}

		//「。」「、」などが 行の あたまに こないように 前の行へ
		while (cut < rest.size() && std::wstring(L"。、，．」』）！？ー…").find(rest[cut]) != std::wstring::npos) cut++;
		out.push_back(rest.substr(0, cut));
		rest = rest.substr(cut);
	}
	return out;
}

void Font::DrawWrapped(float x, float y, const std::wstring& text, unsigned int color, FontSize size, unsigned int edgeColor, float maxWidth, int maxLines, float lineGap)
{
	//おさまる まで すこしずつ 小さく する(ぎょうの あいだも おなじ ぶん つめて、ぜんたいの 高さを かえない)
	const float height = lineGap * maxLines;
	float scale = 1.0f;
	std::vector<std::wstring> lines = Wrap(text, size, maxWidth);
	while (lines.size() * lineGap * scale > height + 0.5f && scale > kMinScale)
	{
		scale -= 0.05f;
		lines = Wrap(text, size, maxWidth / scale);
	}
	const float gap = lineGap * scale;
	for (size_t i = 0; i < lines.size(); i++)
	{
		if ((i + 1) * gap > height + 0.5f) break;
		DrawTop(x, y + i * gap, lines[i].c_str(), color, size, edgeColor, scale);
	}
}

int Font::Width(const std::wstring& text, FontSize size)
{
	return GetDrawStringWidthToHandle(text.c_str(), static_cast<int>(text.size()), Get(size));
}

void Font::DrawCenterAlpha(float x, float y, float scale, int alpha, const wchar_t* text, unsigned int color, FontSize size, unsigned int edgeColor)
{
	if (alpha <= 0) return;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	DrawCenterScaled(x, y, scale, text, color, size, edgeColor);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
