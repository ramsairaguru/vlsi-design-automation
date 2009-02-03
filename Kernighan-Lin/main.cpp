#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <climits>
#include <cstdlib>
using namespace std;

// Step 1:
//   V = set of 2n elements
//   {A, B} is an initial partition of V such that
//   A and B are equal-sized subsets of V with no overlap and their union covering all of V.

// Step 2:
//   Compute Dv for all v in V
//   Initialize variables:
//     queue = null;
//     iteration = 1;
//     A' = A;
//     B' = B;

// Step 3:
//   Choose ai from A' and bi from B' which maximizes
//     gi = Dai + Dbi - 2caibi
//   Add pair (ai, bi) to queue
//   Remove ai from A' and bi from B'

// Step 4:
//   If A' and B' are both empty then goto Step 5
//   Else
//     Re-calculate D-values for A' union B'
//     Increment iteration and goto Step 3
//   Endif

// Step 5:
//   Find k to maximize the partial sum of gi from i to k in the queue
//   If G > 0 then
//     Move X = {a1, ..., ak} to B and Y = {b1, ..., bk} to A
//     Goto Step 2
//   Else STOP
//   Endif

typedef vector<bool> LocationVector;
typedef vector<int> GainVector;

class AdjacencyMatrix {
	vector<map<unsigned int, unsigned short> > C;
public:
	int n;
	AdjacencyMatrix(const char* filename) {
		int stage = 0;
		int a, b = 0;
		string line;
		ifstream infile(filename);
		if (infile.is_open())
		{
			while (infile.good() )
			{
				getline(infile,line);
				switch(stage) {
					case 0:
						if(sscanf(line.c_str(),"%d",&n) == EOF) {
							cerr << "Could not find number of nodes in file." << endl;
							exit(1);
						}
						C.assign(n, map<unsigned int, unsigned short>());
						stage = 1;
						break;
					case 1:
						// Ignore the number of nets
						stage = 2;
						break;
					case 2:
						if(sscanf(line.c_str(),"%d %d",&a, &b) == EOF) {
							//cerr << "Ignoring Malformed netlist line: " << line << endl;
							break;
						}
						// Convert 1-based to 0-based
						a--;
						b--;
						incrementCost(a, b);
						break;
					default:
						cerr << "Invalid parse stage reached!";
						exit(3);
						break;
				}
			}
			infile.close();
		}
		else {
			cout << "Unable to open file" << endl;
			exit(1);
		}
	}
	
	unsigned short getCost(unsigned int a, unsigned int b) {
		map<unsigned int, unsigned short>::iterator i;
		map<unsigned int, unsigned short> row = C[a];
		i = row.find(b);
		if(i == row.end())
			return 0;
		else
			return i->second;
	}
	
	void incrementCost(unsigned int a, unsigned int b) {
		// Duplicate in both places
		C[a][b] = getCost(a, b) + 1;
		C[b][a] = getCost(b, a) + 1;
	}
	
	void prettyPrint() {
		cerr << "  ";
		for(int i = 0; i < n; i++) {
			cerr << i << " ";
		}
		cerr << endl;
		for(int i = 0; i < n; i++) {
			cerr << i << " ";
			for(int j = 0; j < n; j++) {
				cerr << getCost(i, j) << " ";
			}
			cerr << endl;
		}
	}
	
	unsigned int computeCutset(LocationVector& V) {
		int total = 0;
		for(int i = 0; i < n; i++) {
			bool setOfi = V[i];
			map<unsigned int, unsigned short> row = C[i];
			for(map<unsigned int, unsigned short>::iterator j = row.begin(); j != row.end(); j++) {
				unsigned short cost = j->second;
				if(setOfi != V[j->first] and cost != 0) {
					total+= cost;
				}
			}
		}
		// Divide by 2 because connections will be counted twice.
		return total / 2;
	}
	
