#include "mathUtils.h"
#include <stdlib.h>
#include <random>

//指定範囲のランダム値を計算する
float MathUtils::RandomRangeFloat(float min, float max)
{
	std::random_device dc;
	std::mt19937 rd(dc());
	std::uniform_real_distribution distf(min, max);


	return distf(rd);
}

int MathUtils::RandomRangeInt(int min, int max)
{
	std::random_device dc;
	std::mt19937 rd(dc());
	std::uniform_int_distribution<int> disti(static_cast<int>(min), static_cast<int>(max));


	return disti(rd);
}
