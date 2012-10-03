#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "PersistenceHandler.h"

PersistenceHandler::PersistenceHandler(ParamHandler* handler, char* sha1path):
	paramHandler(handler) {
	memcpy(this->sha1path,"Persistence/",12);
	memcpy(&(this->sha1path[12]), sha1path, 40);
	this->sha1path[52]=0;
	printf("shapath %s\n", this->sha1path);
}
	
PersistenceHandler::~PersistenceHandler() {

}

int PersistenceHandler::writeParamsToFile() {
	printf("PERSISTENCE: write params\n");
 	int fd = open(this->sha1path, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
 	//int filedesc = open("test", O_WRONLY | O_APPEND);
 
    if (fd < 0) {
    	printf("PERSISTENCE: couldn't open file\n");
        return -1;
    }
 	list<struct opt_param_t*>* paramsList = paramHandler->getParams();
 	list<struct opt_param_t*>::iterator paramsListIt;
 	for(paramsListIt = paramsList->begin(); paramsListIt != paramsList->end(); paramsListIt++) {
 		write(fd,"1",1);
 		if (write(fd, (void*) *paramsListIt, sizeof(struct opt_param_t)) != sizeof(struct opt_param_t)) {
	       	printf("ERROR: writing parameters to persistence file\n");
	        return -1;
	    }
 	}
	write(fd,"0",1);
    close(fd);
 
    return 0;
}

int PersistenceHandler::readParamsFromFile() {
	printf("PERSISTENCE: read params\n");
 	int fd = open(this->sha1path, O_RDWR | O_APPEND);
 	//int filedesc = open("test", O_WRONLY | O_APPEND);
 
    if (fd < 0) {
    	printf("PERSISTENCE: couldn't open file\n");
        return -1;
    }

    struct opt_param_t curParam;
 	char isNext;
 	list<struct opt_param_t*>* paramsList = paramHandler->getParams();
 	list<struct opt_param_t*>::iterator paramsListIt;

 	while(true) {
 		read(fd, (void*) &isNext, 1);
 		printf("isnext %d", (int) isNext);
 		if(isNext == '0') {
 			return 0;
 		}
 		read(fd, (void*) &curParam, sizeof(struct opt_param_t));
 		printf("curparam value: %d\n", curParam.curval);
 		for(paramsListIt = paramsList->begin(); paramsListIt != paramsList->end(); paramsListIt++) {
	 		if (curParam.address == (*paramsListIt)->address
	 				&& curParam.min == (*paramsListIt)->min
	 				&& curParam.max == (*paramsListIt)->max
	 				&& curParam.curval <= curParam.max
	 				&& curParam.curval >= curParam.min) {
	 			(*paramsListIt)->curval = curParam.curval;
	 			(*paramsListIt)->changed = true;
		    }
	 	}
 	}
    close(fd);
 
    return 0;
}