/***************************************************************************
* =========================================================================
*
*   Project 2 Austin Dase & Justine Caylor
* 
* =========================================================================
*
*             File Name: proj2.c 
*           Description: Facial feature extraction for emotion classifiaction 
*   Initial Coding Date: November 22, 2018
*	   Last update Date: November 27, 2018
*           Portability: Standard (ANSI) C
*             Credit(s): Austin Dase & Justine Caylor; Towson University
*
****************************************************************************/

/*
** include header files
*/

#include "CVIPtoolkit.h"
#include "CVIPconvert.h"
#include "CVIPdef.h"
#include "CVIPimage.h"
#include "CVIPlab.h"
#include <stdio.h>
#include "CVIPluminance.h"
#include <CVIPmatrix.h>
#include <CVIPmorph.h>
#include "CVIPfs.h"
#include <limits.h>
#include <CVIPspfltr.h>
#include <dirent.h>

///#include<dirent.h>

/* There is a limit to how many objects can be found. The first pass of mark8 can only store 1000 objects */
/* 
Define constant values
*/

#define 	WHITE_LAB	255
#define		BLACK_LAB	0
#define     CHAIN		8
#define     PIE         3.14



/* This is the main function that does preprocessing 
	Flow is:
		1. Reduce number of bands to 1
		2. Sobel
		3. Dilate
		3. Dilate
		4. Erode
		5. Set edges to zero - for help with object detection
		6. Object detection
		7. Inspect results of object detection
			7.a If eyes and mouth are detected go to calculation step (not yet implemented)
			7.b If incorrect counts of eyes / mouth are detected go back and do some more 
				filtering and the try object detection - ( not yet fully implemented, this will be some sort of loop)

	TODO:
		* Implement the loop to go back and filter more if features are not properly extracted.
		* After identifying the facial features, extract the measurments to be used as classification features
		* Function to find the center point of objects

	Other Notes:
		The debug parameter is set in proj2_all_v2 function at the call to proj2_v2
			Debug of 0 will show the intermediary images. Debug of 1 will not.
		Object numbers in the log file now corrospond to the pixel value for that object.
*/
Image *proj2_v2(Image *inputImage, int debug, char filename[128] ){
	struct RetVal  markChainRet;
	Image   *dirImage;      /* For sobel */
	byte 	**image; 	    /* 2-d matrix data pointer */
	//FILE    *logFile;       /* Log file */
	//FILE    *logFileFace;   /* Log file for facial features*/
	int 	r,		        /* row index */
			c, 		        /* column index */
			bands,		    /* band index */
			no_of_rows,	    /* number of rows in image */
			no_of_cols,	    /* number of columns in image */
			no_of_bands;
	int threshval = 128;
	int limit = 1;
	int count = 0;
	




	//logFile = fopen("objects.txt","w");
	//logFileFace = fopen("facialFeatures.txt","w");
    //Gets the number of image bands (planes)
    no_of_bands = getNoOfBands_Image(inputImage);

    //Gets the number of rows in the input image  
    no_of_rows =  getNoOfRows_Image(inputImage);

    //Gets the number of columns in the input image  
    no_of_cols =  getNoOfCols_Image(inputImage);
	
	/* Step 0 - Reduce the number of bands to 1 */
	if(no_of_bands > 1){
		inputImage = extract_band(inputImage, 1);
		no_of_bands = getNoOfBands_Image(inputImage);
	}

	//inputImage = auto_thresh(inputImage, limit, debug);

	/* Step 1.5 Morphological FIltering - closing */
	/* This is returning a format that is causing errors */
	dirImage = new_Image(inputImage->image_format, 
		inputImage->color_space, inputImage->bands, 
		getNoOfRows_Image(inputImage), 
		getNoOfCols_Image(inputImage), CVIP_FLOAT, REAL);

	inputImage = sobel_filter(inputImage, dirImage, -1, 3, -1, threshval);
	if(debug == 0){
		view_Image(inputImage,"sobel");
	}
    //view_Image(dirImage, "dirImage");
	//inputImage = (Image *)luminance_Image(inputImage);

	inputImage = (Image *)MorphDilate(inputImage,1,3,3,3);
	if(debug == 0){
		view_Image(inputImage,"dilated");
	}

	inputImage = (Image *)MorphDilate(inputImage,1,3,3,3);
	if(debug == 0){
		view_Image(inputImage,"dilated2");
	}

	inputImage = (Image *)MorphErode(inputImage,1,3,3,3);
	if(debug == 0){
		view_Image(inputImage,"eroded");
	}

	/* Step 2 - Set edges to 0 */
	for(bands=0; bands < no_of_bands; bands++) {
  		image = (byte **)getData_Image(inputImage, bands);
		for(r=0; r < no_of_rows; r++) {
			if(r == 0 || r == no_of_rows -1){
				for(c=0;c < no_of_cols;c++){
					image[r][c] = BLACK_LAB;
				}
			}else{
				image[r][0] = BLACK_LAB;
				image[r][no_of_cols - 1] = BLACK_LAB;
			}
		}
	}
	
	markChainRet = mark_chain8(inputImage, debug, filename);

	inputImage = markChainRet.returnImage;
	printf("\nEye Count: %d\n", markChainRet.eye_count);
	printf("Mouth Count: %d\n", markChainRet.mouth_count);
	printf("Eyebrows Count: %d\n", markChainRet.eyebrow_count);

	count = 0;
	/* If < 2 eyes, try eroding again */
	while(markChainRet.eye_count < 2 && count < 4){
		inputImage = thresh(inputImage, 1);
		inputImage = (Image *)MorphErode(inputImage,1,3,3,3);
		inputImage = (Image *)MorphDilate(inputImage,1,3,3,3);
		markChainRet = mark_chain8(inputImage, debug, filename);

		inputImage = markChainRet.returnImage;
		view_Image(inputImage,"Erosion 2");
		printf("\nEye Count2: %d\n", markChainRet.eye_count);
		printf("Mouth Count2: %d\n", markChainRet.mouth_count);
		printf("Eyebrows Count2: %d\n", markChainRet.eyebrow_count);
		count = count + 1;
	}


	if(markChainRet.mouth_count != 2){
		// Pick the one with the larger area
	}
	if(markChainRet.eyebrow_count != 2){
		// NOt sure yet
	}


	/* Find Center Point of All Objects */
	
	/* Invert the object pixel values so they are easy to see - this will also let them corrospond to the object number in the log file*/
	for(bands=0; bands < no_of_bands; bands++) {
		image = (byte **)getData_Image(inputImage, bands);
		for(r=0;r < no_of_rows;r++){
			for(c=0;c<no_of_cols;c++){
				if(image[r][c] > 0){
					image[r][c] = 255 - image[r][c];
				}
			}
		}
    }	
	

	/* the following call performs filter operation of threshold value of 100*/
    //inputImage = moravec_corner_filter(inputImage,140);

	//inputImage = harris_corner_filter(inputImage, 0.05f, 20000, 20, 10.0f);
	
	return inputImage;
}



