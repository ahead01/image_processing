#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

int main(){
	char orig_path[256] = "C:/Users/Austin/Pictures/jaffe/jaffe-bmp/KA.AN1.39.bmp";
	char new_path[256] =  "C:/Users/Austin/Pictures/jaffe/jaffe-bmp/KA.AN1.39-new.bmp";
	char log_path[256] =  "C:/Users/Austin/Pictures/jaffe/jaffe-bmp/KA.AN1.39-log.txt";

	struct BMP_FILE image;


	printf("\nStarting reading file\n");
	read_bmp(orig_path, &image);
	printf("\nFinished reading file\n");

	new_color_map(&image);

	printf("\nStarting writing file\n");
	write_bmp(new_path, &image);
	printf("\nFinished writing file\n");

	close_image(&image);

	return 0;
}