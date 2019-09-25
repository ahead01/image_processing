#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"


int read_bmp(char* path, struct BMP_FILE* image){
	FILE *fp;
	unsigned char headerData[54];
	int byteVal;
	int color_table_size;
	int image_pixels_size;
	int index;

	fp = fopen(path, "rb");
	byteVal = fgetc(fp);
	/* read in the header */
	index = 0;
	while(byteVal != EOF && index < 54){
		headerData[index] = byteVal;
		index++;
		byteVal =fgetc(fp);
		//printf(" %d %x ", index, byteVal);
	}


	//image->bmp_header.header             = headerData[0]  + headerData[1]  * 256;
	image->bmp_header.header[0]             = headerData[0];
	image->bmp_header.header[1]             = headerData[1];
	image->bmp_header.file_size          = headerData[2]  + headerData[3]  * 256 + headerData[4]  * 65536 + headerData[5]  * 16777216;
	image->bmp_header.res1               = headerData[6]  + headerData[7]  * 256;
	image->bmp_header.res2               = headerData[8]  + headerData[9]  * 256;
	image->bmp_header.image_offset       = headerData[10] + headerData[11] * 256 + headerData[12] * 65536 + headerData[13] * 16777216;
									     
	image->dib_header.dib_header_size    = headerData[14] + headerData[15] * 256 + headerData[16] * 65536 + headerData[17] * 16777216;
	image->dib_header.image_width        = headerData[18] + headerData[19] * 256 + headerData[20] * 65536 + headerData[21] * 16777216;
	image->dib_header.image_height       = headerData[22] + headerData[23] * 256 + headerData[24] * 65536 + headerData[25] * 16777216;
	image->dib_header.color_planes       = headerData[26] + headerData[27] * 256;
	image->dib_header.bits_per_pixel     = headerData[28] + headerData[29] * 256;
	image->dib_header.compression        = headerData[30] + headerData[31] * 256 + headerData[32] * 65536 + headerData[33] * 16777216;
	image->dib_header.raw_image_size     = headerData[34] + headerData[35] * 256 + headerData[36] * 65536 + headerData[37] * 16777216;
	image->dib_header.hres               = headerData[38] + headerData[39] * 256 + headerData[40] * 65536 + headerData[41] * 16777216;
	image->dib_header.vres               = headerData[42] + headerData[43] * 256 + headerData[44] * 65536 + headerData[45] * 16777216;
	image->dib_header.color_palette_size = headerData[46] + headerData[47] * 256 + headerData[48] * 65536 + headerData[49] * 16777216;
	image->dib_header.important_colors   = headerData[50] + headerData[51] * 256 + headerData[52] * 65536 + headerData[53] * 16777216;

	print_image(image);

	color_table_size = image->bmp_header.image_offset - ( image->dib_header.dib_header_size + 14);
	printf("\nColor table size %d\n", color_table_size);
	image_pixels_size = image->bmp_header.file_size - image->bmp_header.image_offset;
	printf("\nImage pixels size %d\n", image_pixels_size);

	image->image_pixels = (unsigned char*)malloc(image_pixels_size);
	image->color_table  = (unsigned char*)malloc(color_table_size);
	
	fseek(fp,14 + image->dib_header.dib_header_size,SEEK_SET);
	byteVal = fgetc(fp);
	index = 0;

	/* Read in the color table*/
	while( byteVal != EOF && index < color_table_size){
		image->color_table[index] = byteVal;
		index++;
		byteVal = fgetc(fp);
	}

	printf("\nColor Table Size %d", index);

	fseek(fp,image->bmp_header.image_offset,SEEK_SET);
    
	byteVal = fgetc(fp);
	index = 0;

	/* Read in the rest of the image */
	while(byteVal != EOF && index < image_pixels_size){
		image->image_pixels[index] = byteVal;
		index++;
		byteVal = fgetc(fp);
	}

	fclose(fp);

	printf("\nImage Data Size %d", index);
	printf("\n");
	return 0;

}

