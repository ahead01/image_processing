#pragma once

#ifndef BMP_H
#define BMP_H

	struct BMP_HEADER {
		unsigned char header[2];
		int file_size;
		short res1;
		short res2;
		int image_offset;
	};

	struct BMP_DIB_HEADER {
		int dib_header_size;
		int image_width;
		int image_height;
		short color_planes;
		short bits_per_pixel;
		int compression;
		int raw_image_size;
		int hres;
		int vres;
		int color_palette_size;
		int important_colors;
	};

	struct BMP_FILE {
		struct BMP_HEADER bmp_header;
		struct BMP_DIB_HEADER dib_header;
		unsigned char* color_table;
		unsigned char* image_pixels;
	};

	struct BMP_HEADER_BYTES {
		char header[2];
		char file_size[4];
		char res1[2];
		char res2[2];
		char image_offset[4];
	};

	struct BMP_DIB_HEADER_BYTES {
		char dib_header_size[4];
		char image_width[4];
		char image_height[4];
		char color_planes[2];
		char bits_per_pixel[2];
		char compression[4];
		char raw_image_size[4];
		char hres[4];
		char vres[4];
		char color_palette_size[4];
		char important_colors[4];
	};

	struct BMP_FILE_BYTES {
		struct BMP_HEADER_BYTES bmp_header;
		struct BMP_DIB_HEADER_BYTES dib_header;
	};

	int read_bmp(char* path, struct BMP_FILE* image);
	int write_bmp(char* path, struct BMP_FILE* image);
	void print_image(struct BMP_FILE* image);
	void close_image(struct BMP_FILE* image);
    void new_color_map(struct BMP_FILE* image);

#endif