Image *thresh(Image *inputImage, int threshold){
	int 	r,		        /* row index */
			c, 		        /* column index */
			bands,		    /* band index */
			no_of_rows,	    /* number of rows in image */
			no_of_cols,	    /* number of columns in image */
			no_of_bands;	/* number of image bands */
	byte 	**image; 	    /* 2-d matrix data pointer */

	no_of_bands = getNoOfBands_Image(inputImage);

    //Gets the number of rows in the input image  
    no_of_rows =  getNoOfRows_Image(inputImage);

    //Gets the number of columns in the input image  
    no_of_cols =  getNoOfCols_Image(inputImage);


		/* Actually Apply the Threshold */
	printf("\nUsing threshold value of: %d",threshold);
	for(bands=0; bands < no_of_bands; bands++) {
		image = (byte **)getData_Image(inputImage, bands);
		for(r=0; r < no_of_rows; r++) {
			for(c=0; c < no_of_cols; c++) {
				if(image[r][c] < (byte) threshold){
					image[r][c] = BLACK_LAB;
				}
				else { 
					image[r][c] =  WHITE_LAB;
				}
			}
		}
	}

	return inputImage;
}

struct FaceObject initFaceObj(struct FaceObject obj){
	obj.area = -1;
	obj.center_col = -1;
	obj.center_row = -1;
	obj.circularity = -1;
	obj.diameter = -1;
	obj.perimeter = -1;
	obj.type = -1;

	return obj;
}

