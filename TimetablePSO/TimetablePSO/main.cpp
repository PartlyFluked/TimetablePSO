#include "particle.h"

void main() {
	ofstream myfile;
	myfile.open("results.csv");			//output file
	srand(time(NULL));					//seed random number generator

	int numClasses = 0, numStudents = 0, numParticles = 0, maxIterations = 0;
	char enableBaseline = 'y', enableAltTop = 'n', enableRankedPref = 'n', edgeMethod = '0', realPref = 'n', enableDupeClasses = 'n', enableDensRes = 'n', enableGravWell= 'n';
	cout << "Number of Classes?: ";
	cin >> numClasses;
	cout << "Number of Students?: ";
	cin >> numStudents;
	cout << "Number of Particles?: ";
	cin >> numParticles;
	cout << "Number of Iterations?: ";
	cin >> maxIterations;

	cout << "Use baseline algorithm?: (y/n) ";
	cin >> enableBaseline;
	if (enableBaseline == 'n') {
		cout << "Use alternate topology?: (y/n) ";
		cin >> enableAltTop;
		cout << "Use ranked preferences?: (y/n) ";
		cin >> enableRankedPref;
		cout << "Use edge method?: (0) Inertia, (1) Stop, (2) Bounce, (3) Torus ";
		cin >> edgeMethod;
		cout << "Use real-weighted preferences?: (y/n) ";
		cin >> realPref;
		cout << "Use duplicate classes?: (y/n) ";
		cin >> enableDupeClasses;
		cout << "Use density resistance?: (y/n) ";
		cin >> enableDensRes;
		//cout << "Use graivty wells?: (y/n) ";
		//cin >> enableGravWell; //not implemented
	}

	University *uni = new University(numClasses, numStudents, realPref, enableDupeClasses); //University instance
	ParticleSpace pSpace(uni, numParticles, maxIterations, enableAltTop, enableRankedPref, edgeMethod, enableDensRes, enableGravWell); //PSO instance

	//Used for debugging. 1 particle, 1 student, 4 classes.
		//pSpace.uniSpace->students[0].rankedPreferred[0][0].day = 0; pSpace.uniSpace->students[0].rankedPreferred[0][0].time = 0;
		//pSpace.uniSpace->students[0].rankedPreferred[1][0].day = 1; pSpace.uniSpace->students[0].rankedPreferred[1][0].time = 1;
		//pSpace.uniSpace->students[0].rankedPreferred[2][0].day = 2; pSpace.uniSpace->students[0].rankedPreferred[2][0].time = 2;
		//pSpace.uniSpace->students[0].rankedPreferred[3][0].day = 3; pSpace.uniSpace->students[0].rankedPreferred[3][0].time = 3;
		//pSpace.particles[0].position = { 0,0,1,1,2,2,3,3 };

	myfile << "Best score,Average score,Average velocity,Average position[0],Best position\n";

	auto tempTime = chrono::high_resolution_clock::now(); //Get current time

	while (pSpace.iteration < maxIterations) {

		pSpace.step(); //Calculate new fitnesses, positions, velocities

		if (pSpace.altTopology) { //Increases connectivity of graph
			pSpace.connect();
		}

		cout << "Iteration: " << pSpace.iteration << "    Best score: " << pSpace.globalBestScore << "    Average score: " << pSpace.averageScore << "    Average velocity: " << pSpace.averageVelocity << "    Average position[0]: " << pSpace.averagePosition[0] << "    iters: " << uni->iters << endl;
		myfile << pSpace.globalBestScore << "," << pSpace.averageScore << "," << pSpace.averageVelocity << "," << pSpace.averagePosition[0] << ",";
		for (int i = 0; i < pSpace.numDimensions; i++) {
			myfile << pSpace.globalBestPos[i] << ",";
		}
		myfile << ",\n";
		//cout << pSpace.inspectSpace() << endl; //print current best fitness and position
		//cout << pSpace.inspectParticles() << endl; //print details on ALL particles
		pSpace.iteration++;
	}
	int optimisedPercent = 100 - (pSpace.globalBestScore * 100 / pSpace.worstScore);
	auto tempTime2 = chrono::high_resolution_clock::now() - tempTime;  //Get final time
	cout << "Finished in " << chrono::duration_cast<std::chrono::microseconds>(tempTime2).count()/1000000 << " seconds." << endl << "Optimisation score: " << optimisedPercent << endl << "Press any key to close.";
	_getch();
	myfile.close();
}