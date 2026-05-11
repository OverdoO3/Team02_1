#pragma once

//浮動小数算術
class MathUtils
{
public:
	//指定の範囲のランダム値を計算する
	static float RandomRangeFloat(float min, float max);
	//指定の範囲のランダム値を計算する
	static int RandomRangeInt(int min, int max);
};