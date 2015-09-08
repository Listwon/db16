#include <vector>
#include <iostream>
#include <string>
#include "lodepng.h"
#include <cstdlib>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#include "stb_image.h"

typedef struct {
	unsigned w;
	unsigned h;
	unsigned format; //1 = Y, 2 = YA, 3 = RGB, 4 = RGBA
	std::vector<unsigned char> data;
} image_v;

inline unsigned char clamp_color(double x) {
	return x < 0 ? 0 : (x > 255 ? 255 : (unsigned char)x);
}

int main(int argc, char *argv[]) {
	char* filename = argc > 1 ? argv[1] : "db16fs.png";

	char *pfile;
	pfile = filename + strlen(filename);
	for (; pfile > filename; pfile--)
	{
		if ((*pfile == '\\') || (*pfile == '/'))
		{
			pfile++;
			break;
		}
	}

	std::string fullname = pfile;
	std::string output = "db16fs_" + fullname.substr(0, fullname.find_last_of(".")) + ".png";

	const unsigned char db16[] = {
		20, 12, 28,
		68, 36, 52,
		48, 52, 109,
		78, 74, 78,
		133, 76, 48,
		52, 101, 36,
		208, 70, 72,
		117, 113, 97,
		89, 125, 206,
		210, 125, 44,
		133, 149, 161,
		109, 170, 44,
		210, 170, 153,
		109, 194, 202,
		218, 212, 94,
		222, 238, 214
	};

	enum RGBA { R, G, B, A };

	image_v img;
	unsigned error;
	if (argc < 2) {
		img.w = 256;
		img.h = 256;
		img.format = 4;
		img.data.resize(img.w*img.h * 4);
		for (unsigned y = 0; y < img.h; y++) {
			for (unsigned x = 0; x < img.w; x++) {
				img.data[img.format * (img.w * y + x) + RGBA::R] = 255 * !(x & y);
				img.data[img.format * (img.w * y + x) + RGBA::G] = x ^ y;
				img.data[img.format * (img.w * y + x) + RGBA::B] = x | y;
				img.data[img.format * (img.w * y + x) + RGBA::A] = 255;
			}
		}
	}
	else {
		int w, h, format;
		unsigned char * data = stbi_load(filename, &w, &h, &format, 0);
		img.w = (unsigned)w;
		img.h = (unsigned)h;
		img.format = (unsigned)format;
		img.data.insert(img.data.end(), &data[0], &data[w*h*format]);
	}

	int diff16[16] = {};
	int min, cindex, rerr, gerr, berr;
	
	for (unsigned y = 0; y < img.h; y++) {
		for (unsigned x = 0; x < img.w; x++) {
			min = 10000000;
			cindex = 0;
			rerr = 0;
			gerr = 0;
			berr = 0;
			for (int c = 0; c < 16; c++) {
				rerr = img.data[img.format * (img.w * y + x) + RGBA::R] - db16[c * 3 + RGBA::R];
				gerr = img.data[img.format * (img.w * y + x) + RGBA::G] - db16[c * 3 + RGBA::G];
				berr = img.data[img.format * (img.w * y + x) + RGBA::B] - db16[c * 3 + RGBA::B];
				diff16[c] = rerr*rerr + gerr*gerr + berr*berr;

				if (diff16[c] < min) {
					min = diff16[c];
					cindex = c;
				}
			}

			rerr = img.data[img.format * (img.w * y + x) + RGBA::R] - db16[cindex * 3 + RGBA::R];
			gerr = img.data[img.format * (img.w * y + x) + RGBA::G] - db16[cindex * 3 + RGBA::G];
			berr = img.data[img.format * (img.w * y + x) + RGBA::B] - db16[cindex * 3 + RGBA::B];

			double alpha = 0.4375, beta = 0.0625, gamma = 0.3125, delta = 0.1875;
			if (x < img.w - 1) {
				//alpha
				img.data[img.format * (img.w * y + x + 1) + RGBA::R] = clamp_color(img.data[img.format * (img.w * y + x + 1) + RGBA::R] + rerr * alpha);
				img.data[img.format * (img.w * y + x + 1) + RGBA::G] = clamp_color(img.data[img.format * (img.w * y + x + 1) + RGBA::G] + gerr * alpha);
				img.data[img.format * (img.w * y + x + 1) + RGBA::B] = clamp_color(img.data[img.format * (img.w * y + x + 1) + RGBA::B] + berr * alpha);
			}

			if (y < img.h - 1) {
				//beta, gamma, delta
				if (x < img.w - 1) {
					img.data[img.format * (img.w * (y + 1) + x + 1) + RGBA::R] = clamp_color(img.data[img.format * (img.w * (y + 1) + x + 1) + RGBA::R] + rerr * beta);
					img.data[img.format * (img.w * (y + 1) + x + 1) + RGBA::G] = clamp_color(img.data[img.format * (img.w * (y + 1) + x + 1) + RGBA::G] + gerr * beta);
					img.data[img.format * (img.w * (y + 1) + x + 1) + RGBA::B] = clamp_color(img.data[img.format * (img.w * (y + 1) + x + 1) + RGBA::B] + berr * beta);
				}

				img.data[img.format * (img.w * (y + 1) + x) + RGBA::R] = clamp_color(img.data[img.format * (img.w * (y + 1) + x) + RGBA::R] + rerr * gamma);
				img.data[img.format * (img.w * (y + 1) + x) + RGBA::G] = clamp_color(img.data[img.format * (img.w * (y + 1) + x) + RGBA::G] + gerr * gamma);
				img.data[img.format * (img.w * (y + 1) + x) + RGBA::B] = clamp_color(img.data[img.format * (img.w * (y + 1) + x) + RGBA::B] + berr * gamma);

				if (x > 0) {
					img.data[img.format * (img.w * (y + 1) + x - 1) + RGBA::R] = clamp_color(img.data[img.format * (img.w * (y + 1) + x - 1) + RGBA::R] + rerr * delta);
					img.data[img.format * (img.w * (y + 1) + x - 1) + RGBA::G] = clamp_color(img.data[img.format * (img.w * (y + 1) + x - 1) + RGBA::G] + gerr * delta);
					img.data[img.format * (img.w * (y + 1) + x - 1) + RGBA::B] = clamp_color(img.data[img.format * (img.w * (y + 1) + x - 1) + RGBA::B] + berr * delta);
				}
			}
			img.data[img.format * (img.w * y + x) + RGBA::R] = db16[cindex * 3 + RGBA::R];
			img.data[img.format * (img.w * y + x) + RGBA::G] = db16[cindex * 3 + RGBA::G];
			img.data[img.format * (img.w * y + x) + RGBA::B] = db16[cindex * 3 + RGBA::B];
			if (img.format == 4) {
				img.data[img.format * img.w * y + img.format * x + RGBA::A] = 255;
			}
		}
	}

	//Encode the image
	if (img.format == 4) {
		error = lodepng::encode(output, img.data, img.w, img.h, LCT_RGBA);
	}
	else if (img.format == 3) {
		error = lodepng::encode(output, img.data, img.w, img.h, LCT_RGB);		
	}
	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	
	return 0;
}