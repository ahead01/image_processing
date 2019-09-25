/***************************************************************************
* =========================================================================
*
*   Project 1 Austin Dase & Justine Caylor
* 
* =========================================================================
*
*             File Name: teamproject1.c 
*           Description: Performs mark8 to count images and chaing to determine perimeter. Other features
*           such as area, diameter, circularity, and classification is calculated
*   Initial Coding Date: October 20, 12018
*	   Last update Date: November 05, 12018
*           Portability: Standard (ANSI) C
*             Credit(s): Austin Dase & Justine Caylor; Towson University
*
****************************************************************************/

#define 	WHITE_LAB	255
#define		BLACK_LAB	0
#define     CHAIN		8
#define     PIE         3.14

Image *teamproject1(Image *inputImage){
	byte 	**image; 	    /* 2-d matrix data pointer */
	FILE    *logFile;       /* Log file */
	int 	r,		        /* row index */
			c, 		        /* column index */
			bands,		    /* band index */
			no_of_rows,	    /* number of rows in image */
			no_of_cols,	    /* number of columns in image */
			no_of_bands,	/* number of image bands */
			nextr,          /* next row index */
			nextc;          /* next column index */
	int threshval = 128;
	int frequency[256]	= {0};
	int temp;
	int minVal;
	int outMin = 1000;
	int outMax = 0;
	int count = 0;
	int sum = 0;
	int area = 0;
	int tableKey[3000] = {0};
	int one,two,three,four;
	int neighbors[4];
	int object_count = 0;
	int object = 0;
	int true_object_count = 0;
	int compass[8][2] =   {{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0},{1,1}}; // Values to add to r,c when searcing neighbors for the chain8 code
	int d = 0;
	double even = 1.0; // Value to add to p for even movement
	double odd = 1.41; // Value to add to p for odd movement
	double diameter = 0.0;
	double circularity = 0.0;
	double p = 0;      // Perimeter
	int start[2];      // Hold the starting r,c for the object
	int areas[1000] = {0}; // Hold the areas of all the objecs at the index of the object #
	double perims[1000] = {0}; // Hold the perimeters of all the objecs at the index of the object #
	double diameters[1000] = {0}; // Hold the diameters of all the objecs at the index of the object #
	double circularities[1000] = {0}; // Hold the circularities of all the objecs at the index of the object #
	int mean1 = 0;
	int mean2 = 0;
	int average;
	int thresh2;
	int diff;
	int limit = 5;
	int val;
	//char classifier1[10]="Small";
	//string classifier1 = "Small";
	//string classifier2 = "Medium";
	//string classifier3 = "Large";
	//char classifier2[10]="Medium";
	//char classifier3[10]="Large";
	//string classified[100]={""};
	char *classified[1000] = {""};
	char *classes[3] = {"Small","Medium","Large"};
	odd = sqrt(2);
	logFile = fopen("objectClassifiaction.txt","w");
    //Gets the number of image bands (planes)
    no_of_bands = getNoOfBands_Image(inputImage);

    //Gets the number of rows in the input image  
    no_of_rows =  getNoOfRows_Image(inputImage);

    //Gets the number of columns in the input image  
    no_of_cols =  getNoOfCols_Image(inputImage);
	
	/* Step 1 - Automatic Threshold the image */
	/* Calculate the average pixel value */
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
	// Show the binary image
	view_Image(inputImage,"binaryImage");
	/* End of Automatic Threshold */
	
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
	
	/* Step 3 - First Pass mark8 */
	for(bands=0; bands < no_of_bands; bands++) {
  		image = (byte **)getData_Image(inputImage, bands);
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
			//classify based on the area
			if(area < 150){
				classified[object]=classes[0];
				printf("\nClassification for object %d is %s",object, classes[0]);
				//printf("\nClassification for object %d is SMALL",object);
		}
			else if(area>=150 && area < 500){
				classified[object]=classes[1];
				printf("\nClassification for object %d is %s",object, classes[1]);
				//printf("\nClassification for object %d is MEDIUM",object);
    }	
			else{
				classified[object]=classes[2];
				printf("\nClassification for object %d is %s",object, classes[2]);
			    //printf("\nClassification for object %d is LARGE",object);
    }	
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
						//image[r][c] = WHITE_LAB; // Optional - turn the perimeter white
						d = (d + 5) % 8; // Next direction to look
						do{
							nextr = r + compass[d][0];
							nextc = c + compass[d][1]; // Possible next pixle coordinates
							//Increase the count
							count = count + 1;
							// Now make sure next r and next c are inbounds
							if(nextr >= 0 && nextc >= 0 && nextr < no_of_rows && nextc < no_of_cols){
								//Check to see if image[nextr][nextc] is still part of an object
								if(image[nextr][nextc] == object || image[nextr][nextc] == WHITE_LAB){
									// If so - move there and increase p
									r = nextr;
									c = nextc;
									//image[r][c] = WHITE_LAB; // Optional - turn the perimeter white
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
	count = 0;
	for(bands=0; bands < no_of_bands; bands++) {
		image = (byte **)getData_Image(inputImage, bands);
		for(object = 1; object < object_count + 1; object++){
			if(areas[object] > 0){
				count = count + 1;
				fprintf(logFile,"\nObject %d \n\tArea: %d\n\tDiameter: %f\n\tPerimeter: %f\n\tCircularity: %f\n\tClassification: %s\n",count,areas[object],diameters[object],perims[object],circularities[object],classified[object]);
			}
		}
    }	
	fclose(logFile);
	count = 0;
	/* Invert the object pixel values so they are easy to see */
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
	
	
	return inputImage;
}