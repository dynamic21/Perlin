#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using olc::Pixel;
using olc::Key;
using olc::vi2d;
using std::to_string;

#define screenSize 100

class Example : public olc::PixelGameEngine
{
public:
	double* screen;
	unsigned int seed = (unsigned int)std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	double z;
	int octavesN;
	double frequencyW;
	double layerW;

	double Noise(uint64_t x, uint64_t y, uint64_t z, uint64_t seed)
	{
		uint64_t tmp2 = x * 0x4a39b70d;
		uint32_t tmp = (tmp2 >> 32) ^ tmp2 ^ y;
		tmp2 = (uint64_t)tmp * 0x12fad5c9;
		tmp = (tmp2 >> 32) ^ tmp2 ^ z;

		tmp2 = (uint64_t)tmp * 0x4a39b70d;
		tmp = (tmp2 >> 32) ^ tmp2 ^ seed;
		tmp2 = (uint64_t)tmp * 0x12fad5c9;
		tmp = (tmp2 >> 32) ^ tmp2;

		return double(tmp) / 0xFFFFFFFF;
	}

	double Interpolate(double a, double b, double x) { return (b - a) * (x * x * x * (x * (x * 6 - 15) + 10)) + a; }

	double InterpolatedNoise(double x, double y, double z, unsigned int seed)
	{
		int integer_X = x;
		double fractional_X = x - integer_X;
		int integer_Y = y;
		double fractional_Y = y - integer_Y;
		int integer_Z = z;
		double fractional_Z = z - integer_Z;

		double v1 = Noise(integer_X, integer_Y, integer_Z, seed),
			v2 = Noise(integer_X + 1, integer_Y, integer_Z, seed),
			v3 = Noise(integer_X, integer_Y + 1, integer_Z, seed),
			v4 = Noise(integer_X + 1, integer_Y + 1, integer_Z, seed),
			v5 = Noise(integer_X, integer_Y, integer_Z + 1, seed),
			v6 = Noise(integer_X + 1, integer_Y, integer_Z + 1, seed),
			v7 = Noise(integer_X, integer_Y + 1, integer_Z + 1, seed),
			v8 = Noise(integer_X + 1, integer_Y + 1, integer_Z + 1, seed),
			i1 = Interpolate(v1, v2, fractional_X),
			i2 = Interpolate(v3, v4, fractional_X),
			i3 = Interpolate(v5, v6, fractional_X),
			i4 = Interpolate(v7, v8, fractional_X),
			i5 = Interpolate(i1, i2, fractional_Y),
			i6 = Interpolate(i3, i4, fractional_Y);
		return Interpolate(i5, i6, fractional_Z);
	}

	double ValueNoise_2D(double x, double y, double z, unsigned int seed = 314159, int numOctaves = 8, double frequencyWeight = 1.3, double layerWeight = 1.6)
	{
		double total = 0,
			frequency = pow(frequencyWeight, numOctaves),
			weight = 1,
			sum = 0;
		x += seed;
		y += seed;
		z += seed;
		for (int i = 0; i < numOctaves; i++)
		{
			frequency /= frequencyWeight;
			total += InterpolatedNoise(x * frequency, y * frequency, z * frequency, seed) * weight;
			sum += weight;
			weight *= layerWeight;
		}
		return total / sum;
	}

	Example()
	{
		sAppName = "Example";
	}

	bool OnUserCreate() override
	{
		octavesN = 8;
		frequencyW = 1.31;
		layerW = 1.43;
		z = 0;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(Pixel(0, 0, 0));

		if (GetKey(Key::W).bPressed) octavesN++;
		if (GetKey(Key::S).bPressed) octavesN--;
		if (GetKey(Key::E).bHeld) frequencyW += fElapsedTime;
		if (GetKey(Key::D).bHeld) frequencyW -= fElapsedTime;
		if (GetKey(Key::R).bHeld) layerW += fElapsedTime;
		if (GetKey(Key::F).bHeld) layerW -= fElapsedTime;
		if (GetKey(Key::Q).bHeld) z += fElapsedTime;
		if (GetKey(Key::A).bHeld) z -= fElapsedTime;

		for (int x = 0; x < screenSize; x++)
			for (int y = 10; y < screenSize; y++)
			{
				int h = (ValueNoise_2D((double)x / 10, (double)y / 10, z, seed, octavesN, frequencyW, layerW)) * 0xff;
				Pixel color = Pixel(h, h, h);
				FillRect(x * 4, y * 4, 8, 8, color);
			}

		DrawString(0, 0, to_string(z) + "; " + to_string(octavesN) + "; " + to_string(frequencyW) + "; " + to_string(layerW), Pixel(0xff, 0xff, 0xff));

		return true;
	}
};

int main()
{
	Example demo;
	if (demo.Construct(screenSize * 5, screenSize * 5, 2, 2))
		demo.Start();
	return 0;
}