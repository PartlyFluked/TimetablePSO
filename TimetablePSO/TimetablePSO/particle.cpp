#include "particle.h"

ParticleSpace::ParticleSpace(University *uni, int numParticles, int maxIterations, char enableAltTop, char enableRankedPref, char edgeMethod, char enableDensRes, char enableGravWell) {		//initialize space of particles
	if (enableAltTop == 'y') { altTopology = true; }
	if (enableRankedPref = 'y') {
		this->rankedPref = true;
	}
	this->edgeMethod = edgeMethod - '0';
	if (enableDensRes = 'y') {
		this->densRes = true;
	}
	if (enableGravWell = 'y') {
		this->gravWell = true;
	}
	this->numParticles = numParticles;
	numClasses = uni->classes.size();
	numIterations = maxIterations;
	numDimensions = numClasses * 2;
	for (int i = 0; i < numDimensions; i++) {
		averagePosition.push_back(0);
	}
	for (int i = 0; i < numParticles; i++) {
		Particle *newPart = new Particle(this, i);
		particles.push_back(*newPart);
		edgeMap.push_back(vector<bool>(numParticles, false)); //create empty edge map
	}
	worstScore = uni->students.size()*(20 + 3 * 10); //worst possible fitness score
	uniSpace = uni; //University instance used
}

Particle::Particle(ParticleSpace *pSpace, int ID) {		//initialize particle to random position and velocity
	this->pSpace = pSpace;
	this->partID = ID;

	for (int dim = 0; dim < pSpace->numDimensions; dim++) {
		position.push_back(((float)rand() / (float)RAND_MAX) * pSpace->bounds[dim % 2]);
		velocity.push_back(2 * (((float)rand() / (float)RAND_MAX) - 0.5));
	}
}

string Particle::inspectPart(void) {
	return "Position[0]: " + to_string(position[0]) + "/tVelocity[0]: " + to_string(velocity[0]);
}

void Particle::evalFitness(University *uni) {
	int tempScore = 0;
	#pragma omp parallel for reduction(+:tempScore) //parallelize for loop, combines tempScore between threads at the end
	for (int stu = 0; stu < uni->students.size(); stu++) {
		int classTimes[4][2] = { { -1,-1 },{ -1,-1 },{ -1,-1 },{ -1,-1 } }; //class timeslots given
		int classNum = 0;
		for (auto &preferredClasses : uni->students[stu].rankedPreferred) {
			int potentialDay, potentialTime, bestClassScore = 999999999;
			//If dupeClasses is set and a class has more than CLASSMAX students, multiple duplicate classes are created.
			//This loop picks the best of these duplicates with the lowest score
			//Greatly improves optimisation at the cost of CPU time but not directly part of the PSO algorithm
			for (int dupeNum = 0; dupeNum < (uni->counts[preferredClasses[0].ID] / CLASSMAX) + 1; dupeNum++) { 
				uni->iters++;
				int matches = 0, clashes = 0;
				int dim; //dimension
				if (dupeNum > 0) {
					int skip = dupeNum; //Classes in the uni's class array to skip in order to get to the right duplicate.
					for (int classNum = 0; classNum < uni->classes.size(); classNum++) {
						if (uni->classes[classNum].ID == preferredClasses[0].ID) { skip--; }
						if (skip == 0) { dim = classNum * 2; break; }
					}
				}
				else {
					dim = preferredClasses[0].ID * 2;
				}
				int tempPotentialDay = position[dim];
				int tempPotentialTime = position[dim + 1];
				for (auto &time : classTimes) {
					if (time[0] == tempPotentialDay && time[1] == tempPotentialTime) { clashes += 1; break; } //If another of the student's units has the same timeslot, adds a clash point.
				}
				if (pSpace->rankedPref) {
					for (int rank = 0; rank < 5; rank++) {
						if (tempPotentialDay == preferredClasses[rank].day && tempPotentialTime == preferredClasses[rank].time) { //Designated class time matches one of the student's ranked preferences.
							matches += (5 - rank);
							break;
						}
					}
				}
				else {
					if (tempPotentialDay == preferredClasses[0].day && tempPotentialTime == preferredClasses[0].time) { //Designated class time perfectly matches student preference.
						matches += 5;
					}
				}
				int currScore = (5 - matches) + clashes * 10;
				if (currScore < bestClassScore) { potentialDay = tempPotentialDay; potentialTime = tempPotentialTime; bestClassScore = currScore; } //Chooses best score out of the duplicate classes
				if (!uni->dupeClasses) { break; }
			}
			classTimes[classNum][0] = potentialDay;
			classTimes[classNum][1] = potentialTime;
			classNum++;		
			tempScore += bestClassScore;
		}
	}
	score = tempScore; //update particle's fitness score
	if (score < perBestScore) { //update personal bests
		perBestScore = score;
		perBestPos = position;
	}
	//cout << inspectPart();
}

