/*struct Vec3 {
	int x,y,z;
	inline Vec3(int x, int y, int z) : x(x), y(y), z(z) {}
};

Vec3 func(int a, int c, int b) {
	return Vec3{a+12,b+13,c+16};
}

int call(Vec3 f);

void print(int i);

void doCall() {
	Vec3 vec(2,5,7);
	print(call(vec));
}*/

struct Vec20 {
	int m[20];
};

Vec20 call(int x);

int myThing(int y) {
	return call(y+2).m[0]+5;
}
