#include <stdio.h>
#include "floatfann.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

template <class V> string printArray(V *array, int len) {
    stringstream ss;
    ss << "[";
    for (int i = 0; i < len; i++) {
        ss << array[i];
        if (i < len-1)
            ss << ", ";
    }

    ss << "]";

    return ss.str();
}

int main()
{
    fann_type *calc_out;
    fann_type input[8];

    struct fann *ann = fann_create_from_file("identity_float.net");

    input[0] = 0;
    input[1] = 1;
    input[2] = 0;
    input[3] = 0;
    input[4] = 0;
    input[5] = 0;
    input[6] = 0;
    input[7] = 0;

    calc_out = fann_run(ann, input);

    cout << "Identity test:" << endl
         << printArray(input, 8) << endl
         << printArray(calc_out, 8) << endl;
    

    fann_destroy(ann);
    return 0;
}
