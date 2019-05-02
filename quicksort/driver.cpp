#include <iostream> //cout, endl
#include <cstdlib>  //srand
#include <algorithm>//copy, random_shuffle
#include <random>   //random_device, mt19937
#include <iterator> //ostream_iterator
#include <chrono>
#include "quicksort.h"
#include "ratio.h"

using namespace std::chrono;

template< typename T>
bool check_is_sorted( T* a, int size )
{
    for ( int i=1; i<size; ++i ) {
        if ( ! ( a[i-1] <= a[i] ) ) {
            return false;
        }
    }
    return true;
}

bool test_int( int size, unsigned num_threads ) {
    int* a = new int[size];
	for ( int i=0; i<size; ++i ) { a[i] = i; }
    std::random_device rd;
    std::mt19937 gen( rd() );
    std::random_shuffle( a, a+size );
    quicksort( a, 0, size, num_threads );
    bool retval = check_is_sorted( a, size );
    delete [] a;
    return retval;
}
bool test_ratio( int size, unsigned num_threads ) {
    Ratio* a = new Ratio[size];
	for ( int i=0; i<size; ++i ) { Ratio r( 1, i+1 ); a[i] = r; }
    std::random_device rd;
    std::mt19937 gen( rd() );
    std::random_shuffle( a, a+size );
    quicksort( a, 0, size, num_threads );
    bool retval = check_is_sorted( a, size );
    delete [] a;
    return retval;
}

void test0() {
    if ( test_int( 200, 1 ) ) { std::cout << "OK\n"; }
    else                      { std::cout << "Failed\n"; }
}
void test1() {
    if ( test_int( 200, 2 ) ) { std::cout << "OK\n"; }
    else                      { std::cout << "Failed\n"; }
}
void test2() {
    if ( test_int( 20000, 4 ) ) { std::cout << "OK\n"; }
    else                      { std::cout << "Failed\n"; }
}
void test3() {
    if ( test_int( 20000, 8 ) ) { std::cout << "OK\n"; }
    else                      { std::cout << "Failed\n"; }
}
void test4() { // 1.575s
    if ( test_ratio( 100, 1 ) ) { std::cout << "OK\n"; }
    else                      { std::cout << "Failed\n"; }
}
void test5() { // 1.045s
    if ( test_ratio( 100, 2 ) ) { std::cout << "OK\n"; }
    else                      { std::cout << "Failed\n"; }
}
void test6() { // 0.862s
    if ( test_ratio( 100, 4 ) ) { std::cout << "OK\n"; }
    else                      { std::cout << "Failed\n"; }
}
void test7() { // 0.862s
    if ( test_ratio( 100, 8 ) ) { std::cout << "OK\n"; }
    else                      { std::cout << "Failed\n"; }
}
void test8() { // 
    if ( test_ratio( 200, 1 ) ) { std::cout << "OK\n"; }
    else                      { std::cout << "Failed\n"; }
}
void test9() { // 
    if ( test_ratio( 200, 2 ) ) { std::cout << "OK\n"; }
    else                      { std::cout << "Failed\n"; }
}
void test10() { //
    if ( test_ratio( 200, 4 ) ) { std::cout << "OK\n"; }
    else                      { std::cout << "Failed\n"; }
}
void test11() { //
    if ( test_ratio( 200, 200 ) ) { std::cout << "OK\n"; }
    else                      { std::cout << "Failed\n"; }
}
void test12() {
    unsigned long const hardware_threads= std::thread::hardware_concurrency();
    std::cout << " Number of cores " << hardware_threads << "\n";

    unsigned fastest_threadNum_sofar = 1;
    float time_sofar = 1000000000.0;
    for (unsigned i = 1; i < 200; ++i){
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        test_ratio(200, i);
        high_resolution_clock::time_point t2 = high_resolution_clock::now();

        auto duration = duration_cast<microseconds>( t2 - t1 ).count();
        if (static_cast<float>(duration) < time_sofar){
            time_sofar = static_cast<float>(duration);
            fastest_threadNum_sofar = i;
        }
        std::cout << "threadNum : " << i << " running time: " << duration << std::endl;

    }

    std::cout << "fastest_threadNum : " << fastest_threadNum_sofar << " running time: " << time_sofar << std::endl;

}

void (*pTests[])() = { 
    test0,test1,test2,test3,test4,test5,test6,test7,test8,test9,test10,test11,test12
}; 

#include <cstdio>    /* sscanf */
int main( int argc, char ** argv ) {
    if (argc==2) { //use test[ argv[1] ]
		int test = 0;
		std::sscanf(argv[1],"%i",&test);
		try {
            pTests[test]();
		} catch( const char* msg) {
			std::cerr << msg << std::endl;
		}
        return 0;
	}
}
