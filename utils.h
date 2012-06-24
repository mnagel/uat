#pragma once

#include <string>
#include <time.h>
#include <list>
#include <vector>
#include <math.h>

#define FD_STDOUT 1
#define FD_STDIN 0

using namespace std;

void errorExit(string msg); 
timespec diff(timespec start, timespec end);
timespec tsAdd(timespec ts1, timespec ts2);
long long timespecToLongLong(timespec ts);
timespec longLongToTimespec(long long l);
bool isTimespecLower(timespec first, timespec second);
int iround(double d);
int getRelativePerformance(timespec* first, timespec* second);
int sortedInsert(std::list<int>* l, int i);
int sortedInsert(std::vector<int>* l, int i);
int sortedInsert(std::list<int*>* l, int* i);
int sortedInsert(std::vector<int*>* l, int* i);
