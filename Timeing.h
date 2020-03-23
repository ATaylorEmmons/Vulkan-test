
struct FrameRate {
	bool limitFrameRate;
	float fps;
	double targetFrameDuration;
	double frameDuration;
	
	//Give the os n milliseconds to wake up the thread
	//	this is a control to minimize the time spent 
	//  spinning in the wait loop
	int waitLoopDuration;
};

struct GameTime {
	double totalTime;
	double time;
	double dt;
};
