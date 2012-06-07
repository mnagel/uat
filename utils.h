#pragma once

#include <string>
#include <time.h>

#define FD_STDOUT 1
#define FD_STDIN 0

using namespace std;

void errorExit(string msg); 
void diff(timespec* start, timespec* end, timespec* diff);
bool isTimespecLower(timespec* first, timespec* second);
double getRelativePerformance(timespec* first, timespec* second);
