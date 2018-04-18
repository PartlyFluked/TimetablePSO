
#include <string>
#include <vector>
#include <array>

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <conio.h>

#include <chrono>
#include <random>

#include <fstream>

#define NUMDAYS 5
#define NUMHOURS 8
#define CLASSMAX 40

using namespace std;

class Class
{
public:
	int ID, day, time;
	Class(int ID, int day = 0, int time = 0) : ID(ID), day(day), time(time) {};
	//~Class();
};

class University;

class Student
{
public:					
	vector<vector<Class>> rankedPreferred; //vector of ranked preferences for each class

	University *uni; //University that student is contained in

	Student(University *uni, int numClasses);
	//~Student();
};

class University
{
public:
	vector<Class> classes; //Units
	vector<int> counts; //number of students desiring each unit
	vector<Student> students; //Students

	bool realPreferences = false;
	bool dupeClasses = false;

	default_random_engine generator;
	normal_distribution<double> distribution;

	double timeWeights[5][8] = {
		{ 0.41, 0.64, 0.83, 0.91, 0.87, 0.77, 0.68, 0.61 },
		{ 0.44, 0.75, 0.95, 1, 0.96, 0.88, 0.75, 0.61 },
		{ 0.45, 0.66, 0.73, 0.77, 0.82, 0.8, 0.71, 0.6 },
		{ 0.39, 0.61, 0.71, 0.72, 0.73, 0.72, 0.66, 0.56 },
		{ 0.29, 0.41, 0.5, 0.59, 0.63, 0.59, 0.5, 0.41 } }; //values estimated from googlemaps popularity graph of reid library
	double totalWeights = 26.62;

	long long iters = 0;

	University(int numClasses, int numStudents, char realPref, char enableDupeClasses);
	//~University();
};