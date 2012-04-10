#ifndef __WLM_HPP__
#define __WLM_HPP__

#define WEIGHTS_FILE "weights.txt"
#define ETA_FILE "eta.txt"
#define DEFAULT_ETA .001

/*
 * Weighted Learning Machine . hpp
 * Shaun Meyer, Mar 2012
 *
 *
 * Where in RL, V is our PERFECT function, and \hat{v} is an approximation
 * we shall consider V to be the best we can do.
 * 
 * It is made up of weights, such as
 *  w0 + x1w1 + xiwi + x(i+1)w(i+1) + ... + xnwn.
 * which are determined by the least mean squares method:
 * 
 * (Vtrain(b) - V(b))^2; 
 * E(rror) => wi <- wi + n(Vtrain(b) - V(b))*Xi
 *
 * Successor(b) reveals the n+1 state in the game-trace.
 *
 * Game state: A game has a state, a set of attributes which we care about, 
 * that the V*() functions will use: Careful to distinguish it from the
 * current game configuration (eg, state is the number of black pieces while
 * configuration is their positions on the board).
 * 
 */


#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ctime>
#include <sys/time.h>
#include <cstdlib>
#include <cmath>

using namespace std;

/* 
 * State class
 *
 * A thin wrapper around a vector of ints.
 *
 * Defaults to size 0, allows insert(int).
 *
 * Overrides subscript operator for direct access
 * and default constructor may take a length.
 */

struct State 
{

	vector<int> x;

	State (unsigned int);

	/* Prototypes */
	int size() {return x.size();};
	int& operator[] (const int);
	void insert(int);
	string toString();

   
};

/* State Object functions */

State::State(unsigned int len=0) {
	if (len)
		x.resize(len);
}

void State::insert(int i) {
	x.push_back(i);
}

int& State::operator[] (const int i) {
	return x[i];
}

string State::toString() {
	stringstream ss;

	for (int i = 0; i < x.size(); i++)
		{
			ss << x[i];
			if (i+1 != x.size())
				ss << ", ";
		}

	return ss.str();
}

/*
 * Weight class 
 */

struct Weight
{

	vector<double> w;
	string filename;
	double eta;

	Weight(string filename=WEIGHTS_FILE);
    Weight(Weight &w);
    ~Weight();
    int size() {return w.size();};
    double& operator[] (const int);
    int save();
    int load(); 
    void insert(int i);
    string toString();
    void adjust(State b, double error);
    double Vhat(State b);
    double random();
    void load_eta();

};

/* Weight Object functions */

Weight::Weight(string filename) {
	this->filename = filename;
	load_eta();
    load();
}

Weight::~Weight() {
}

double& Weight::operator[] (const int i) {
	return w[i];
}

int Weight::save() {
	ofstream weight_file(filename.c_str());

	if (!weight_file.good()) 
		return 1; // can't save the file!

	for (int i = 0; i < w.size(); i++)
		weight_file << w[i] << endl;
   
	weight_file.close();
	return 0;
}

int Weight::load() {
	ifstream weight_file(filename.c_str());

	string line;

	if (!weight_file.good())
		{
			weight_file.close();
			return 1; // can't open the file! 
		}

	w.clear();

	while (getline(weight_file, line)) {
		w.push_back(atof(line.c_str()));
	}

	weight_file.close();
	return 0;
}

void Weight::insert(int i) {
	w.push_back(i);
}

string Weight::toString() {
	stringstream ss;

	for (int i = 0; i < w.size(); i++)
		{
			ss << w[i];
			if (i+1 != w.size())
				ss << ", ";
		}

	return ss.str();
}
	  

double Weight::Vhat(State b) {
	// empty w or b is exceptional.
	if (!b.size() || !w.size())
		throw logic_error("Weight or B is empty.");

	// We have dynamic-length state & weight,
	// calculate for the shorter.
	int len = 0;

	if (b.size() < w.size())
		len = b.size();
	else 
		len = size() - 1;


	// Implicit w0 var.
    //	double result = w[0];
    double result = random()*w[0]; // set w0 randomly.

	// Multiply up the rest of the xi*wi vars.
	for (int i = 0; i < len; i++)
		result += b[i] * w[i+1];

    /*
    if (result > 100)
        return 100;
    else if (result < -100)
        return -100;
    
	return result;
    */
    

    /* Ln() result */
    if (result == 0)
        return 0;
    if (result < 0) {
        result *= -1;
        return log(result)*-10;
    }
    else {
        return log(result)*10;
    }

}

void Weight::adjust(State b, double error) {
	/* Adjust the weights with wi <- wi + n(error)*xi */

	// Ensure that we have as many weights as states.
	if (b.size()+1 != w.size())
		w.resize(b.size()+1);

	for (int i=0; i<w.size(); i++) {
		double modifier =  eta*error*b[i];
		/*		if (modifier)
				cout << "Updating w[" << i << "] => " << modifier << endl; */
		w[i+1] += modifier;
	}
	  
}

double Weight::random() {
    // Return a random number between -16 and 16 
    struct timeval t;
    struct timezone tz;
    gettimeofday(&t, &tz);
    srand(t.tv_usec);
    int r = rand();
    return log(r)-20;

}

void Weight::load_eta() {
    // Attempt to read the eta file from eta.txt, otherwise return
    // DEFAULT_ETA

    ifstream eta_file("eta.txt");

    if (!eta_file)
        eta = DEFAULT_ETA;
    else
        eta_file >> eta;

    eta_file.close();

}

#endif
