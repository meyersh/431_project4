#include <iostream>
#include <sys/time.h>
#include <cmath>
//#include <rand>

using namespace std;

int main() {
    struct timeval t;
    struct timezone tz;
    gettimeofday(&t, &tz);

    cout << t.tv_usec << endl;

    srand(t.tv_usec);

    for (int i = 0; i<25; i++)
        cout << log(rand())-20 << endl;
}
