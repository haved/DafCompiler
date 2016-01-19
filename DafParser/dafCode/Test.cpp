#include "DafLang.h"

#include <stdio.h>

typedef struct {
	float_daf x, y, z;
} Vector;

int main () {
	uint8_daf i = 0;
	boolean_daf boooll = false_daf;
	vector_daf<Vector> vectors; //Confusing, I know
	i++;
	Vector v;
	v.x = 2.4f;
	v.y = 1.6f;
	v.z = 5.3f;
	vectors.push_back(v);
	printf("Size: %d\n", vectors.size());
	printf("[0].x = %f\n", vectors.back().x);
	printf("Test: %f", v.z);
	return 0;
}