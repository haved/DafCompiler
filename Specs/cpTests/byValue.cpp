struct Vec3 {
	int x,y,z;
	inline Vec3(int x, int y, int z) : x(x), y(y), z(z) {}
};

Vec3 func(int a, int c, int b) {
	a+=16;
	b+=32;
	c+=20;
	return Vec3{a,b,c};
}
