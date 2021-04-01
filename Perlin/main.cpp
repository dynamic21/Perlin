#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using olc::Pixel;
using olc::Key;
using olc::vi2d;
using std::to_string;

#define screenSize 200
#define keyLag 0.15

class Example : public olc::PixelGameEngine
{
public:
	double* screen;
	vi2d pos;
	double keyTimer[4];

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

	double Interpolate(double a, double b, double x) { return (b - a) * (x * x * x * (x * (x * 6 - 15) + 10)) + a; }

	double InterpolatedNoise(double x, double y, unsigned int seed)
	{
		int integer_X = x;
		double fractional_X = x - integer_X;
		int integer_Y = y;
		double fractional_Y = y - integer_Y;

		double v1 = Noise(integer_X, integer_Y, seed),
			v2 = Noise(integer_X + 1, integer_Y, seed),
			v3 = Noise(integer_X, integer_Y + 1, seed),
			v4 = Noise(integer_X + 1, integer_Y + 1, seed),
			i1 = Interpolate(v1, v2, fractional_X),
			i2 = Interpolate(v3, v4, fractional_X);
		return Interpolate(i1, i2, fractional_Y);
	}

	double ValueNoise_2D(double x, double y, unsigned int seed = 0, int numOctaves = 8)
	{
		double total = 0;
		unsigned int frequency = 1,
			weight = 1 << numOctaves,
			sum = 0;
		for (int i = 0; i < numOctaves; i++)
		{
			weight >>= 1;
			total += InterpolatedNoise(x * frequency, y * frequency, seed) * weight;
			frequency <<= 1;
			sum += weight;
		}
		return total / sum;
	}

	Example()
	{
		sAppName = "Example";
	}

	bool OnUserCreate() override
	{
		screen = new double[screenSize * screenSize];
		unsigned int seed = (unsigned int)std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
		for (int x = 0; x < screenSize; x++)
			for (int y = 0; y < screenSize; y++)
				screen[x + y * screenSize] = ValueNoise_2D((double)x / 10, (double)y / 10, seed, 3);
		pos = { screenSize / 2, screenSize / 2 };
		while (!(screen[pos.x + pos.y * screenSize] > 0.5))
		{
			int x = Noise(0, 0, seed);
			int seed = Noise(0, 0, x);
			pos += {x, seed};
		}
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(Key::W).bHeld || GetKey(Key::UP).bHeld)
		{
			keyTimer[0] += fElapsedTime;
			if (keyTimer[0] > keyLag)
			{
				keyTimer[0] -= keyLag;
				if (screen[pos.x + screenSize * (pos.y - 1)])
				{
					pos.y--;
				}
				else if (screen[pos.x + screenSize * (pos.y - 2)]) {
					screen[pos.x + screenSize * (pos.y - 2)] = 0;
					screen[pos.x + screenSize * (pos.y - 1)] = 1;
					pos.y--;
				}
			}
		}
		else { keyTimer[0] = keyLag; }
		if (GetKey(Key::A).bHeld || GetKey(Key::LEFT).bHeld)
		{
			keyTimer[1] += fElapsedTime;
			if (keyTimer[1] > keyLag)
			{
				keyTimer[1] -= keyLag;
				if (screen[pos.x - 1 + screenSize * pos.y])
				{
					pos.x--;
				}
				else if (screen[pos.x - 2 + screenSize * pos.y]) {
					screen[pos.x - 2 + screenSize * pos.y] = 0;
					screen[pos.x - 1 + screenSize * pos.y] = 1;
					pos.x--;
				}
			}
		}
		else { keyTimer[1] = keyLag; }
		if (GetKey(Key::S).bHeld || GetKey(Key::DOWN).bHeld)
		{
			keyTimer[2] += fElapsedTime;
			if (keyTimer[2] > keyLag)
			{
				keyTimer[2] -= keyLag;
				if (screen[pos.x + screenSize * (pos.y + 1)])
				{
					pos.y++;
				}
				else if (screen[pos.x + screenSize * (pos.y + 2)]) {
					screen[pos.x + screenSize * (pos.y + 2)] = 0;
					screen[pos.x + screenSize * (pos.y + 1)] = 1;
					pos.y++;
				}
			}
		}
		else { keyTimer[2] = keyLag; }
		if (GetKey(Key::D).bHeld || GetKey(Key::RIGHT).bHeld)
		{
			keyTimer[3] += fElapsedTime;
			if (keyTimer[3] > keyLag)
			{
				keyTimer[3] -= keyLag;
				if (screen[pos.x + 1 + screenSize * pos.y])
				{
					pos.x++;
				}
				else if (screen[pos.x + 2 + screenSize * pos.y]) {
					screen[pos.x + 2 + screenSize * pos.y] = 0;
					screen[pos.x + 1 + screenSize * pos.y] = 1;
					pos.x++;
				}
			}
		}
		else { keyTimer[3] = keyLag; }

		for (int x = 0; x < ScreenWidth(); x++)
			for (int y = 0; y < ScreenHeight(); y++)
			{
				int h = screen[x + y * screenSize] * 0xff;
				Pixel color = Pixel(h, h, h);
				Draw(x, y, color);
			}
		DrawString(0, 0, to_string(keyTimer[0]), Pixel(0xff, 0, 0));
		Draw(pos, Pixel(0, 0xff, 0));
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