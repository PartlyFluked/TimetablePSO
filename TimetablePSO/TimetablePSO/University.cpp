#include "University.h"

University::University(int numClasses, int numStudents, char realPref, char enableDupeClasses) { //Initialize classes and students
	if (realPref == 'y') { this->realPreferences = true; }
	if (enableDupeClasses == 'y') { this->dupeClasses = true; }
	for (int i = 0; i < numClasses; i++) {
		Class *unit = new Class(i);
		classes.push_back(*unit);
		counts.push_back(0);
	}
	for (int i = 0; i < numStudents; i++) {
		Student *pupil = new Student(this, numClasses);
		students.push_back(*pupil);
	}
	if (this->dupeClasses) { //If duplicate classes is set, creates duplicates of popular classes such that scheduling can be partitioned between them
		for (int i = 0; i < numClasses; i++) {
			for (int j = 0; j < counts[i] / CLASSMAX; j++) {
				Class *dupe = new Class(i);
				classes.push_back(*dupe);
			}
		}
	}
}

Student::Student(University *uni, int numClasses) {
	this->uni = uni;
	vector<int> classIds;

	default_random_engine generator;
	normal_distribution<double> distribution(numClasses/2, numClasses/4);

	while (classIds.size() < 4) { //Student chooses 4 random units, popularized over a normal distribution if real preferences flag is set
		int classId = rand() % numClasses;
		if (uni->realPreferences) { classId = distribution(generator); 
			if (classId < 0) { classId = 0; }
			if (classId > numClasses - 1) { classId = numClasses - 1; }
		}
		bool found = false;
		for (auto &i : classIds) {
			if (i == classId) { found = true; break; }
		}
		if (!found) { 
			classIds.push_back(classId); 
			uni->counts[classId]++;
		}
	}
	while (rankedPreferred.size() < 4) { //Create preferences for each unit
		vector<array<int, 2>> rankedPreference;
		while (rankedPreference.size() < 5) { //Creates 5 rankings for each unit
			int day, time;
			if (uni->realPreferences) { //Distributes chosen timeslots over a weighting scheme if real preferences flag is set
				float roulette = (((float)rand()) / ((float)RAND_MAX)) * uni->totalWeights;
				int point = -1;
				while (roulette > 0) {
					point++;
					roulette -= uni->timeWeights[point / 8][point % 8];
				}
				day = point / 8;
				time = point % 8;
			}
			else {
				day = rand() % NUMDAYS;
				time = rand() % NUMHOURS;
			}
			array<int, 2> ar = { day, time };
			bool found = false;
			for (auto &i : rankedPreference) {
				if (ar == i) { found = true; break; }
			}
			if (!found) { rankedPreference.push_back(ar); }
		}
		vector<Class> rankedClasses;
		for (int i = 0; i < 5; i++) {
			Class *choice = new Class(classIds[rankedPreferred.size()], rankedPreference[i][0], rankedPreference[i][1]);
			rankedClasses.push_back(*choice);
		}
		rankedPreferred.push_back(rankedClasses);
	}
}