/* Implements automatic thresholding */
Image *auto_thresh(Image *inputImage, int limit, int debug){
	int diff;
	int sum;
	int count;
	int average;
	int mean1 = 0;
	int mean2 = 0;
	int frequency[256]	= {0};
	int thresh2;
	int 	r,		        /* row index */
			c, 		        /* column index */
			bands,		    /* band index */
			no_of_rows,	    /* number of rows in image */
			no_of_cols,	    /* number of columns in image */
			no_of_bands;	/* number of image bands */
	byte 	**image; 	    /* 2-d matrix data pointer */
	int threshval;
	
	no_of_bands = getNoOfBands_Image(inputImage);

    //Gets the number of rows in the input image  
    no_of_rows =  getNoOfRows_Image(inputImage);

    //Gets the number of columns in the input image  
    no_of_cols =  getNoOfCols_Image(inputImage);
	
	/* Step 1 - Automatic Threshold the image */
	/* Calculate the average pixel value */
	sum = 0;
	count = 0;
	average = 0;
	for(bands=0; bands < no_of_bands; bands++) {
		image = (byte **)getData_Image(inputImage, bands);
		for(r=0; r < no_of_rows; r++) {
				for(c=0; c < no_of_cols; c++) {
					sum = sum + image[r][c];
					count = count + 1;
				}
		 }
	}
	average = sum / count;
	sum = 0; // Clear out the sum
	threshval = average; // Book suggested that the starting threshold is the average pixel value
	/* Iterate through the automatic thresholding algorithm until limit is reached */
	printf("\nUsing a limit of %d ",limit);
	while(1 == 1){
		printf("\nCurrent threshold is: %d",threshval);
		mean1 = 0;
		mean2 = 0;
		frequency[BLACK_LAB] = 0;
		frequency[WHITE_LAB] = 0;
		for(bands=0; bands < no_of_bands; bands++) {
			image = (byte **)getData_Image(inputImage, bands);
			for(r=0; r < no_of_rows; r++) {
					for(c=0; c < no_of_cols; c++) {
						if(image[r][c] > (byte) threshval){
							//image[r][c] = BLACK_LAB;
							frequency[BLACK_LAB] = frequency[BLACK_LAB] + 1; // Count
							mean1 = mean1 + image[r][c]; // Sum
						}
						else { 
							//image[r][c] =  WHITE_LAB;
							frequency[WHITE_LAB] = frequency[WHITE_LAB] + 1; // Count
							mean2 = mean2 + image[r][c]; // Sum
						}
					}
			 }
		}

		//printf("\nsum1: %d sum2: %d mean1: %d mean2: %d ",frequency[BLACK_LAB],frequency[WHITE_LAB] , mean1, mean2);
		/* Mean of each side */
		mean1 = mean1 / frequency[BLACK_LAB] ;
		mean2 = mean2 / frequency[WHITE_LAB] ;
		thresh2 = (mean1 + mean2) / 2;
		diff = abs(thresh2 - threshval);
		threshval = thresh2;
		//printf("\nmean1: %d mean2: %d thresh2: %d diff: %d ", mean1, mean2, thresh2, diff);
		if(diff < limit){
			break;
		}
	}
	
	/* Actually Apply the Threshold */
	printf("\nUsing threshold value of: %d",threshval);
	for(bands=0; bands < no_of_bands; bands++) {
		image = (byte **)getData_Image(inputImage, bands);
		for(r=0; r < no_of_rows; r++) {
			for(c=0; c < no_of_cols; c++) {
				if(image[r][c] > (byte) threshval){
					image[r][c] = BLACK_LAB;
				}
				else { 
					image[r][c] =  WHITE_LAB;
				}
			}
		}
	}
	/* Show the binary image */
	if(debug == 0){
		view_Image(inputImage,"binaryImage");
	}
	/* End of Automatic Threshold */
	
	return (inputImage);
}

/* Object detection - returns a struct that contains the new image as well as the counts of identified facial features. 
	This will generate a log file named facialFfeatures.txt which is overwritten each time this function is called.
	Parameters to play with:
		The comparison values in the last loop that determine the cutoff for an object of interest:
			First is .5 for circularity to identify eyes
			Second is .2 for circularity (effectivly < .5 and > .2) to find the mouth / nose
			Third is the area cutoff - this filters out unwanted objects, currently its set to 180 - I had ok results with 200 as well
		
*/
struct RetVal mark_chain8(Image *inputImage, int debug, char filename[128] ){
	int eye_count = 0;
	int mouth_count = 0;
	int eyebrow_count = 0;
	byte 	**image; 	    /* 2-d matrix data pointer */
	//FILE    *logFileFace;   /* Log file for facial features*/
	int 	r,		        /* row index */
			c, 		        /* column index */
			bands,		    /* band index */
			no_of_rows,	    /* number of rows in image */
			no_of_cols,	    /* number of columns in image */
			no_of_bands,	/* number of image bands */
			nextr,          /* next row index */
			nextc;          /* next column index */
	int one,two,three,four;
	int neighbors[4];
	int object_count = 0;
	int temp;
	int minVal;
	int tableKey[3000] = {0};
	int outMin = 1000;
	int outMax = 0;
	int object = 0;
	int areas[1000] = {0}; // Hold the areas of all the objecs at the index of the object #
	double perims[1000] = {0}; // Hold the perimeters of all the objecs at the index of the object #
	double diameters[1000] = {0}; // Hold the diameters of all the objecs at the index of the object #
	double circularities[1000] = {0}; // Hold the circularities of all the objecs at the index of the object #
	double diameter = 0.0;
	int area = 0;
	double circularity = 0.0;
	double p = 0;      // Perimeter
	int true_object_count = 0;
	int compass[8][2] =   {{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0},{1,1}}; // Values to add to r,c when searcing neighbors for the chain8 code
	int d = 0;
	double even = 1;
	double odd = sqrt(2);
	int start[2];      // Hold the starting r,c for the object
	int count;
	int sum = 0;
	struct RetVal ret;
	FILE *logFileFace;
	//int center_point[2]; /* For storing the center point of each object */
	struct CenterPoint center_point;
	char logfileName[128] = "";
	struct FaceObject leftEye = { -1, -1, -1, -1, -1, -1, -1};
	struct FaceObject leftEyeBrow  = { -1, -1, -1, -1, -1, -1, -1};
	struct FaceObject rightEye  = { -1, -1, -1, -1, -1, -1, -1};
	struct FaceObject rightEyeBrow  = { -1, -1, -1, -1, -1, -1, -1};
	struct FaceObject mouth  = { -1, -1, -1, -1, -1, -1, -1};
	struct FaceObject nose  = { -1, -1, -1, -1, -1, -1, -1};
	struct FaceObject tempObj  = { -1, -1, -1, -1, -1, -1, -1};
	struct Objects objects;

