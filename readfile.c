/*
 * readfile.c
 *
 *  Created on: Feb 13, 2016
 *      Author: J. Michael Rutter
 */


#include <stdio.h>
#include <math.h>

#define BIN32_BYTES 4 //Number of bytes in an IEEE 754 binary number
#define BYTE_SIZE 8   //Number of bits in a byte
#define EXP_SIZE 8    //Number of bits in the exponent section
#define MANT_SIZE 23  //Number of bits in the mantissa section

float bin2Float(int b2f);				//Converts IEEE 754 binary number to floating-point
int bswap32(int a);						//Converts big endian number to little endian
int getExp(int b[]);					//Converts exponent from binary to decimal
float getFrac(int b[]);					//Converts mantissa from binary to decimal
float machNum(float alt, float spd);	//Converts the airspeed in knots to a mach number

int main() {
	//Open the data file
	FILE *input = fopen("Alt_AS.dat", "r");
	if(input == NULL) {
		fprintf(stderr, "The file does not exist in the current directory.\n");
		return -1;
	} else
		fprintf(stdout, "File successfully opened.\n");

	//Find the length of the file
	fseek(input, 0, SEEK_END);
	int size = ftell(input);
	rewind(input);

	//Grab binaries from file and convert to floating point numbers
	int token=0;
	float time=0.;
	int bFloat;
	float altitude, speed, mach;

	while(token < size) {
		//Get the altitude
		time += 0.1;
		token += 4;
		fread(&bFloat, BIN32_BYTES, 1, input);
		bFloat = bswap32(bFloat);
		altitude = bin2Float(bFloat);
		fseek(input, token, SEEK_SET);

		//Get the speed
		token += 4;
		fread(&bFloat, BIN32_BYTES, 1, input);
		bFloat = bswap32(bFloat);
		speed = bin2Float(bFloat);
		fseek(input, token, SEEK_SET);
		mach = machNum(altitude, speed);

		//Print results
		fprintf(stdout, "Time: %.1f s\t Altitude=%.3f ft\t Speed=%.3f knots\t Mach#=%.2f\n", time, altitude, speed, mach);
	}

	fclose(input);
	return 0;
}

float bin2Float(int b2f) {
	int sign, i;
	int exp[8], frac[23];

	//Determine fraction
	for(i=0; i<MANT_SIZE; i++) {
		frac[MANT_SIZE-1-i] = b2f & 1;
		b2f >>= 1;
	}

	//Determine exponent
	for(i=0; i<EXP_SIZE; i++) {
		exp[EXP_SIZE-1-i] = b2f & 1;
		b2f >>= 1;
	}

	//Determine sign bit
	sign = b2f & 1;

	//Convert binary parts to decimal
	float fr = getFrac(frac);
	int ex = getExp(exp);

	//Return final floating point number
	return pow(-1, sign) * (1.0 + fr) * pow(2.0, ex-127);
}

int bswap32(int a) {
  a = ((a & 0x000000FF) << 24) |
      ((a & 0x0000FF00) <<  8) |
      ((a & 0x00FF0000) >>  8) |
      ((a & 0xFF000000) >> 24);
  return a;
}

int getExp(int b[]) {
   int i, sum=0;
   for(i=0; i<EXP_SIZE; i++)
	   sum += b[i] * pow(2.0, EXP_SIZE-i-1);
   return sum;
}

float getFrac(int b[]) {
	int i;
	float sum=0;
	for(i=0; i<MANT_SIZE; i++)
		sum += b[i] * pow(2.0, -i-1);
	return sum;
}

float machNum(float alt, float spd) {
	return (spd / (-.0023 * alt + 660));
}