	void initializeDValues(GainVector& D, LocationVector& V) {
		D.clear();
		int Di = 0;
		for(int i = 0; i < n; i++) {
			bool setOfi = V[i];
			Di = 0;
			map<unsigned int, unsigned short> row = C[i];
			for(map<unsigned int, unsigned short>::iterator j = row.begin(); j != row.end(); j++) {
				unsigned short cost = j->second;
				if(setOfi == V[j->first]) {
					Di -= cost;
				}
				else {
					Di += cost;
				}
			}
			D.push_back(Di);
		}
	}
};
class GainQueueItem {
public:
	int indexA;
	int indexB;
	int netGain;
	GainQueueItem(int A, int B, int gain) {
		indexA = A;
		indexB = B;
		netGain = gain;
	}
};
typedef vector<GainQueueItem> GainQueue;

void randomizeVector(LocationVector& V) {
	int n = V.size();
	int n2 = (int)(n/2);
	int rand_index = 0;
	for(int i = 0; i < n2;) {
		rand_index = rand() % n;
		if(!V[rand_index]) {
			V[rand_index] = true;
			i++;
		}
	}
}

void choosePair(GainQueueItem& gi, AdjacencyMatrix& C, LocationVector& V, GainVector& D, vector<bool>& locks) {
	int n = C.n;
	for(int i = 0; i < n; i++) {
		if(locks[i]) continue;
		bool set = V[i];
		for(int j = i + 1; j < n; j++) {
			if(locks[j] or set == V[j]) continue;
			int gab = (D[i] + D[j]) - (2 * C.getCost(i, j));
			//cout << "g" << i << "," << j << ": " << gab << endl;
			if(gab > gi.netGain) {
				// Found a better candidate
				gi.indexA = i;
				gi.indexB = j;
				gi.netGain = gab;
			}
		}
	}
}

void choosePairGreedy(GainQueueItem& gi, AdjacencyMatrix& C, LocationVector& V, GainVector& D, vector<bool>& locks) {
	int n = C.n;
	int bestA = -1;
	int bestB = -1;
	int Da = INT_MIN;
	int Db = INT_MIN;
	int Di = INT_MIN;
	for(int i = 0; i < n; i++) {
		if(locks[i]) continue;
		Di = D[i];
		if(V[i]) {
			// Set B
			if(Di > Db) {
				Db = Di;
				bestB = i;
			}
		} else {
			// Set A
			if(Di > Da) {
				Da = Di;
				bestA = i;
			}
		}
	}
	gi.indexA = bestA;
	gi.indexB = bestB;
	gi.netGain = (Da + Db) - (2 * C.getCost(bestA, bestB));
}

void recalculateD(GainQueueItem& gi, AdjacencyMatrix& C, LocationVector& V, GainVector& D, vector<bool>& locks) {
	int A = gi.indexA;
	int B = gi.indexB;
	for(int i = 0; i < C.n; i++) {
		if(locks[i]) continue;
		int cia = C.getCost(i, A);
		int cib = C.getCost(i, B);
		if(cia != 0 or cib != 0) {
			if(V[i] == V[A]) {
				D[i] += 2*cia - 2*cib;
			} else {
				D[i] += 2*cib - 2*cia;
			}
		}
	}
}