	/* Initilize Objets 
	leftEye = initFaceObj(leftEye);
	rightEye = initFaceObj(rightEye);
	leftEyeBrow = initFaceObj(leftEyeBrow);
	rightEyeBrow = initFaceObj(rightEyeBrow);
	mouth = initFaceObj(mouth);
	nose = initFaceObj(nose);
	*/

	objects.leftEye = leftEye;
	objects.rightEye = rightEye;
	objects.mouth = mouth;
	objects.nose = nose;
	objects.leftEyeBrow = leftEyeBrow;
	objects.rightEyeBrow = rightEyeBrow;

	strcpy(logfileName,"features");
	strcat(logfileName, filename);
	strcat(logfileName, ".txt");

	logFileFace = fopen(logfileName,"w");
	    //Gets the number of image bands (planes)
    no_of_bands = getNoOfBands_Image(inputImage);

    //Gets the number of rows in the input image  
    no_of_rows =  getNoOfRows_Image(inputImage);

    //Gets the number of columns in the input image  
    no_of_cols =  getNoOfCols_Image(inputImage);


	/* Step 3 - First Pass mark8 */
	for(bands=0; bands < no_of_bands; bands++) {
  		image = (byte **)getData_Image(inputImage, bands);
		object_count = 0;
  		for(r=1; r < no_of_rows - 1; r++) {
			for(c=1; c < no_of_cols - 1; c++) {
				if(image[r][c]  == WHITE_LAB){
					// Add neighbors to neighbors array
					one   = image[r-1][c+1];
					two   = image[r-1][c];
					three = image[r-1][c-1];
					four  = image[r][c+-1];
					neighbors[0] = one;
					neighbors[1] = two;
					neighbors[2] = three;
					neighbors[3] = four;
					// If all neighbors are background its a new object
					if(one == two && two == three && three == four && four == BLACK_LAB){
						object_count = object_count + 1; // Start at 1 because background is zero
						image[r][c] = object_count;
					}else{
						//Otherwise find the min of the neighbors that arn't zero
						minVal = 1000;
						for(count = 0; count < 4; count++){
							if(neighbors[count] > 0 && neighbors[count] < minVal){
								minVal = neighbors[count];
							}
						}
						// Now we have the min - assign that value to the image
						image[r][c] = minVal;
						// Now we have to check if neighbors have more than one value i.e. a conflict
						//if all surrounding are from the same object , its part of that one
						if(one == two && two == three && three == four && four == minVal){
							continue;
						}else{
							//There is a conflict - record the value and then record the conflict
							for(count = 0; count < 4; count++){
								if(neighbors[count] == 0 || neighbors[count] == minVal){
									// Dont worry if it's a zero value or the same as the min
									continue;
								}else{
									// If there isnt already a value for that key set ti
									if(tableKey[neighbors[count]] == 0){
										tableKey[neighbors[count]] = minVal;
									}else{
										// Otherwise set the value to the min of the minVal and the existing key
										temp = tableKey[neighbors[count]];
										tableKey[neighbors[count]] = min(temp,minVal);
									}
									//printf("\ntableKey %d is %d", neighbors[count], tableKey[neighbors[count]]);
								}
							}
						}
					}
				}
			}
		}
    }
	if(debug == 0){
		view_Image(inputImage,"mark8 - pass1");
	}

	/* Step 4 - Second Pass mark8 */
	for(temp = 3000; temp >=0; temp--){
		if(tableKey[temp] > 0){
			//printf("\n%d , %d", temp, tableKey[temp]);
			for(bands=0; bands < no_of_bands; bands++) {
				image = (byte **)getData_Image(inputImage, bands);
				for(r=1; r < no_of_rows -1; r++) {
					for(c=1; c < no_of_cols -1; c++) {
						if(image[r][c] == temp ){
							image[r][c] = tableKey[temp];
						}
					}
				}
			}
		}
	}
	
	
	/* Step 5 - Get new object count  */
	for(bands=0; bands < no_of_bands; bands++) {
		image = (byte **)getData_Image(inputImage, bands);
  		for(r=1; r < no_of_rows -1 ; r++) {
			for(c=1; c < no_of_cols -1 ; c++) {
				if(image[r][c] > outMax){
					outMax = image[r][c];
				}
				if(image[r][c] < outMin){
					outMin = image[r][c];
				}
			}
		}
    }
	if(debug == 0){
		view_Image(inputImage,"mark8 - pass2");
	}
	//printf("\noutMax: %d outMin: %d",outMax,outMin);
	//printf("\nOriginal Object Count: %d", object_count);
	//printf("\nSecond Pass Object Count: %d", outMax);
	printf("\n");
	object_count = outMax;
	
