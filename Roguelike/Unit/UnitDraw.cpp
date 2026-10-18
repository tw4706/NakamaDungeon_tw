#include "Unit.h"
#include "DrawUtil.h"
#include "PixelArt.h"
#include "CharaSprite.h"
#include "Sprite.h"
#include<DxLib.h>
#include<cmath>
#include<algorithm>

using namespace MathUtil;

namespace
{
	void Shadow(const Vec2& feet, float w)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 90);
		DrawOvalAA(feet.x, feet.y, w, w * 0.3f, 16, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	//ほのお(下が太く、上がとがる)
	void Flame(float x, float y, float w, float h, float t)
	{
		const float sway = std::sin(t * 3.0f) * w * 0.3f;
		SetDrawBlendMode(DX_BLENDMODE_ADD, 200);
		DrawTriangleAA(x - w, y, x + w, y, x + sway, y - h, GetColor(255, 110, 30), TRUE);
		DrawCircleAA(x, y, w, 12, GetColor(255, 110, 30), TRUE);
		DrawTriangleAA(x - w * 0.5f, y, x + w * 0.5f, y, x + sway * 0.6f, y - h * 0.6f, GetColor(255, 220, 90), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	//こおりの けっしょう
	void Crystal(float x, float y, float w, float h, float lean)
	{
		const float tx = x + lean;
		DrawQuadrangleAA(x - w, y, tx - w * 0.6f, y - h * 0.75f, tx, y - h, tx + w * 0.6f, y - h * 0.75f, GetColor(170, 225, 255), TRUE);
		DrawQuadrangleAA(x - w, y, tx - w * 0.6f, y - h * 0.75f, tx, y - h, x, y, GetColor(225, 245, 255), TRUE);
		DrawLineAA(x + w, y, tx + w * 0.6f, y - h * 0.75f, GetColor(110, 170, 230), 1.0f);
	}

	//いなずま
	void Bolt(float x1, float y1, float x2, float y2, float t, unsigned int color)
	{
		float px = x1;
		float py = y1;
		for (int k = 1; k <= 4; k++)
		{
			const float r = k / 4.0f;
			const float zig = (k == 4) ? 0.0f : ((k % 2) ? 1.0f : -1.0f) * 6.0f * (0.6f + 0.4f * std::sin(t * 9.0f + k));
			const float nx = x1 + (x2 - x1) * r + zig;
			const float ny = y1 + (y2 - y1) * r;
			DrawLineAA(px, py, nx, ny, color, 2.5f);
			px = nx;
			py = ny;
		}
	}

	void Halo(float x, float y, float r, unsigned int color)
	{
		DrawOvalAA(x, y, r, r * 0.3f, 20, color, FALSE, 3.0f);
		SetDrawBlendMode(DX_BLENDMODE_ADD, 90);
		DrawOvalAA(x, y, r * 1.1f, r * 0.4f, 20, color, FALSE, 5.0f);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	void AngelWings(const Vec2& c, float r, float t, unsigned int color)
	{
		const float flap = std::sin(t * 2.0f) * r * 0.15f;
		for (int side = -1; side <= 1; side += 2)
		{
			for (int k = 0; k < 3; k++)
			{
				const float len = r * (1.1f - k * 0.2f);
				DrawOvalAA(c.x + side * (r * 0.7f + len * 0.45f), c.y - r * 0.25f + k * r * 0.2f - flap * (1.0f - k * 0.3f), len * 0.5f, r * 0.16f, 16, color, TRUE);
			}
		}
	}

	void Crown(float x, float y, float w, unsigned int color)
	{
		DrawBoxAA(x - w, y - w * 0.35f, x + w, y, color, TRUE);
		for (int k = -1; k <= 1; k++)
		{
			const float px = x + k * w * 0.75f;
			DrawTriangleAA(px - w * 0.3f, y - w * 0.3f, px + w * 0.3f, y - w * 0.3f, px, y - w * (k == 0 ? 1.1f : 0.9f), color, TRUE);
			DrawCircleAA(px, y - w * (k == 0 ? 1.1f : 0.9f), w * 0.08f, 8, color, TRUE);
		}
		DrawCircleAA(x, y - w * 0.17f, w * 0.15f, 10, GetColor(230, 50, 70), TRUE);
		DrawCircleAA(x - w * 0.6f, y - w * 0.17f, w * 0.1f, 8, GetColor(60, 120, 230), TRUE);
		DrawCircleAA(x + w * 0.6f, y - w * 0.17f, w * 0.1f, 8, GetColor(60, 120, 230), TRUE);
	}

	void Aura(const Vec2& c, float r, float t, unsigned int color)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(70 + 40 * std::sin(t * 1.5f)));
		DrawCircleAA(c.x, c.y, r * 1.35f, 24, color, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		for (int k = 0; k < 4; k++)
		{
			const float a = std::fmod(t * 0.4f + k * 0.25f, 1.0f);
			const float px = c.x + std::sin(k * 2.1f) * r * 0.9f;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(180 * (1.0f - a)));
			DrawCircleAA(px, c.y + r * 0.5f - a * r * 1.6f, r * 0.12f * (1.0f - a * 0.5f), 10, color, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	/// <summary>
	/// 体の うしろに かく かざり(けはい・はね)
	/// </summary>
	void DecorBack(int id, const Vec2& feet, float size, float t)
	{
		const unsigned int snow = GetColor(250, 252, 255);
		switch (id)
		{
		case 30: Aura({ feet.x, feet.y - size * 0.55f }, size * 0.7f, t, GetColor(90, 30, 130)); break;
		case 32: Aura({ feet.x, feet.y - size * 0.8f }, size * 0.55f, t, GetColor(120, 40, 160)); break;
		case 33: Aura({ feet.x, feet.y - size * 0.5f }, size * 0.5f, t, GetColor(60, 20, 90)); break;
		case 43: Aura({ feet.x, feet.y - size * 0.75f }, size * 0.55f, t, GetColor(100, 40, 160)); break;
		case 35: AngelWings({ feet.x, feet.y - size * 0.6f }, size * 0.6f, t, snow); break;
		case 37: AngelWings({ feet.x, feet.y - size * 0.95f }, size * 0.5f, t, snow); break;
		case 44: AngelWings({ feet.x, feet.y - size * 0.95f }, size * 0.66f, t, GetColor(255, 245, 210)); break;
		case 40:
			DrawTriangleAA(feet.x - size * 0.75f, feet.y - size * 0.05f, feet.x + size * 0.75f, feet.y - size * 0.05f, feet.x, feet.y - size * 1.1f, GetColor(180, 30, 50), TRUE);
			break;
		default: break;
		}
	}

	/// <summary>
	/// 追加モンスターの かざり(IDごと)
	/// </summary>
	void Decor(int id, const Vec2& feet, float size, float t, float facing, const Vec2& top, const Vec2& body, float bodyR, const Vec2& head)
	{
		const unsigned int snow = GetColor(250, 252, 255);
		const unsigned int gold = GetColor(250, 210, 70);
		const unsigned int ice = GetColor(200, 240, 255);
		switch (id)
		{
		case 20:	//ユキプルン: ゆきの ぼうし
			DrawOvalAA(top.x, top.y + size * 0.12f, size * 0.38f, size * 0.16f, 20, snow, TRUE);
			DrawCircleAA(top.x + size * 0.05f, top.y - size * 0.08f, size * 0.12f, 12, snow, TRUE);
			for (int k = 0; k < 3; k++)
			{
				const float a = t * 0.5f + k * 2.09f;
				DrawUtil::Star(body.x + std::cos(a) * bodyR * 1.3f, body.y + std::sin(a) * bodyR * 0.5f, size * 0.07f, a, ice);
			}
			break;
		case 21:	//フブキバット: つらら と ふぶき
			for (int side = -1; side <= 1; side += 2)
			{
				for (int k = 0; k < 2; k++)
				{
					const float x = body.x + side * size * (0.55f + k * 0.2f);
					DrawTriangleAA(x - size * 0.05f, body.y + size * 0.1f, x + size * 0.05f, body.y + size * 0.1f, x, body.y + size * (0.35f - k * 0.08f), ice, TRUE);
				}
			}
			for (int k = 0; k < 5; k++)
			{
				const float a = std::fmod(t * 0.3f + k * 0.2f, 1.0f);
				DrawCircleAA(body.x - size * 0.8f + a * size * 1.6f, body.y - size * 0.5f + std::sin(k * 1.7f + a * 6.0f) * size * 0.4f, size * 0.035f, 6, snow, TRUE);
			}
			break;
		case 22:	//コオリゴーレム: かたの けっしょう
			Crystal(body.x - bodyR * 0.85f, body.y - size * 0.3f, size * 0.12f, size * 0.55f, -size * 0.1f);
			Crystal(body.x + bodyR * 0.85f, body.y - size * 0.3f, size * 0.12f, size * 0.55f, size * 0.1f);
			Crystal(top.x - size * 0.1f, top.y + size * 0.05f, size * 0.08f, size * 0.3f, 0.0f);
			break;
		case 23:	//アイスウルフ: せなかの けっしょう
			for (int k = 0; k < 3; k++)
			{
				Crystal(body.x - facing * size * (0.3f - k * 0.22f), body.y - size * 0.25f, size * 0.09f, size * (0.3f + k * 0.08f), -facing * size * 0.06f);
			}
			break;
		case 24:	//ユキノセイ: ゆきの かんむり
			for (int k = -1; k <= 1; k++) Crystal(top.x + k * size * 0.13f, top.y + size * 0.08f, size * 0.05f, size * (k == 0 ? 0.28f : 0.18f), k * size * 0.04f);
			DrawUtil::Star(body.x, body.y + size * 0.15f, size * 0.08f, t, ice);
			break;
		case 25:	//マグマプルン: ひびと あわ
			SetDrawBlendMode(DX_BLENDMODE_ADD, 220);
			DrawLineAA(body.x - bodyR * 0.6f, body.y + size * 0.2f, body.x - bodyR * 0.2f, body.y + size * 0.05f, GetColor(255, 220, 80), 2.5f);
			DrawLineAA(body.x - bodyR * 0.2f, body.y + size * 0.05f, body.x + bodyR * 0.1f, body.y + size * 0.28f, GetColor(255, 220, 80), 2.5f);
			DrawLineAA(body.x + bodyR * 0.4f, body.y - size * 0.15f, body.x + bodyR * 0.7f, body.y + size * 0.05f, GetColor(255, 220, 80), 2.5f);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			for (int k = 0; k < 3; k++)
			{
				const float a = std::fmod(t * 0.35f + k * 0.33f, 1.0f);
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(220 * (1.0f - a)));
				DrawCircleAA(top.x + (k - 1) * size * 0.2f, top.y + size * 0.1f - a * size * 0.5f, size * 0.06f, 8, GetColor(255, 150, 60), TRUE);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			}
			break;
		case 26:	//ヒクイドリ: ほのおの とさか
			Flame(top.x - facing * size * 0.05f, top.y + size * 0.08f, size * 0.09f, size * 0.32f, t);
			Flame(body.x - facing * size * 0.65f, body.y - size * 0.2f, size * 0.09f, size * 0.3f, t + 1.0f);
			break;
		case 27:	//ヨウガンゴーレム: ようがんの ひび
			SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(180 + 60 * std::sin(t)));
			DrawLineAA(body.x - bodyR * 0.5f, body.y - size * 0.3f, body.x - bodyR * 0.1f, body.y, GetColor(255, 160, 40), 3.0f);
			DrawLineAA(body.x - bodyR * 0.1f, body.y, body.x - bodyR * 0.3f, body.y + size * 0.35f, GetColor(255, 160, 40), 3.0f);
			DrawLineAA(body.x + bodyR * 0.2f, body.y - size * 0.1f, body.x + bodyR * 0.55f, body.y + size * 0.2f, GetColor(255, 160, 40), 3.0f);
			DrawLineAA(head.x - size * 0.2f, head.y - size * 0.15f, head.x + size * 0.15f, head.y - size * 0.2f, GetColor(255, 160, 40), 2.5f);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			Flame(top.x + size * 0.15f, top.y + size * 0.05f, size * 0.07f, size * 0.22f, t);
			break;
		case 28:	//カエンオオカミ: ほのおの たてがみ と しっぽ
			for (int k = 0; k < 3; k++) Flame(head.x - facing * size * (0.15f + k * 0.12f), head.y - size * 0.1f + k * size * 0.08f, size * 0.08f, size * 0.3f, t + k);
			Flame(body.x - facing * size * 0.85f, body.y - size * 0.25f, size * 0.1f, size * 0.35f, t + 2.0f);
			break;
		case 29:	//ヒノセイレイ: ほのおの かみ
			Flame(top.x, top.y + size * 0.12f, size * 0.16f, size * 0.4f, t);
			Flame(top.x - size * 0.15f, top.y + size * 0.18f, size * 0.08f, size * 0.25f, t + 1.0f);
			Flame(top.x + size * 0.15f, top.y + size * 0.18f, size * 0.08f, size * 0.25f, t + 2.0f);
			break;
		case 30:	//ヤミプルン: つの と やみの けはい
			for (int side = -1; side <= 1; side += 2)
			{
				DrawTriangleAA(body.x + side * bodyR * 0.35f, top.y + size * 0.35f, body.x + side * bodyR * 0.65f, top.y + size * 0.42f, body.x + side * bodyR * 0.7f, top.y - size * 0.05f, GetColor(40, 20, 50), TRUE);
			}
			break;
		case 31:	//シニガミバット: おおがま
		{
			const float sx = body.x + facing * size * 0.5f;
			DrawLineAA(sx, body.y + size * 0.5f, sx - facing * size * 0.1f, body.y - size * 0.7f, GetColor(110, 80, 60), size * 0.06f);
			DrawQuadrangleAA(sx - facing * size * 0.1f, body.y - size * 0.7f, sx - facing * size * 0.65f, body.y - size * 0.6f, sx - facing * size * 0.75f, body.y - size * 0.4f, sx - facing * size * 0.1f, body.y - size * 0.6f, GetColor(200, 205, 215), TRUE);
			DrawOvalAA(top.x, top.y + size * 0.15f, size * 0.3f, size * 0.12f, 16, GetColor(30, 20, 40), TRUE);
			break;
		}
		case 32:	//ノロイノヨロイ: のろいの けはい
			break;
		case 33:	//ヤミオオカミ: とげの たてがみ と あかい め
			for (int k = 0; k < 4; k++)
			{
				const float x = body.x - facing * size * (0.35f - k * 0.2f);
				DrawTriangleAA(x - size * 0.08f, body.y - size * 0.25f, x + size * 0.08f, body.y - size * 0.25f, x - facing * size * 0.06f, body.y - size * 0.55f, GetColor(40, 25, 60), TRUE);
			}
			SetDrawBlendMode(DX_BLENDMODE_ADD, 160);
			DrawCircleAA(head.x + facing * size * 0.1f, head.y - size * 0.02f, size * 0.14f, 12, GetColor(255, 40, 60), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			break;
		case 34:	//ボウレイ: くさり と ランタン
		{
			for (int k = 0; k < 5; k++)
			{
				DrawCircleAA(body.x - facing * size * (0.3f + k * 0.1f), body.y + size * (0.3f + k * 0.06f), size * 0.04f, 8, GetColor(150, 150, 160), FALSE, 2.0f);
			}
			const float lx = body.x + facing * size * 0.6f;
			const float ly = body.y + std::sin(t) * size * 0.05f;
			DrawLineAA(lx, ly - size * 0.25f, lx, ly - size * 0.1f, GetColor(80, 70, 60), 2.0f);
			DrawBoxAA(lx - size * 0.1f, ly - size * 0.1f, lx + size * 0.1f, ly + size * 0.15f, GetColor(80, 70, 60), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_ADD, 220);
			DrawCircleAA(lx, ly + size * 0.03f, size * 0.07f, 10, GetColor(120, 255, 180), TRUE);
			DrawCircleAA(lx, ly + size * 0.03f, size * 0.2f, 12, GetColor(40, 120, 80), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			break;
		}
		case 35:	//ソラプルン: てんしの はね と わっか
			Halo(top.x, top.y - size * 0.1f, size * 0.22f, gold);
			break;
		case 36:	//カミナリバット: いなずま
			Bolt(body.x - size * 0.6f, body.y - size * 0.6f, body.x - size * 0.4f, body.y + size * 0.1f, t, GetColor(255, 240, 120));
			Bolt(body.x + size * 0.6f, body.y - size * 0.6f, body.x + size * 0.4f, body.y + size * 0.1f, t + 1.0f, GetColor(255, 240, 120));
			DrawTriangleAA(top.x - size * 0.06f, top.y + size * 0.05f, top.x + size * 0.08f, top.y + size * 0.05f, top.x - size * 0.02f, top.y - size * 0.25f, GetColor(255, 230, 80), TRUE);
			break;
		case 37:	//テンシノヨロイ: はね と わっか
			Halo(top.x, top.y + size * 0.05f, size * 0.25f, gold);
			break;
		case 38:	//ライジュウ: いなずまの しっぽ
			Bolt(body.x - facing * size * 0.55f, body.y, body.x - facing * size * 0.95f, body.y - size * 0.6f, t, GetColor(255, 240, 120));
			Bolt(head.x, head.y - size * 0.3f, head.x + facing * size * 0.1f, head.y - size * 0.6f, t + 2.0f, GetColor(255, 255, 200));
			if (std::fmod(t, 3.0f) < 0.4f)
			{
				SetDrawBlendMode(DX_BLENDMODE_ADD, 120);
				DrawCircleAA(body.x, body.y, bodyR * 1.2f, 20, GetColor(255, 240, 120), TRUE);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			}
			break;
		case 39:	//ホシノセイレイ: ほしの かざり
			DrawUtil::Star(top.x, top.y - size * 0.05f, size * 0.14f, 0.0f, gold);
			for (int k = 0; k < 3; k++)
			{
				const float a = t * 0.6f + k * 2.09f;
				DrawUtil::Star(body.x + std::cos(a) * size * 0.6f, body.y + std::sin(a) * size * 0.25f, size * 0.06f, a, GetColor(255, 240, 180));
			}
			break;
		case 40:	//キングプルン: おうかん と マント
			Crown(top.x, top.y + size * 0.22f, size * 0.34f, gold);
			break;
		case 41:	//ヒョウガドラゴン: こおりの とげ と いき
			for (int k = 0; k < 3; k++) Crystal(body.x - facing * size * (0.25f - k * 0.2f), body.y - size * 0.35f, size * 0.08f, size * (0.3f + k * 0.07f), -facing * size * 0.05f);
			Crystal(top.x, top.y + size * 0.1f, size * 0.06f, size * 0.25f, -facing * size * 0.05f);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(100 + 60 * std::sin(t)));
			DrawOvalAA(head.x + facing * size * 0.45f, head.y + size * 0.05f, size * 0.2f, size * 0.1f, 16, ice, TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			break;
		case 42:	//ヨウガンマジン: もえる かた
			Flame(body.x - bodyR * 0.85f, body.y - size * 0.45f, size * 0.1f, size * 0.35f, t);
			Flame(body.x + bodyR * 0.85f, body.y - size * 0.45f, size * 0.1f, size * 0.35f, t + 1.5f);
			SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(160 + 60 * std::sin(t)));
			DrawLineAA(body.x - size * 0.2f, body.y - size * 0.3f, body.x + size * 0.05f, body.y - size * 0.05f, GetColor(255, 160, 40), 3.0f);
			DrawLineAA(body.x + size * 0.05f, body.y - size * 0.05f, body.x - size * 0.05f, body.y + size * 0.25f, GetColor(255, 160, 40), 3.0f);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			break;
		case 43:	//ヤミノシンカン: やみの けはい
			break;
		case 44:	//テンクウノオウ: きんの わっか と はね
			Halo(top.x, top.y - size * 0.05f, size * 0.25f, gold);
			Crown(head.x, head.y - size * 0.22f, size * 0.15f, gold);
			break;
		default:
			break;
		}
	}
}

//==============================//
// ドット絵の キャラ(data/gfx/chara。かざりは まえと おなじく ずけいを ドット絵に して かさねる)
//==============================//

namespace
{
	enum class Motion
	{
		Walk,	//フレームを じゅんに かえる
		Hop,	//1まい絵を ぴょこぴょこ させる
		Float,	//ういて ゆれる
	};

	struct Look
	{
		CharaArt art;
		float tint;		//体の いろ(MonsterData::color)に ぬりかえる つよさ(0は そざいの まま)
		float zoom;		//ふつうの 16ドットの 絵を 1 とした 大きさ
		Motion motion;
		float fps;		//0は 1まい目 だけ
	};

	//モンスターID の じゅんばん
	constexpr Look kLooks[] =
	{
		{ CharaArt::SlimeGreen, 0.0f, 1.0f, Motion::Walk, 4.0f },		//0 プルン
		{ CharaArt::Bat, 0.8f, 1.0f, Motion::Float, 0.0f },				//1 コウモン
		{ CharaArt::Mole, 0.45f, 1.0f, Motion::Walk, 4.0f },			//2 モグモグ
		{ CharaArt::Drop, 0.7f, 1.0f, Motion::Float, 4.0f },			//3 ヒールン
		{ CharaArt::Rock, 0.5f, 1.0f, Motion::Hop, 0.0f },				//4 イシコロ
		{ CharaArt::Flame, 0.0f, 1.0f, Motion::Float, 6.0f },			//5 ヒノタマ
		{ CharaArt::Wolf, 0.65f, 1.1f, Motion::Walk, 3.0f },			//6 ガルル
		{ CharaArt::Ghost, 0.45f, 1.0f, Motion::Float, 4.0f },			//7 ユーレイ
		{ CharaArt::SlimeBlue, 0.0f, 1.0f, Motion::Walk, 4.0f },		//8 コオリプルン
		{ CharaArt::Bat, 0.8f, 1.0f, Motion::Float, 0.0f },				//9 ヨルバット
		{ CharaArt::Golem, 0.55f, 0.9f, Motion::Walk, 3.0f },			//10 ゴロゴーレム
		{ CharaArt::Orb, 0.55f, 1.0f, Motion::Float, 4.0f },			//11 マホウダマ
		{ CharaArt::SlimeBlue, 0.8f, 1.0f, Motion::Walk, 4.0f },		//12 ドクプルン
		{ CharaArt::Wolf, 0.8f, 1.1f, Motion::Walk, 3.0f },				//13 ホムラウルフ
		{ CharaArt::Ghost, 0.6f, 1.0f, Motion::Float, 4.0f },			//14 モリノセイレイ
		{ CharaArt::DragonGreen, 0.0f, 1.0f, Motion::Walk, 4.0f },		//15 コドラ
		{ CharaArt::Golem, 0.65f, 0.9f, Motion::Walk, 3.0f },			//16 テツゴーレム
		{ CharaArt::Ghost, 0.85f, 1.0f, Motion::Float, 4.0f },			//17 カゲボウシ
		{ CharaArt::SlimeBlue, 0.85f, 1.0f, Motion::Walk, 4.0f },		//18 キラプルン
		{ CharaArt::DragonBoss, 0.7f, 0.18f, Motion::Float, 0.0f },		//19 マオウドラゴン
		{ CharaArt::SlimeBlue, 0.6f, 1.0f, Motion::Walk, 4.0f },		//20 ユキプルン
		{ CharaArt::Bat, 0.8f, 1.0f, Motion::Float, 0.0f },				//21 フブキバット
		{ CharaArt::Golem, 0.7f, 0.9f, Motion::Walk, 3.0f },			//22 コオリゴーレム
		{ CharaArt::Wolf, 0.8f, 1.1f, Motion::Walk, 3.0f },				//23 アイスウルフ
		{ CharaArt::SpiritIce, 0.0f, 1.0f, Motion::Float, 5.0f },		//24 ユキノセイ
		{ CharaArt::SlimeBlue, 0.85f, 1.0f, Motion::Walk, 4.0f },		//25 マグマプルン
		{ CharaArt::Bird, 0.45f, 1.0f, Motion::Walk, 4.0f },			//26 ヒクイドリ
		{ CharaArt::Golem, 0.75f, 0.9f, Motion::Walk, 3.0f },			//27 ヨウガンゴーレム
		{ CharaArt::Wolf, 0.8f, 1.1f, Motion::Walk, 3.0f },				//28 カエンオオカミ
		{ CharaArt::SpiritFire, 0.0f, 1.0f, Motion::Float, 5.0f },		//29 ヒノセイレイ
		{ CharaArt::SlimeBlue, 0.85f, 1.0f, Motion::Walk, 4.0f },		//30 ヤミプルン
		{ CharaArt::Bat, 0.85f, 1.0f, Motion::Float, 0.0f },			//31 シニガミバット
		{ CharaArt::ArmorDark, 0.35f, 1.0f, Motion::Hop, 0.0f },		//32 ノロイノヨロイ
		{ CharaArt::Wolf, 0.85f, 1.1f, Motion::Walk, 3.0f },			//33 ヤミオオカミ
		{ CharaArt::Ghost, 0.6f, 1.0f, Motion::Float, 4.0f },			//34 ボウレイ
		{ CharaArt::SlimeBlue, 0.55f, 1.0f, Motion::Walk, 4.0f },		//35 ソラプルン
		{ CharaArt::Bat, 0.85f, 1.0f, Motion::Float, 0.0f },			//36 カミナリバット
		{ CharaArt::ArmorHoly, 0.55f, 1.0f, Motion::Hop, 0.0f },		//37 テンシノヨロイ
		{ CharaArt::Lion, 0.25f, 1.0f, Motion::Hop, 0.0f },				//38 ライジュウ
		{ CharaArt::SpiritIce, 0.7f, 1.0f, Motion::Float, 5.0f },		//39 ホシノセイレイ
		{ CharaArt::KingSlime, 0.8f, 0.3f, Motion::Walk, 5.0f },		//40 キングプルン
		{ CharaArt::DragonBoss, 0.3f, 0.18f, Motion::Float, 0.0f },		//41 ヒョウガドラゴン
		{ CharaArt::Demon, 0.25f, 0.36f, Motion::Walk, 5.0f },			//42 ヨウガンマジン
		{ CharaArt::Priest, 0.35f, 1.0f, Motion::Hop, 0.0f },			//43 ヤミノシンカン
		{ CharaArt::DragonBoss, 0.7f, 0.18f, Motion::Float, 0.0f },		//44 テンクウノオウ
		{ CharaArt::SlimeBlue, 0.75f, 1.0f, Motion::Walk, 4.0f },		//45 モヤプルン
		{ CharaArt::Crab, 0.35f, 0.95f, Motion::Walk, 5.0f },			//46 タテガニ
		{ CharaArt::Boar, 0.3f, 1.0f, Motion::Walk, 3.0f },				//47 トッシンボア
		{ CharaArt::Doll, 0.4f, 1.0f, Motion::Hop, 0.0f },				//48 ノロイニンギョウ
		{ CharaArt::Rock, 0.65f, 1.0f, Motion::Hop, 0.0f },				//49 バクダンイワ
		{ CharaArt::Rat, 0.3f, 1.0f, Motion::Walk, 3.0f },				//50 ヌスミネズミ
		{ CharaArt::DragonBoss, 0.85f, 0.18f, Motion::Float, 0.0f },		//51 ヨミノオウ
	};
	constexpr int kLookNum = static_cast<int>(sizeof(kLooks) / sizeof(kLooks[0]));

	constexpr float kBaseSize = 34.0f;	//ダンジョンの ふつうの 大きさ。この とき 1ドットを 3ピクセルで かく(ゆかの タイルと おなじ)
	constexpr float kShinyTint = 0.8f;	//いろちがいは そざいの いろを つよく ぬりかえる

	const Look& LookOf(int id)
	{
		return kLooks[(id >= 0 && id < kLookNum) ? id : 0];
	}

	/// <summary>
	/// そざいの 1ドットを なんピクセルで かくか(大きい ときは ぼやけない ように せいすうに そろえる)
	/// </summary>
	float ArtScale(float size, float zoom)
	{
		const float s = size / kBaseSize * PixelArt::kDot * zoom;
		return s >= 1.0f ? std::floor(s + 0.5f) : s;
	}

	/// <summary>
	/// 体の 下の まんなかを feet に あわせて かく
	/// </summary>
	void DrawArt(int handle, const CharaSprite::Info& info, const Vec2& feet, float scale, bool flipX)
	{
		const float cx = (info.left + info.right) * 0.5f;
		const float x = std::floor(feet.x - (flipX ? info.frameW - cx : cx) * scale + 0.5f);
		const float y = std::floor(feet.y - info.bottom * scale + 0.5f);
		Sprite::DrawRect(handle, x, y, info.frameW * scale, info.frameH * scale, flipX);
	}

	/// <summary>
	/// いまの あかるさに かけあわせて かく(ずかんの かげ・ダメージの あか)
	/// </summary>
	class BrightScope
	{
	public:
		BrightScope(int r, int g, int b)
		{
			GetDrawBright(&r_, &g_, &b_);
			SetDrawBright(r_ * r / 255, g_ * g / 255, b_ * b / 255);
		}
		~BrightScope() { SetDrawBright(r_, g_, b_); }
	private:
		int r_ = 255;
		int g_ = 255;
		int b_ = 255;
	};

	bool HasDecorBack(int id)
	{
		return id == 30 || id == 32 || id == 33 || id == 35 || id == 37 || id == 40 || id == 43 || id == 44;
	}

	bool HasDecor(int id)
	{
		return id >= 20 && id <= 44 && id != 32 && id != 43;
	}

	bool IsSameColor(const Game::Rgb& a, const Game::Rgb& b)
	{
		return a.r == b.r && a.g == b.g && a.b == b.b;
	}
}

void UnitDraw::Hero(const Vec2& feet, float size, float time, float facing, bool isHurt, bool hasSword)
{
	const CharaSprite::Info& info = CharaSprite::Get(CharaArt::Hero);
	const float scale = ArtScale(size, 1.0f);
	const int frame = static_cast<int>(time * 4.0f) % info.frames;
	const bool flipX = facing < 0.0f;

	Shadow(feet, (info.right - info.left) * scale * 0.45f);
	BrightScope bright(255, isHurt ? 120 : 255, isHurt ? 120 : 255);
	DrawArt(CharaSprite::Frame(CharaArt::Hero, frame), info, feet, scale, flipX);

	//てに もった けん(からだの よこ)
	if (!hasSword) return;
	const CharaSprite::Info& sword = CharaSprite::Get(CharaArt::Sword);
	const Vec2 hand = { feet.x + facing * (info.right - info.left) * 0.5f * scale, feet.y - scale };
	DrawArt(CharaSprite::Frame(CharaArt::Sword, 0), sword, hand, scale, flipX);
}

float UnitDraw::FitMonsterSize(float size, const MonsterData& data, float maxHeight)
{
	const Look& look = LookOf(data.id);
	const CharaSprite::Info& info = CharaSprite::Get(look.art);
	const float spriteH = static_cast<float>(info.bottom - info.top);
	//大きさを すこしずつ へらして おさまる ところを さがす(ドットの 大きさが せいすうに そろうので とびとび になる)
	for (int k = 0; k < 24 && size > 4.0f; k++)
	{
		if (spriteH * ArtScale(size, look.zoom) <= maxHeight) break;
		size *= 0.92f;
	}
	return size;
}

void UnitDraw::Monster(const Vec2& feet, float size, const MonsterData& data, float time, float facing, bool isHurt)
{
	const Look& look = LookOf(data.id);
	const CharaSprite::Info& info = CharaSprite::Get(look.art);
	const float scale = ArtScale(size, look.zoom);
	const float t = time * 5.0f + data.id;
	const bool flipX = facing < 0.0f;

	//ずかんの かげ(まだ みていない)は まっくろで かく。かざりも つけない
	const bool isSilhouette = data.color.r < 40 && data.color.g < 40 && data.color.b < 40;

	int frame = 0;
	if (look.fps > 0.0f) frame = static_cast<int>(time * look.fps + data.id * 0.37f) % info.frames;

	//うく・はねる(ドットの 大きさ ずつ うごかす)
	float lift = 0.0f;
	if (look.motion == Motion::Float) lift = (3.0f + std::round(std::sin(time * 2.5f + data.id))) * scale;
	if (look.motion == Motion::Hop && std::fabs(std::sin(time * 3.0f + data.id)) > 0.7f) lift = scale;
	const Vec2 base = { feet.x, feet.y - lift };

	//かざりを つける いち(あたまの 上・体の まんなか・体の 大きさ・あたま)
	const float w = (info.right - info.left) * scale;
	const float h = (info.bottom - info.top) * scale;
	const Vec2 top = { base.x, base.y - h };
	const Vec2 body = { base.x, base.y - h * 0.45f };
	const float bodyR = w * 0.45f;
	const Vec2 head = { base.x + facing * w * 0.15f, base.y - h * 0.7f };

	const int dot = PixelArt::DotFor(size);
	const auto local = [&](const Vec2& f, float s, const Vec2& p) { return f + (p - feet) * s; };

	Shadow(feet, w * (look.motion == Motion::Float ? 0.35f : 0.45f));
	if (!isSilhouette && HasDecorBack(data.id))
	{
		PixelArt::Draw(feet, dot, [&](const Vec2& f, float s) { DecorBack(data.id, local(f, s, base), size * s, t); });
	}

	{
		int handle = -1;
		if (isSilhouette) handle = CharaSprite::Frame(look.art, frame);
		else
		{
			//いろちがいは そざいの いろを ぬりかえて わかる ように する
			const bool isShiny = !IsSameColor(data.color, MonsterTable::Get(data.id).color);
			const float tint = isShiny ? std::max(look.tint, kShinyTint) : look.tint;
			handle = CharaSprite::Tinted(look.art, frame, data.color, tint);
		}
		BrightScope bright(isSilhouette ? 25 : 255, isSilhouette ? 20 : (isHurt ? 120 : 255), isSilhouette ? 35 : (isHurt ? 120 : 255));
		DrawArt(handle, info, base, scale, flipX);
	}

	if (!isSilhouette && HasDecor(data.id))
	{
		PixelArt::Draw(feet, dot, [&](const Vec2& f, float s)
			{
				Decor(data.id, local(f, s, base), size * s, t, facing, local(f, s, top), local(f, s, body), bodyR * s, local(f, s, head));
			});
	}
}
