#include "TunerDaemon.h"

int main(int argc, char *argv[]) {
	TunerDaemon daemon;	
	daemon.start();
	daemon.stop();
	return 0;
}