	/* Step 5.5  - Get areas & diameters */
	for(bands=0; bands < no_of_bands; bands++) {
		image = (byte **)getData_Image(inputImage, bands);
		for(object = 1; object < object_count + 1; object++){
			for(r=1; r < no_of_rows -1 ; r++) {
				for(c=1; c < no_of_cols -1 ; c++) {
					if(image[r][c] == object){
						sum = sum + object;
					}
				}
			}
			area = sum / object;
			diameter = sqrt(area/PIE) * 2;
			if(area > 0){
				//printf("\nArea for object %d is %d",object, area);
				//printf("\nDiameter for object %d is %f",object, diameter);
			}
			// Add the area to the areas array for use calculating the circularity
			areas[object] = area;
			diameters[object] = diameter;
			sum = 0;
		}
    }	
	
	
	/* End of mark8 */
	/* Start of chain8 */

		/* Step 6 - chain8 */
		for(bands=0; bands < no_of_bands; bands++) {
			image = (byte **)getData_Image(inputImage, bands);
			//image_out = (byte **)getData_Image(outputImage, bands);
			for(object = 1; object < object_count + 1; object++){
				for(r=0;r < no_of_rows;r++){
					for(c=0;c < no_of_cols;c++){
						//If its equal to the object number its part of an object - start the chain
						if(image[r][c] == object){
							//printf("\nStart of object %d ",object);
							true_object_count = true_object_count + 1;
							start[0] = r;
							start[1] = c; // Store the starting pixel location in start array
							image[r][c] = WHITE_LAB; // Optional - turn the perimeter white
							d = (d + 5) % 8; // Next direction to look
							do{
								nextr = r + compass[d][0];
								nextc = c + compass[d][1]; // Possible next pixle coordinates
								//Increase the count
								count = count + 1;
								// Now make sure next r and next c are inbounds
								if(nextr >= 0 && nextc >= 0 && nextr < no_of_rows && nextc < no_of_cols){
									//Check to see if image[nextr][nextc] is still part of an object or the last pixel, WHITE_LAB
									if(image[nextr][nextc] == object || image[nextr][nextc] == WHITE_LAB){ 
										// If so - move there and increase p
										r = nextr;
										c = nextc;
										image[r][c] = WHITE_LAB; // Optional - turn the perimeter white
										// Reset the neighbor conut
										count = 0;
										// Check if d is even or odd to increase p by the right amount
										if((d % 2) == 0){
											//Even
											p = p + even;
										}else{
											// Odd
											p = p + odd;
										}
										d = (d + 5) % 8; // Next direction to look
									}else{
										// pixel at neighbor is not part of the object - increase d by 1 and try again
										d = (d + 1) % 8;
									}
								
								}else{
									// next r or next c is out of bounds - increase d by one and try again
									d = (d + 1) % 8;
								}
								// If we've checked 8 neighbors and nothing it must be a one pixel object
								if(count > 8){
									printf("\n\nBREAKING! %d\n", object);
									break;
								}
								// Check to see if we're at the starting pixel - if so break the loop
							}while(nextr != start[0] || nextc != start[1]);
							// Finished with that object - print its p
							//printf("\n\tObject %d perimeter is: %f",object,p);
							circularity = ( ((4*PIE) * areas[object]) / (p * p) );
							//circularity = 2 * sqrt(areas[object]) / (p * p);
							//printf("\n\tObject %d circularity is: %f",object,circularity);
							circularities[object] = circularity;
							perims[object] = p;
							// Reset d and p
							d = 0;
							p = 0;
							// No need to scan the rest
							c = no_of_cols;
							r = no_of_rows;
						}
					}
				}
			}
		}
		printf("\n\nTrue Object Count: %d", true_object_count);
		// Identify the eyes and other candidates as well
		count = 0;
		for(object = 1; object < object_count + 1; object++){
			if(areas[object] > 0){
				count = count + 1;
				//fprintf(logFile,"\nObject %d \n\tArea: %d\n\tDiameter: %f\n\tPerimeter: %f\n\tCircularity: %f\n",object,areas[object],diameters[object],perims[object],circularities[object]);
				if(areas[object] > 180){
					//fprintf(logFileFace,"\nObject %d \n\tArea: %d\n\tDiameter: %f\n\tPerimeter: %f\n\tCircularity: %f\n",object,areas[object],diameters[object],perims[object],circularities[object]);
						center_point = find_center(inputImage, object);
						tempObj.area = areas[object];
						tempObj.center_col = center_point.col;
						tempObj.center_row = center_point.row;
						tempObj.circularity = circularities[object];
						tempObj.diameter = diameters[object];
						tempObj.perimeter = perims[object];
						tempObj.type = 255 - object;
					if(circularities[object] > .5){
						eye_count = eye_count + 1;
						//printf("\nEye: %d", eye_count);
						
						//fprintf(logFileFace,"\nObject %d  - Class - Eye\n\tArea: %d\n\tDiameter: %f\n\tPerimeter: %f\n\tCircularity: %f",255 - object,areas[object],diameters[object],perims[object],circularities[object]);
						//fprintf(logFileFace, "\n\tCenter Point: %d,%d\n", center_point.row, center_point.col);

						if(objects.leftEye.center_col == -1){
							// If left eye is empty - make it this object
							objects.leftEye = tempObj;
						}else if(objects.rightEye.center_col == -1){
							// If left eye is set and right is empty - make it this object
							// But need to make sure they are in the right positions and not the same!
							if(objects.leftEye.center_col > tempObj.center_col){
								objects.rightEye = objects.leftEye;
								objects.leftEye = tempObj;

							}else{
								objects.rightEye = tempObj;
							}

						}else if((objects.leftEye.center_row + 5) > objects.rightEye.center_row && (objects.leftEye.center_row - 5) < objects.rightEye.center_row ){
							// Eyes are already good so do nothing
						
						}else if(tempObj.center_row < (objects.leftEye.center_row + 5) && tempObj.center_row > (objects.leftEye.center_row - 5)){
							// If this object's center is within 10 rows of left eye's center make this the right eye
							objects.rightEye = tempObj;
						}else if(tempObj.center_row < (objects.rightEye.center_row + 5) && tempObj.center_row > (objects.rightEye.center_row - 5)){
							// If this object's center is within 10 rows of right eye's center make this the left eye
							objects.leftEye = tempObj;
						}
					}else if(circularities[object] > .2){
						mouth_count = mouth_count + 1;
						//fprintf(logFileFace,"\nObject %d  - Class - Mouth or Nose \n\tArea: %d\n\tDiameter: %f\n\tPerimeter: %f\n\tCircularity: %f",255 - object,areas[object],diameters[object],perims[object],circularities[object]);
						//fprintf(logFileFace, "\n\tCenter Point: %d,%d\n", center_point.row, center_point.col);
						if(objects.mouth.center_col == -1){
							// If mouth isnt set, make this the mouth
							objects.mouth = tempObj;
						}else if(objects.mouth.center_row < tempObj.center_row){
							// If new object is lower than current mouth - new object is the mouth
							objects.nose = objects.mouth;
							objects.mouth = tempObj;
						}else if(objects.mouth.center_row > tempObj.center_row){
							// Otherwise its the nose
							objects.nose = tempObj;
						}


					}else{
						//fprintf(logFileFace,"\nObject %d  - Class - Unknown \n\tArea: %d\n\tDiameter: %f\n\tPerimeter: %f\n\tCircularity: %f",255 - object,areas[object],diameters[object],perims[object],circularities[object]);
						//fprintf(logFileFace, "\n\tCenter Point: %d,%d\n", center_point.row, center_point.col);
					}
				}
			}
		}
		count = 0;
		for(object = 1; object < object_count + 1; object++){
			if(areas[object] > 0){
				count = count + 1;
				//fprintf(logFile,"\nObject %d \n\tArea: %d\n\tDiameter: %f\n\tPerimeter: %f\n\tCircularity: %f\n",object,areas[object],diameters[object],perims[object],circularities[object]);
				if(areas[object] > 180){
					//fprintf(logFileFace,"\nObject %d \n\tArea: %d\n\tDiameter: %f\n\tPerimeter: %f\n\tCircularity: %f\n",object,areas[object],diameters[object],perims[object],circularities[object]);
					center_point = find_center(inputImage, object);
					tempObj.area = areas[object];
					tempObj.center_col = center_point.col;
					tempObj.center_row = center_point.row;
					tempObj.circularity = circularities[object];
					tempObj.diameter = diameters[object];
					tempObj.perimeter = perims[object];
					tempObj.type = 255 - object;
					printf("\nCenter Point: %d, %d",center_point.row, center_point.col);
					printf("\nCenter Point: %d, %d\n",tempObj.center_row, tempObj.center_col);
					printf("\nArea: %d\n",tempObj.area);
					printf("\nArea: %d\n",areas[object]); 
					if(tempObj.type != objects.leftEye.type && tempObj.type != objects.rightEye.type){
						if(tempObj.center_row < objects.leftEye.center_row || tempObj.center_row < objects.rightEye.center_row){
							if(tempObj.center_col < no_of_cols /2 &&  tempObj.center_col > (objects.leftEye.center_col - 25)){
								objects.leftEyeBrow = tempObj;
							}else if(tempObj.center_col < (objects.rightEye.center_col + 25)){
								objects.rightEyeBrow = tempObj;
							}
						}else if(tempObj.circularity > objects.mouth.circularity){
							// If mouth isnt set, make this the mouth
							objects.mouth = tempObj;
						}
					}
						
				}
			}
		}