int main (int argc, char * const argv[]) {
	if(argc == 1) {
		cerr << "Usage: " << argv[0] << "[[input_file] seed]"<< endl;
		exit(0);
	}
	char* filename = "";
	if(argc > 1)
		filename = argv[1];
	
	long seed = time(NULL);
	if(argc > 2)
		seed = atoi(argv[2]);

	// Set the random seed
	srand(seed);
	cerr << "Using seed: " << seed << endl;
	
	
	// Declare Variables
    // Parse the input file into the adjacency matrix
	cerr << "Reading file: " << filename << endl;
	AdjacencyMatrix C = AdjacencyMatrix(filename);
	int n = C.n;
	int n2 = (int)(n/2);
	GainVector D;	
	LocationVector V = LocationVector(n, false);
	GainQueue queue;
	vector<bool> locks(n, false);

	//C.prettyPrint();

	// Create a random starting solution
	cerr << "Creating a random starting solution..." << endl;
	randomizeVector(V);
	/*
	cerr << "V: ";
	for(int i = 0; i < n; i++) {
		char buffer[4]; sprintf(buffer, "%3d", (bool)V[i]);
		cerr << buffer << " ";
	}
	cerr << endl;
	*/
	
	// Compute the initial cutset size
	cerr << "Computing the inital cutset size..." << endl;
	int cutset = C.computeCutset(V);
	cerr << "Initial cutset: " << cutset << endl;
	cerr << "Iteration cutset: " << cutset << endl;
	cerr << "Generation cutset: " << cutset << endl;

	int maxTotalGain = 1;
	for(int generation = 0; maxTotalGain > 0; generation++) {
		//cerr << "Initializing Generation Variables..." << endl;
		maxTotalGain = INT_MIN;
		int maxTotalGainIndex = -1;
		int currentTotalGain = 0;
		
		// Initialize the D values for all nodes
		//cerr << "Initializing D values..." << endl;
		C.initializeDValues(D, V);
		/*
		cerr << "D: ";
		for(int i = 0; i < n; i++) {
			char buffer[4]; sprintf(buffer, "%3d", D[i]);
			cerr << buffer << " ";
		}
		cerr << endl;
		 */
		
		// Step 3 and 4
		for(int iteration = 0; iteration < n2; iteration++) {
			//cerr << "Choosing best-gain pair..." << endl;
			GainQueueItem gi(0, 0, INT_MIN);
			choosePairGreedy(gi, C, V, D, locks);
			//cerr << "Chosing: " << gi.indexA << ", " << gi.indexB << ": " << gi.netGain << endl << endl;
			// Lock the selected nodes for this iteration
			locks[gi.indexA] = true;
			locks[gi.indexB] = true;
			// Push the pair onto the queue and see if it's the best subqueue
			queue.push_back(gi);
			currentTotalGain += gi.netGain;
			if(currentTotalGain > maxTotalGain) {
				maxTotalGain = currentTotalGain;
				maxTotalGainIndex = iteration;
			}
			// Recalculate D values for still-active nodes
			if(iteration + 1 < n2) {
				recalculateD(gi, C, V, D, locks);
				
				/*
				cerr << "D: ";
				for(int i = 0; i < n; i++) {
					char buffer[4]; sprintf(buffer, "%3d", D[i]);
					cerr << buffer << " ";
				}
				cerr << endl;
				*/
			}
		}
		/*
		cerr << "Queue: " << endl;
		for(int i = 0; i < queue.size(); i++) {
			GainQueueItem gi = queue[i];
			cerr << i << " (" << gi.indexA << "," << gi.indexB << "): " << gi.netGain << endl;
		}
		*/
		cerr << "Best Generation Gain: " << maxTotalGain << endl;
		
		// Apply the optimum subqueue
		//cerr << "Applying the subqueue..." << endl;
		if(maxTotalGain > 0) {
			for(int i = 0; i <= maxTotalGainIndex; i++) {
				GainQueueItem gi = queue[i];
				V[gi.indexA].flip();
				V[gi.indexB].flip();
				cutset -= gi.netGain;
				cerr << "Iteration cutset: " << cutset << endl;
			}
			/*
			cerr << "V: ";
			for(int i = 0; i < n; i++) {
				char buffer[4]; sprintf(buffer, "%3d", (bool)V[i]);
				cerr << buffer << " ";
			}
			cerr << endl;
			*/
			
			cerr << "Gereation cutset: " << cutset << endl;
		}
		// Unlock all nodes
		locks.assign(n, false);
		// Clear the queue
		queue.clear();		
	}
	cutset -= maxTotalGain;
	cerr << "Final cutset: " << cutset << endl;
	//cerr << "Final cutset check: " << C.computeCutset(V) << endl;
	
	// Report the final output
	cout << cutset << endl;

	bool first = true;
	for(int i = 0; i < n; i++) {
		if(!V[i]) {
			if(!first)
				cout << " ";
			else
				first = false;
			cout << i;
		}
	}
	cout << endl;
	
	first = true;
	for(int i = 0; i < n; i++) {
		if(V[i]) {
			if(!first)
				cout << " ";
			else
				first = false;
			cout << i;
		}
	}
	cout << endl;
	
    return 0;
}
