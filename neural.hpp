#import <vector>
#import <cmath>
#import <assert.h>

using namespace std;

double sigmoid(double in) {
    return 1.0/(1.0 + exp(-in));
}

template<class V> vector<V> multiplyVectors(vector<V> a, vector<V> b) {
    // Taking two vectors of the same length, multiply them and
    // return the result as a vector.

    assert(a.size() == b.size());

    vector<V> res(a.size());
    for (int i = 0; i < a.size(); i++)
        res[i] = a[i]*b[i];

    return res;
}

template<class V> double sumVector(vector<V> a) {
    // Taking two vectors of the same length, multiply them and
    // return the result as a vector.

    double res = 0;
    for (int i = 0; i < a.size(); i++)
        res += a[i];

    return res;
}

struct neuralNetwork {

    /* weights[layer][src_node][dest_node] */
    vector< vector< vector<double> > > weights;

    /* node_value[layer][node] */
    vector< vector<double> > node_value;

    neuralNetwork(int input_nodes, int output_nodes, int layer2_nodes /*, ...*/) {
        /* Default configuration is input -> layer2 -> output.
           Of course, each of these can have an arbitrary number of nodes. */

        weights.resize(3); // 1 input + 1 output + 1 hidden.
        
        weights[0].resize(input_nodes);
        weights[1].resize(layer2_nodes);
        weights[2].resize(output_nodes);

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < weights[i].size(); j++)
                weights[i][j].resize(weights[i+1].size(), .001*(i*j));
        }

        node_value.resize(3);
        node_value[0].resize(input_nodes);
        node_value[1].resize(layer2_nodes);
        node_value[2].resize(output_nodes);
        

    }

    vector<double> feedForward(vector<double> input) {
        assert(input.size() == weights[0].size());


        // Copy the node values from the input nodes.
        for (int i = 0; i < input.size(); i++) 
            node_value[0][i] = input[i];


        // For each hidden node, get all the weights*nodes, sum them, and sigmoid.
        for (int i = 0; i < weights[1].size(); i++) {
            node_value[1][i] = sigmoid( 
                                       sumVector( multiplyVectors(
                                                                  node_value[0], 
                                                                  weights[1][i])))-.5;
        }
            
        // For each output node, get all the l2 weights*nodes, sum them, and sigmoid.
        for (int i = 0; i < weights[2].size(); i++) {
            node_value[2][i] = sigmoid(sumVector(multiplyVectors(node_value[1], 
                                                                 weights[2][i])))-.5;
        }

        /*

        // Calculate the hidden node values
        for (int i = 0; i < weights[0].size(); i++) {
            for (int h = 0; h < weights[1].size(); h++) {
                node_value[1][h] += node_value[0][i]*weights[0][i][h];
            }
        }

        // Apply sigmoid to the hidden node values.
        for (int i = 0; i < weights[1].size(); i++)
            node_value[1][i] = sigmoid(node_value[1][i]);

        // Calculate the output node values
        for (int i = 0; i < weights[1].size(); i++) {
            for (int o = 0; o < weights[2].size(); o++) {
                node_value[2][o] += node_value[1][i] * weights[1][i][o];
            }
        }

        // Apply sigmoid to output values
        for (int i= 0; i < node_value[2].size(); i++) 
            node_value[2][i] = sigmoid(node_value[2][i]);
        */
        return node_value[2];
    }

};
