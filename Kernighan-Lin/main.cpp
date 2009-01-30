#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// Step 1:
//   V = set of 2n elements
//   {A, B} is an initial partition of V such that
//   A and B are equal-sized subsets of V with no overlap and their union covering all of V.

// Step 2:
//   Compute Dv for all v in V
//   Initialize variables:
//     queue = null;
//     i = 1;
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
//     Increment i and goto Step 3
//   Endif

// Step 5:
//   Find k to maximize the partial sum of gi from i to k in the queue
//   If G > 0 then
//     Move X = {a1, ..., ak} to B and Y = {b1, ..., bk} to A
//     Goto Step 2
//   Else STOP
//   Endif

class AdjacencyMatrix {
	bool* C;
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
							cout << "Could not find number of nodes in file." << endl;
							exit(1);
						}
						C = new bool[n*n];
						// Fill the upper-triangular with false values
						for(int i = 0; i < n; i++) {
							for(int j = i; j < n; j++) {
								C[i*n+j] = false;
							}
						}
						stage = 1;
						break;
					case 1:
						// Ignore the number of nets
						stage = 2;
						break;
					case 2:
						if(sscanf(line.c_str(),"%d %d",&a, &b) == EOF) {
							cout << "Malformed netlist line: " << line << endl;
							exit(2);
						}
						// Convert 1-based to 0-based
						setCost(a-1, b-1, true);
						break;
					default:
						cout << "Invalid parse stage reached!";
						exit(3);
						break;
				}
			}
			infile.close();
		}
		else cout << "Unable to open file" << endl;
	}
	
	bool getCost(int i, int j) {
		// Only use the upper-triangular of the matrix
		return C[min(i, j)*n+max(i, j)];
	}
	
	void setCost(int i, int j, bool b) {
		// Only use the upper-triangular of the matrix
		C[min(i, j)*n+max(i, j)] = b;
	}
	
	void prettyPrint() {
		for(int i = 0; i < n; i++) {
			for(int j = 0; j < n; j++) {
				if(j < i)
					cout << "  ";
				else {
					cout << getCost(i, j) << " ";
				}
			}
			cout << endl;
		}		
	}
};

typedef bool* LocationVector;
typedef int* GainVector;

GainVector initializeDValues(AdjacencyMatrix& m, LocationVector& V) {
	int n = m.n;
	GainVector D = new int[n];
	for(int i = 0; i < n; i++) {
		bool setOfi = V[i];
		int Di = 0;
		for(int j = 0; j < n; j++) {
			int cij = m.getCost(i, j);
			if( V[j] == setOfi ) {
				Di += cij;
			}
			else {
				Di -= cij;
			}
		}
		D[i] = Di;
	}
	return D;
}

LocationVector randomVector(int n, int seed) {
	srand(seed);
	LocationVector V = new bool[n];
	for(int i = 0; i < n; i++) {
		V[i] = false;
	}
	int n2 = (int)(n/2);
	int rand_index = 0;
	for(int i = 0; i < n2;) {
		rand_index = rand() % n;
		if(!V[rand_index]) {
			V[rand_index] = true;
			i++;
		}
	}
	return V;
}

int main (int argc, char * const argv[]) {
    cout << "Reading file..." << endl;
	char* filename = "../../input.txt";
	if(argc > 1)
		filename = argv[1];
	AdjacencyMatrix* C = new AdjacencyMatrix(filename);
	C->prettyPrint();

	int n = C->n;
	LocationVector V = randomVector(n, 72);
	cout << "V: ";
	for(int i = 0; i < n; i++) {
		char buffer[4]; sprintf(buffer, "%3d", V[i]);
		cout << buffer << " ";
	}
	cout << endl;
	
	GainVector D = initializeDValues(*C, V);
	cout << "D: ";
	for(int i = 0; i < n; i++) {
		char buffer[4]; sprintf(buffer, "%3d", D[i]);
		cout << buffer << " ";
	}
	cout << endl;	
	
    return 0;
}