void Particle::updateVelocity(vector<double> &gBestPos) {
	#pragma omp parallel for  //parallelizes for loop
	for (int j = 0; j < pSpace->numDimensions; j++) {
		double rand1 = ((float)rand()) / ((float)RAND_MAX);
		double rand2 = ((float)rand()) / ((float)RAND_MAX);
		double rand3 = ((float)rand()) / ((float)RAND_MAX);
		// Calculate change in velocity due to cognitive element.
		double cognitiveVelocity = 0;
		if (pSpace->edgeMethod == 3) { //If edgecase = torus
			double dist = perBestPos[j] - position[j];
			if (abs(dist) > ((float)pSpace->bounds[j % 2]) / 2) { 
				if (dist > 0) { dist = pSpace->bounds[j % 2] - dist; }
				else { dist = dist + pSpace->bounds[j % 2]; }
			}
			cognitiveVelocity = pSpace->cognitive*rand1*(dist);
		}
		else {
			cognitiveVelocity = pSpace->cognitive*rand1*(perBestPos[j] - position[j]);
		}
		// Calculate change in velocity due to social element.
		double socialVelocity = 0;
		if (locBestPos.size() != 0) {
			if (pSpace->edgeMethod == 3) { //if edgecase = torus
				double dist = locBestPos[j] - position[j];
				if (abs(dist) > ((float)pSpace->bounds[j % 2]) / 2) {
					if (dist > 0) { dist = pSpace->bounds[j % 2] - dist; }
					else { dist = dist + pSpace->bounds[j % 2]; }
				}
				socialVelocity = pSpace->social*rand2*(dist);
			}
			else { socialVelocity = pSpace->social*rand2*(locBestPos[j] - position[j]); }
		}
		double pressureVelocity = 0;
		if (pSpace->densRes) { pressureVelocity = -0.2*rand3*(pSpace->averagePosition[j] - position[j]); }
		velocity[j] = (pSpace->inertia*velocity[j] + cognitiveVelocity + socialVelocity + pressureVelocity);
	}
}

void Particle::updatePosition() {
	#pragma omp parallel for //parallelizes for loop
	for (int j = 0; j < position.size(); j++) {
		double tempPos = position[j] + velocity[j];
		if (tempPos > pSpace->bounds[j % 2]) { // Position exceeds upper limit.
			position[j] = pSpace->bounds[j % 2]-0.0001;
			switch (pSpace->edgeMethod) { //updates position based on edge case
				case 1: velocity[j] = 0;
						break;
				case 2: velocity[j] = -velocity[j]; 
						position[j] = 2 * pSpace->bounds[j % 2] - tempPos;
						break;
				case 3: position[j] = 0;
						break;
			}
		}
		else if (tempPos < 0) { // Position is less than lower limit.
			position[j] = 0;
			switch (pSpace->edgeMethod) { //updates position based on edge case
				case 1: velocity[j] = 0;
						break;
				case 2: velocity[j] = -velocity[j]; position[j] = -1 * tempPos;
						break;
				case 3: position[j] = pSpace->bounds[j % 2] - 0.0001;
						break;
			}
		}
		else {
			position[j] = tempPos;
		}
	}
}

void ParticleSpace::step(void) {
	for (int i = 0; i < numParticles; i++) {
		particles[i].updatePosition();
		particles[i].evalFitness(uniSpace);
		if (particles[i].perBestScore < globalBestScore) { //update global bests
			globalBestScore = particles[i].perBestScore;
			globalBestPos = particles[i].perBestPos;
		}
		if (altTopology) { //If alternate topology flag is set, update neighborhood bests
			for (int j = i; j < numParticles; j++) {
				if (edgeMap[i][j]) {
					if (particles[i].perBestScore < particles[i].locBestScore) {
						if (particles[j].perBestScore < particles[i].perBestScore) {
							particles[i].locBestPos = particles[j].perBestPos;
							particles[i].locBestScore = particles[j].perBestScore;
							particles[j].locBestPos = particles[j].perBestPos;
							particles[j].locBestScore = particles[j].perBestScore;
						}
						else {
							particles[i].locBestPos = particles[i].perBestPos;
							particles[i].locBestScore = particles[i].perBestScore;
							particles[j].locBestPos = particles[i].perBestPos;
							particles[j].locBestScore = particles[i].perBestScore;
						}
					}
				}
			}
		}
		else {
			particles[i].locBestScore = globalBestScore;
			particles[i].locBestPos = globalBestPos;
		}
	}

	averageScore = 0;
	averageVelocity = 0;
	fill(averagePosition.begin(), averagePosition.end(), 0);

	for (int i = 0; i < numParticles; i++) {
		averageScore += particles[i].score;
		for (int j = 0; j < numDimensions; j++ ) {
			averagePosition[j] += particles[i].position[j]/numParticles;
			averageVelocity += abs(particles[i].velocity[j]);
		}
		particles[i].updateVelocity(globalBestPos);
	}

	averageScore = averageScore / particles.size();			//update averages
	averageVelocity = averageVelocity / particles.size();
}

void ParticleSpace::connect(void) { //turns disconnected graph into a connected graph over the iterations
	for (int i = 0; i < numParticles; i++) {
		double newEdges = 2*((float)(numParticles - 1) / numIterations);
		if ((float)rand() / (float)RAND_MAX < newEdges) {
			while (newEdges > 0) {
				int tempRand = rand() % numParticles;
				edgeMap[i][tempRand] = true;
				edgeMap[tempRand][i] = true;
				newEdges--;
			}
		}
	}
}

string ParticleSpace::inspectSpace(void) {
	string output = "Step:  " + to_string(iteration) + "    Best Position:  [";
	for (int dim = 0; dim < numDimensions; dim++) {
		output += to_string(globalBestPos[dim]) + " ";
	}
	return output + "]    Best Score:  " + to_string(globalBestScore);
}

string ParticleSpace::inspectParticles(void) {
	string output;
	for (int i = 0; i < numParticles; i++) {
		double dist = 0, fit = 0;
		output += "Particle " + to_string(i) + ": ";
		for (int dim = 0; dim < numDimensions; dim++) {
			dist += abs(particles[i].position[dim]);
			cout << to_string(particles[i].position[dim]) << " ";
		}
		"\tDistance: " + to_string(dist / numDimensions) + "\tScore:  " + to_string(particles[i].score) + "\n";
	}
	return output;
}