#pragma once
#include"Game.h"

/// <summary>
/// キャラの ドット絵(data/gfx/chara。itch.io の CC0 アセット。くわしくは CREDITS.txt)
/// 1まいの 画像に 正面むきの フレームが 横に ならんでいる
/// </summary>
enum class CharaArt
{
	Hero,
	Sword,
	SlimeBlue,
	SlimeGreen,
	KingSlime,
	Bat,
	Mole,
	Drop,
	Rock,
	Flame,
	Wolf,
	Lion,
	Ghost,
	Golem,
	Orb,
	DragonGreen,
	DragonBoss,
	Bird,
	ArmorDark,
	ArmorHoly,
	SpiritIce,
	SpiritFire,
	Demon,
	Priest,
	Crab,
	Boar,
	Doll,
	Rat,
	Num,
};

namespace CharaSprite
{
	/// <summary>
	/// 1まいの 画像の じょうほう(体の はんいは すべての フレームを あわせた もの)
	/// </summary>
	struct Info
	{
		int frameW = 0;
		int frameH = 0;
		int frames = 1;
		int left = 0;	//体が ある はんい(left〜right-1, top〜bottom-1)
		int top = 0;
		int right = 0;
		int bottom = 0;
	};

	bool Init();
	void Terminate();

	const Info& Get(CharaArt art);

	/// <summary>
	/// そのままの フレーム
	/// </summary>
	int Frame(CharaArt art, int frame);

	/// <summary>
	/// いろを ぬりかえた フレーム(あかるさの もようと くらい ふちは のこす。おなじ ものは つくりおき)
	/// </summary>
	/// <param name="strength">0〜1(1で ほぼ その いろ)</param>
	int Tinted(CharaArt art, int frame, const Game::Rgb& color, float strength);
}
