typedef enum {
	M=0,
	S=4,
	R=8,
} bandtype;

void shutdown(){
	std::cerr << "Sorry, the parameters you provided were not valid.\n\tValid modes are:\n\t\t0 - mode 0\n\t\t1 - mode 1\n\t\t2 - mode 2\n\t\t3 - mode 3\n\tValid types are:\n\t\tm - mono\n\t\ts - stereo\n\t\tr - rds\n";
	exit(1);
}

