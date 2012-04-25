#import <iostream>
#import <vector>
#import <string>
#import <sstream>
#import <assert.h>
#import "neural.hpp"

template<class T> string vector2str(vector<T> input) {
    stringstream ss;
    ss << "[";
    for (int i = 0; i < input.size(); i++) {
        ss << input[i];
        if (i != input.size()-1)
            ss << ", ";
    }
    ss << "]";

    return ss.str();
      
}

int main() {

    neuralNetwork nnet(1,2,3);
    cout << "Number of layers: " << nnet.weights.size() << endl;

    // Validate the correct number of layers
    assert (nnet.weights.size() == 3);
    
    // Validate that we have the correct number of weights.

    // (node 0,0 has 1,x edges)
    assert (nnet.weights[0][0].size() == nnet.weights[1].size());

    // (node 1,0 has 2,x edges)
    assert (nnet.weights[1][0].size() == nnet.weights[2].size());
    
    vector<double> input(1);
    input[0] = 23.3;
    vector<double> output = nnet.feedForward(input);



    cout << "Input:  " << vector2str(input) << endl;
    cout << "Inputv: " << vector2str(nnet.node_value[0]) << endl;
    cout << "Layer2v:" << vector2str(nnet.node_value[1]) << endl;
    cout << "Output: " << vector2str(output) << endl;

    return 0;
}
