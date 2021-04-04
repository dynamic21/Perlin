#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using olc::Pixel;
using olc::Key;
using olc::vd2d;
using std::to_string;

using std::cout;
using std::endl;

using std::chrono::duration_cast;
using std::chrono::seconds;
using std::chrono::microseconds;
using std::chrono::high_resolution_clock;

unsigned int m_z = (unsigned int)duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()).count();
unsigned int m_w = (unsigned int)duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();

#define screenSize 200
#define friction 0.1

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

class Example : public olc::PixelGameEngine
{
public:
	double* screen;
	unsigned int seed;
	vd2d pos;
	vd2d posv;

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
		int integer_X = floor(x),
			integer_Y = floor(y),
			integer_Z = floor(z);

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

	double ValueNoise_2D(double x, double y, double z, unsigned int seed = intRand(), int numOctaves = 8, double frequencyWeight = 1.3, double layerWeight = 1.5)
	{
		double total = 0,
			frequency = pow(frequencyWeight, numOctaves - 1),
			weight = 1,
			sum = 0;

		for (int i = 0; i < numOctaves; i++)
		{
			total += InterpolatedNoise(x * frequency, y * frequency, z * frequency, seed) * weight;

			sum += weight;
			weight *= layerWeight;
			frequency /= frequencyWeight;

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
		seed = intRand();
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(Key::SPACE).bPressed) { seed = intRand(); }

		if (GetKey(Key::W).bHeld || GetKey(Key::UP).bHeld) { posv.y -= fElapsedTime * 10; }
		if (GetKey(Key::A).bHeld || GetKey(Key::LEFT).bHeld) { posv.x -= fElapsedTime * 10; }
		if (GetKey(Key::S).bHeld || GetKey(Key::DOWN).bHeld) { posv.y += fElapsedTime * 10; }
		if (GetKey(Key::D).bHeld || GetKey(Key::RIGHT).bHeld) { posv.x += fElapsedTime * 10; }

		posv *= pow(friction, fElapsedTime);
		pos += posv;

		Clear(Pixel(0, 0, 0));

		for (int x = 0; x < screenSize; x++)
			for (int y = 0; y < screenSize; y++)
			{
				int h = (ValueNoise_2D((x + pos.x) / 50, (y + pos.y) / 50, 0, seed)) * 0xff;
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