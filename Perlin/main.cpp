#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using olc::Pixel;
using olc::Key;
using olc::vd2d;
using olc::vi2d;
using std::to_string;

using std::cout;
using std::endl;
using std::max;
using std::min;

using std::chrono::duration_cast;
using std::chrono::seconds;
using std::chrono::microseconds;
using std::chrono::high_resolution_clock;

#define screenSize 300
#define halfScreenSize screenSize / 2
#define friction 0.1

class Example : public olc::PixelGameEngine
{
public:
	unsigned int m_z;
	unsigned int m_w;
	unsigned int seed;
	double* screen;
	double zoom;
	vd2d pos;
	vd2d posv;

	unsigned int intRand()
	{
		m_z = 36969 * (m_z & 65535) + (m_z >> 16);
		m_w = 18000 * (m_w & 65535) + (m_w >> 16);
		return (m_z << 16) + m_w;
	}

	unsigned int doubleRand()
	{
		return (intRand() + 1.0) * 2.328306435454494e-10;
	}

	Pixel mapToRainbow(double d) {
		double r = (d > 3) ? max(0.0, std::min(1.0, d - 4)) : max(0.0, std::min(1.0, 2 - d));
		double g = (d > 2) ? max(0.0, std::min(1.0, 4 - d)) : max(0.0, std::min(1.0, d));
		double b = (d > 4) ? max(0.0, std::min(1.0, 6 - d)) : max(0.0, std::min(1.0, d - 2));
		return Pixel(r * 0xff, g * 0xff, b * 0xff);
	}

	Pixel mapToBAndW(double d) {
		d = d > 0.5;
		return Pixel(d * 0xff, d * 0xff, d * 0xff);
	}

	double Noise(int x, int y, int z, unsigned int seed)
	{
		uint64_t tmp2 = (uint64_t)x * 0x4a39b70d;
		uint32_t tmp = (tmp2 >> 32) ^ tmp2 ^ y;

		tmp2 = (uint64_t)tmp * 0x12fad5c9 ^ z;
		tmp = (tmp2 >> 32) ^ tmp2 ^ seed;

		tmp2 = (uint64_t)tmp * 0x4a39b70d;
		tmp = (tmp2 >> 32) ^ tmp2;

		return (tmp + 1.0) * 2.328306435454494e-10;
	}

	double Interpolate(double a, double b, double x) { return (b - a) * (x * x * x * (x * (x * 6 - 15) + 10)) + a; }

	double InterpolatedNoise(double x, double y, double z, unsigned int seed)
	{
		int integer_X = x - (x < 0),
			integer_Y = y - (y < 0),
			integer_Z = z - (z < 0);

		double fractional_X = x - integer_X,
			fractional_Y = y - integer_Y,
			fractional_Z = z - integer_Z;

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

	double ValueNoise_2D(double x, double y, double z, unsigned int seed, int numOctaves = 4)
	{
		double total = 0;
		unsigned int weight = 1,
			frequency = 1 << numOctaves,
			sum = 0;

		for (int i = 0; i < numOctaves; i++)
		{
			total += InterpolatedNoise(x * frequency, y * frequency, z * frequency, seed) * weight;

			frequency >>= 1;
			sum += weight;
			weight <<= 1;

			seed ^= seed << 13;
			seed ^= seed >> 17;
			seed ^= seed << 5;
		}
		return total / sum;
	}

	Example()
	{
		sAppName = "Example";
	}

	bool OnUserCreate() override
	{
		m_z = (unsigned int)duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count();
		m_w = (unsigned int)duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
		zoom = (double)1 / 32;
		seed = intRand();

		DrawString(15, 2, "(Press {Space} to change the seed)  (Use {Q, E} to zoom in and out)  (Use {WASD, Arrow Keys} to move arround)");
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(Key::SPACE).bPressed) { seed = intRand(); }

		if (GetKey(Key::Q).bHeld) { zoom *= pow(2, fElapsedTime); }
		if (GetKey(Key::E).bHeld) { zoom /= pow(2, fElapsedTime); }

		if (GetKey(Key::W).bHeld || GetKey(Key::UP).bHeld) { posv.y -= fElapsedTime * zoom * 0xff; }
		if (GetKey(Key::A).bHeld || GetKey(Key::LEFT).bHeld) { posv.x -= fElapsedTime * zoom * 0xff; }
		if (GetKey(Key::S).bHeld || GetKey(Key::DOWN).bHeld) { posv.y += fElapsedTime * zoom * 0xff; }
		if (GetKey(Key::D).bHeld || GetKey(Key::RIGHT).bHeld) { posv.x += fElapsedTime * zoom * 0xff; }

		posv *= pow(friction, fElapsedTime);
		pos += posv * fElapsedTime;

		for (int x = 0; x < screenSize; x++)
			for (int y = 4; y < screenSize; y++)
			{
				double value = ValueNoise_2D((x - halfScreenSize) * zoom + pos.x, (y - halfScreenSize) * zoom + pos.y, 0, seed);
				FillRect(vi2d(x, y) * 3, vi2d(3, 3), mapToRainbow(6 * value));
				//FillRect(vi2d(x, y) * 3, vi2d(3, 3), mapToBAndW(value));
			}

		return true;
	}
};

int main()
{
	Example demo;
	if (demo.Construct(screenSize * 3, screenSize * 3, 1, 1))
		demo.Start();
	return 0;
}