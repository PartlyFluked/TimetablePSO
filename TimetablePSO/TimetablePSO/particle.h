
#include "University.h""

class ParticleSpace;

class Particle {
public:
	ParticleSpace *pSpace;							//space particle is contained in
	int partID;										//ID of particle

	vector<double> velocity;						//velocity
	vector<double> position;						//position
	double score = 99999999;						//fitness score

	vector<double> perBestPos;						//personal best
	double perBestScore = 99999999;

	vector<double> locBestPos;						//local best
	double locBestScore = 99999999;

	Particle(ParticleSpace *pSpace, int ID);		//constructor

	string inspectPart(void);						//inspect particle
	void evalFitness(University *uni);				//evaluates fitness
	void updateVelocity(vector<double> &gBestPos);	//updates velocity
	void updatePosition(void);						//updates position
};

class ParticleSpace {
public:
	vector<Particle> particles;						//vector of particles
	vector<double> globalBestPos;					//global best position
	double globalBestScore = 99999999;				//global best fitness
	int worstScore;									//worst possible fitness

	int iteration = 0;								//current time step

	int numParticles;								//number of particles
	int numClasses;									//number of classes
	int numIterations;								//number of iterations
	int numDimensions;								//number of dimensions

	vector<vector<bool>> edgeMap;					//neighborhood connectivity map
	vector<int> explored;							//positions explored

	double averageScore = 0;						//average score between all particles
	double averageVelocity = 0;						//average velocity
	vector<double> averagePosition;						//average position

	University *uniSpace;							//University instance used

	bool altTopology = false;						//alternate topology flag

	bool rankedPref = false;						//ranked preferences flag
	int edgeMethod = 0;								//edge case method used
	bool densRes = false;							//density resistance flag
	bool gravWell = false;							//gravity well flag

	double inertia = 0.7298;						//previous velocity weighting
	double cognitive = 1.49618;						//personal exploration weighting
	double social = 1.49618;						//local exploration weighting

	int iters = 0;									//iters through innermost loop

	int bounds[2] = { NUMHOURS, NUMDAYS };			//bounds of dimensions

	ParticleSpace(University *uni, int numParticles, int maxIterations, char enableAltTop, char enableRankedPref, char edgeMethod, char enableDensRes, char enableGravWell); //constructor

	void step(void);								//update fitnesses, positions and velocities
	void connect(void);								//increase neighborhood connectivity
	string inspectSpace(void);						//print space info
	string inspectParticles(void);					//print all particle info
};