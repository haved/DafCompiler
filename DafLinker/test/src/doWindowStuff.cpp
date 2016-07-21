static int time = 0;

const char* getGoodName() {
	time++;	
	return time%2==0?"Goodie":"Oh well";
}