	// Sometimes Nose and Mouth are Switched:
		if(objects.nose.type != -1 && objects.mouth.type != -1){
			if(objects.nose.center_row > objects.mouth.center_row){
				tempObj = objects.nose;
				objects.nose = objects.mouth;
				objects.mouth = tempObj;
			}
		}

	/* Remove unnecessary objects and invert values for objects */
	for(bands=0; bands < no_of_bands; bands++) {
		image = (byte **)getData_Image(inputImage, bands);
		for(r=0;r < no_of_rows;r++){
			for(c=0;c<no_of_cols;c++){
				if(image[r][c] > 0){
					//if (areas[image[r][c]] > 180 && circularities[image[r][c]] > .2){
					if (areas[image[r][c]] > 180){
						//image[r][c] = 255 - image[r][c];
						//fprintf(logFileFace,"\nObject %d \n\tArea: %d\n\tDiameter: %f\n\tPerimeter: %f\n\tCircularity: %f\n",image[r][c],areas[image[r][c]],diameters[image[r][c]],perims[image[r][c]],circularities[image[r][c]]);
					}else{
						image[r][c] = BLACK_LAB;
					}
					
				}
			}
		}
    }

	printf("\nLeftEye: %d\n RightEye: %d\n Nose: %d\n Mouth: %d\n LeftEyebrow %d\n RightEyeBrow: %d\n", objects.leftEye.type, objects.rightEye.type, objects.nose.type, objects.mouth.type, objects.leftEyeBrow.type, objects.rightEyeBrow.type);
	printf("\nLeft Eye Center: %d,%d",objects.leftEye.center_row,objects.leftEye.center_col);
	printf("\nLeft Eye Circularity: %d",objects.leftEye.circularity);

