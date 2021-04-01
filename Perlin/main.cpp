#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using olc::Pixel;
using std::to_string;
using std::vector;

#define screenSize 200

class Example : public olc::PixelGameEngine
{
public:
	int screen[screenSize][screenSize];

	double Noise(int x, int y, unsigned int seed)
	{
		uint32_t tmp = ~y ^ x;
		uint64_t tmp2 = (uint64_t)tmp * 0x4a39b70d;
		tmp = (tmp2 >> 32) ^ tmp2;
		tmp2 = (uint64_t)tmp * 0x12fad5c9;
		tmp = (tmp2 >> 32) ^ tmp2 ^ ~x ^ seed;
		tmp2 = (uint64_t)tmp * 0x4a39b70d;
		tmp = (tmp2 >> 32) ^ tmp2;
		tmp2 = (uint64_t)tmp * 0x12fad5c9;
		tmp = (tmp2 >> 32) ^ tmp2 ^ ~seed ^ y;
		tmp2 = (uint64_t)tmp * 0x4a39b70d;
		tmp = (tmp2 >> 32) ^ tmp2;
		tmp2 = (uint64_t)tmp * 0x12fad5c9;
		tmp = (tmp2 >> 32) ^ tmp2;
		return double(tmp) / 0xFFFFFFFF;
	}

	/*double SmoothedNoise(int x, int y, unsigned int seed)
	{
		double side2 = (Noise(x - 2, y, seed) + Noise(x + 2, y, seed) + Noise(x, y - 2, seed) + Noise(x, y + 2, seed)) * 0 / 32;
		double side1 = (Noise(x - 1, y, seed) + Noise(x + 1, y, seed) + Noise(x, y - 1, seed) + Noise(x, y + 1, seed)) * 4 / 32;
		double corner1 = (Noise(x - 1, y - 1, seed) + Noise(x + 1, y - 1, seed) + Noise(x - 1, y + 1, seed) + Noise(x + 1, y + 1, seed)) * 0 / 32;
		double sideCorner1 = (Noise(x + 1, y + 1, seed) + Noise(x + 1, y, seed) + Noise(x + 1, y - 1, seed)) * 0 / 24;
		double center = Noise(x, y, seed) * 4 / 8;
		return center + side1 + corner1 + side2 + sideCorner1;
	}*/

	double Interpolate(double a, double b, double x) { return (b - a) * (x * x * x * (x * (x * 6 - 15) + 10)) + a; }

	double InterpolatedNoise(double x, double y, unsigned int seed)
	{
		int integer_X = x;
		double fractional_X = x - integer_X;
		int integer_Y = y;
		double fractional_Y = y - integer_Y;

		/*double v1 = SmoothedNoise(integer_X, integer_Y, seed),
			v2 = SmoothedNoise(integer_X + 1, integer_Y, seed),
			v3 = SmoothedNoise(integer_X, integer_Y + 1, seed),
			v4 = SmoothedNoise(integer_X + 1, integer_Y + 1, seed),
			i1 = Interpolate(v1, v2, fractional_X),
			i2 = Interpolate(v3, v4, fractional_X);*/

		double v1 = Noise(integer_X, integer_Y, seed),
			v2 = Noise(integer_X + 1, integer_Y, seed),
			v3 = Noise(integer_X, integer_Y + 1, seed),
			v4 = Noise(integer_X + 1, integer_Y + 1, seed),
			i1 = Interpolate(v1, v2, fractional_X),
			i2 = Interpolate(v3, v4, fractional_X);
		return Interpolate(i1, i2, fractional_Y);
	}

	double ValueNoise_2D(double x, double y, unsigned int seed = 0, int numOctaves = 8, double persistence = 0.5)
	{
		double total = 0,
			amplitude = 1;
		unsigned int frequency = 1 << numOctaves;
		for (int i = 0; i < numOctaves; ++i)
		{
			frequency >>= 1;
			amplitude *= persistence;
			total += InterpolatedNoise(x / frequency, y / frequency, seed) * amplitude;
		}
		return total;
	}

	Example()
	{
		sAppName = "Example";
	}

	bool OnUserCreate() override
	{
		unsigned int seed = (unsigned int)std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		for (int x = 0; x < ScreenWidth(); x++)
			for (int y = 0; y < ScreenHeight(); y++)
			{
				screen[x][y] = ((ValueNoise_2D(x * 10, y * 10, seed))) * 0xff;
			}
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		for (int x = 0; x < ScreenWidth(); x++)
			for (int y = 0; y < ScreenHeight(); y++)
			{
				int h = screen[x][y];
				Pixel color = Pixel(h, h, h);
				Draw(x, y, color);
			}
		return true;
	}
};

int main()
{
	Example demo;
	if (demo.Construct(screenSize, screenSize, 5, 5))
		demo.Start();
	return 0;
}