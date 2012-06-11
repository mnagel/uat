#pragma once

#include <string>
#include <time.h>
#include <list>
#include <vector>

#define FD_STDOUT 1
#define FD_STDIN 0

using namespace std;

void errorExit(string msg); 
timespec diff(timespec start, timespec end);
long long timespecToLongLong(timespec ts);
bool isTimespecLower(timespec first, timespec second);
int getRelativePerformance(timespec* first, timespec* second);
int sortedInsert(std::list<int>* l, int i);
int sortedInsert(std::vector<int>* l, int i);
int sortedInsert(std::list<int*>* l, int* i);
int sortedInsert(std::vector<int*>* l, int* i);