	fprintf(logFileFace,"\n----------------------------------------------------------------------------\n");
	fprintf(logFileFace,"\n LeftEye: %d\n RightEye: %d\n Nose: %d\n Mouth: %d\n LeftEyebrow %d\n RightEyeBrow: %d\n", objects.leftEye.type, objects.rightEye.type, objects.nose.type, objects.mouth.type, objects.leftEyeBrow.type, objects.rightEyeBrow.type);
	fprintf(logFileFace,"\n----------------------------------------------------------------------------\n");

	fprintf(logFileFace,"\nLeft Eye: %d",objects.leftEye.type);
	fprintf(logFileFace,"\nLeft Eye Center: %d,%d",objects.leftEye.center_row,objects.leftEye.center_col);
	fprintf(logFileFace,"\nLeft Eye Circularity: %f",objects.leftEye.circularity);
	fprintf(logFileFace,"\nLeft Eye Area: %d\n",objects.leftEye.area);

	fprintf(logFileFace,"\n----------------------------------------------------------------------------\n");

	fprintf(logFileFace,"\nRight Eye: %d",objects.rightEye);
	fprintf(logFileFace,"\nRight Eye Center: %d,%d",objects.rightEye.center_row,objects.rightEye.center_col);
	fprintf(logFileFace,"\nRight Eye Circularity: %f",objects.rightEye.circularity);
	fprintf(logFileFace,"\nRight Eye Area: %d\n",objects.rightEye.area);

	fprintf(logFileFace,"\n----------------------------------------------------------------------------\n");

	fprintf(logFileFace,"\nLeft Eyebrow: %d",objects.leftEyeBrow);
	fprintf(logFileFace,"\nLeft Eyebrow Center: %d,%d",objects.leftEyeBrow.center_row,objects.leftEyeBrow.center_col);
	fprintf(logFileFace,"\nLeft Eyebrow Circularity: %f",objects.leftEyeBrow.circularity);
	fprintf(logFileFace,"\nLeft Eyebrow Area: %d\n",objects.leftEyeBrow.area);

	fprintf(logFileFace,"\n----------------------------------------------------------------------------\n");

	fprintf(logFileFace,"\nRight Eyebrow: %d",objects.rightEyeBrow);
	fprintf(logFileFace,"\nRight Eyebrow Center: %d,%d",objects.rightEyeBrow.center_row,objects.rightEyeBrow.center_col);
	fprintf(logFileFace,"\nRight Eyebrow Circularity: %f",objects.rightEyeBrow.circularity);
	fprintf(logFileFace,"\nRight Eyebrow Area: %d\n",objects.rightEyeBrow.area);

	fprintf(logFileFace,"\n----------------------------------------------------------------------------\n");

	fprintf(logFileFace,"\nNose: %d",objects.nose);
	fprintf(logFileFace,"\nNose Center: %d,%d",objects.nose.center_row,objects.nose.center_col);
	fprintf(logFileFace,"\nNose Circularity: %f",objects.nose.circularity);
	fprintf(logFileFace,"\nNose Area: %d\n",objects.nose.area);

	fprintf(logFileFace,"\n----------------------------------------------------------------------------\n");
	fprintf(logFileFace,"\nMouth: %d",objects.mouth);
	fprintf(logFileFace,"\nMouth Center: %d,%d",objects.mouth.center_row,objects.mouth.center_col);
	fprintf(logFileFace,"\nMouth Circularity: %f",objects.mouth.circularity);
	fprintf(logFileFace,"\nMouth Area: %d\n",objects.mouth.area);

	fprintf(logFileFace,"\n----------------------------------------------------------------------------\n");
	