int write_bmp(char* path, struct BMP_FILE* image){
	struct BMP_FILE_BYTES bytes;
	FILE* fp;

	fp = fopen(path, "wb");
	/* BMP Header */
	fwrite(&image->bmp_header.header,sizeof(bytes.bmp_header.header),1,fp);
	fwrite(&image->bmp_header.file_size,sizeof(bytes.bmp_header.file_size),1,fp);
	fwrite(&image->bmp_header.res1,sizeof(bytes.bmp_header.res1),1,fp);
	fwrite(&image->bmp_header.res2,sizeof(bytes.bmp_header.res2),1,fp);
	fwrite(&image->bmp_header.image_offset,sizeof(bytes.bmp_header.image_offset),1,fp);

	/* DIB Header */
	fwrite(&image->dib_header.dib_header_size,sizeof(bytes.dib_header.dib_header_size),1,fp);
	fwrite(&image->dib_header.image_width,sizeof(bytes.dib_header.image_width),1,fp);
	fwrite(&image->dib_header.image_height,sizeof(bytes.dib_header.image_height),1,fp);
	fwrite(&image->dib_header.color_planes,sizeof(bytes.dib_header.color_planes),1,fp);
	fwrite(&image->dib_header.bits_per_pixel,sizeof(bytes.dib_header.bits_per_pixel),1,fp);
	fwrite(&image->dib_header.compression,sizeof(bytes.dib_header.compression),1,fp);
	fwrite(&image->dib_header.raw_image_size,sizeof(bytes.dib_header.raw_image_size),1,fp);
	fwrite(&image->dib_header.hres,sizeof(bytes.dib_header.hres),1,fp);
	fwrite(&image->dib_header.vres,sizeof(bytes.dib_header.vres),1,fp);
	fwrite(&image->dib_header.color_palette_size,sizeof(bytes.dib_header.color_palette_size),1,fp);
	fwrite(&image->dib_header.important_colors,sizeof(bytes.dib_header.important_colors),1,fp);

	/* Color Table */
	fwrite(image->color_table,(image->bmp_header.image_offset - (14 + image->dib_header.dib_header_size)),1,fp);

	/* Image Pixels */
	fwrite(image->image_pixels,(image->bmp_header.file_size - image->bmp_header.image_offset),1,fp);

	fclose(fp);

	return 0;

}

void new_color_map(struct BMP_FILE* image){
    int index = 0;
    int image_size = image->bmp_header.file_size - image->bmp_header.image_offset;

    while(index < image_size){
        image->image_pixels[index] = image->color_table[image->image_pixels[index] * 4];
        index++;
    }
    index = 0;
    while(index < image->dib_header.color_palette_size){
        image->color_table[index * 4] = index;
        image->color_table[(index * 4) + 1] = index;
        image->color_table[(index * 4) + 2] = index;
        image->color_table[(index * 4) + 3] = 255;
        index++;
    }

}

void print_image(struct BMP_FILE* image){

	printf("Image header %c%c \n",image->bmp_header.header[0], image->bmp_header.header[1]);
	printf("Image file size %d \n",image->bmp_header.file_size);
	printf("Image res1 %d \n",image->bmp_header.res1);
	printf("Image res2 %d \n",image->bmp_header.res2);
	printf("Image pixel offset %d \n",image->bmp_header.image_offset);

	printf("\nDIB Header Size %d", image->dib_header.dib_header_size);
	printf("\nImage Width %d", image->dib_header.image_width);
	printf("\nImage height %d", image->dib_header.image_height);
	printf("\nColor Planes %d", image->dib_header.color_planes);
	printf("\nBits Per Pixel %d", image->dib_header.bits_per_pixel);
	printf("\nCompression %d", image->dib_header.compression);
	printf("\nRaw image size %d", image->dib_header.raw_image_size);
	printf("\nHRES %d", image->dib_header.hres);
	printf("\nVRES %d", image->dib_header.vres);
	printf("\nColor palette size %d", image->dib_header.color_palette_size);
	printf("\nImportant Colors %d", image->dib_header.important_colors);
	printf("\n");

}

void close_image(struct BMP_FILE* image){
	free(image->color_table);
	free(image->image_pixels);
	printf("\nImage closed\n");
}