	//fprintf(logFileFace,"\n----------------------------------------------------------------------------\n");
	//fprintf(logFileFace,"\n LeftEye: %d\n RightEye: %d\n Nose: %d\n Mouth: %d\n LeftEyebrow %d\n RightEyeBrow: %d\n", objects.leftEye.type, objects.rightEye.type, objects.nose.type, objects.mouth.type, objects.leftEyeBrow.type, objects.rightEyeBrow.type);
	//fprintf(logFileFace,"\n----------------------------------------------------------------------------\n");
	//fprintf(logFileFace,"\nLeft Eye: %d\n\tArea %d\n\tCircularity: %f\n\tCenter Point: %d,%d\n\t, ",objects.leftEye.type, objects.leftEye.area, objects.leftEye.circularity, objects.leftEye.center_row, objects.leftEye.center_col);
	//fprintf(logFileFace,"\nRight Eye: %d\n\tArea %d\n\tCircularity: %f\n\tCenter Point: %d,%d\n\t, ",objects.rightEye.type, objects.rightEye.area, objects.rightEye.circularity, objects.rightEye.center_row, objects.rightEye.center_col);
	//fprintf(logFileFace,"\nRight Eyebrow: %d\n\tArea %d\n\tCircularity: %f\n\tCenter Point: %d,%d\n\t, ",objects.rightEyeBrow.type, objects.rightEyeBrow.area, objects.rightEyeBrow.circularity, objects.rightEyeBrow.center_row, objects.rightEyeBrow.center_col);
	//fprintf(logFileFace,"\nLeft Eyebrow: %d\n\tArea %d\n\tCircularity: %f\n\tCenter Point: %d,%d\n\t, ",objects.leftEyeBrow.type, objects.leftEyeBrow.area, objects.leftEyeBrow.circularity, objects.leftEyeBrow.center_row, objects.leftEyeBrow.center_col);
	//fprintf(logFileFace,"\nNose: %d\n\tArea %d\n\tCircularity: %f\n\tCenter Point: %d,%d\n\t, ",objects.nose.type, objects.nose.area, objects.nose.circularity, objects.nose.center_row, objects.nose.center_col);
	//fprintf(logFileFace,"\nMouth: %d\n\tArea %d\n\tCircularity: %f\n\tCenter Point: %d,%d\n\t, ",objects.mouth.type, objects.mouth.area, objects.mouth.circularity, objects.mouth.center_row, objects.mouth.center_col);
	fclose(logFileFace);
	
	ret.returnImage = inputImage;
	ret.eyebrow_count = eyebrow_count;
	ret.eye_count = eye_count;
	ret.mouth_count = mouth_count;

	return (ret);
}


struct CenterPoint find_center(Image *inputImage, int object){
	byte 	**image; 	    /* 2-d matrix data pointer */
	int 	r,		        /* row index */
			c, 		        /* column index */
			bands,		    /* band index */
			no_of_rows,	    /* number of rows in image */
			no_of_cols,	    /* number of columns in image */
			no_of_bands,	/* number of image bands */
			nextr,          /* next row index */
			nextc,          /* next column index */
			row_sum,
			row_total,
			col_sum,
			col_total,
			area;
	struct CenterPoint center;

	area = 0;
		//Gets the number of image bands (planes)
		no_of_bands = getNoOfBands_Image(inputImage);

		//Gets the number of rows in the input image  
		no_of_rows =  getNoOfRows_Image(inputImage);

		//Gets the number of columns in the input image  
		no_of_cols =  getNoOfCols_Image(inputImage);

		row_total = 0;
		for(bands=0; bands < no_of_bands; bands++) {
			image = (byte **)getData_Image(inputImage, bands);
			for(r=0;r < no_of_rows;r++){
				row_sum = 0;
				for(c=0;c<no_of_cols;c++){
					if(image[r][c] == object){
						row_sum = row_sum + 1;
						area = area + 1;
					}
				}
				row_total = row_total + (row_sum * r);
			}
		}

		col_total = 0;
		for(bands=0; bands < no_of_bands; bands++) {
			image = (byte **)getData_Image(inputImage, bands);
			for(c=0;c < no_of_cols;c++){
				col_sum = 0;
				for(r=0;r<no_of_rows;r++){
					if(image[r][c] == object){
						col_sum = col_sum + 1;
					}
				}
				col_total = col_total + (col_sum * c);
			}
		}

		

		center.col = (int)col_total / area;
		center.row = (int)row_total / area;

		printf("\nArea: %d, col_total: %d, row_total %d, col: %d, row: %d", area, col_total, row_total, center.col, center.row);

		return (center);


}

/* This function loops through the directory of images and calls the proj2_v2() function on each one, then displays the results */
int *proj2_all_v2(void) 
{ 
    struct dirent *de;  // Pointer for directory entry 
	char inputfile[256];
	Image		*cvipImage;
	int count;
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir("C:/CVIPtools/images/jaffedbase/jaffe/"); 
  
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 
  
    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html 
    // for readdir() 
	count = 0;
	while ((de = readdir(dr)) != NULL && count < 4){
		if(strcmp(de->d_name,".") != 0 && strcmp(de->d_name,"..") != 0){
			//printf("%s\n", de->d_name);
			strcpy(inputfile,"C:/CVIPtools/images/jaffedbase/jaffe/");
			strcat(inputfile, de->d_name);
			//printf("%s\n", inputfile);
			cvipImage = read_Image(inputfile,1);
			if(cvipImage == NULL) {
				error_CVIP("init_Image", "could not read image file");
				free(inputfile);
				closedir(dr); 
				return 1;
			}
			view_Image(cvipImage,de->d_name);
			/* Change debug here */
			cvipImage = proj2_v2(cvipImage, 1, de->d_name);
			view_Image(cvipImage,de->d_name);
			count = count + 1;
		}
	}
            
  
    closedir(dr);     
    return 0; 
} 