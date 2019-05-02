#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include "quicksort.h"
#include "ratio.h"
#include "sort_small_arrays.h"

template< typename T>
unsigned partition( T* a, unsigned begin, unsigned end) {
	unsigned i = begin, last = end-1;
	T pivot = a[last];

	for (unsigned j=begin; j<last; ++j) {
		if ( a[j]<pivot ) {
			std::swap( a[j], a[i] );
			++i;
		}
	}
	std::swap( a[i], a[last] );
	return i;
}

template< typename T>
unsigned partition_new( T* a, unsigned begin, unsigned end) {
    if ( end-begin > 8 ) return partition_old( a, begin, end );

	unsigned i = begin, last = end-1, step = (end-begin)/4;

    T* pivots[5] = { a+begin, a+begin+step, a+begin+2*step, a+begin+3*step, a+last };
    quicksort_base_5_pointers( pivots );

	std::swap( a[last], a[begin+2*step] );
	T pivot = a[last];
    
    for (unsigned j=begin; j<last; ++j) {
		if ( a[j]<pivot /*|| a[j]==pivot*/ ) {
			std::swap( a[j], a[i] );
			++i;
		}
	}
	std::swap( a[i], a[last] );
	return i;
}

/* recursive */
template< typename T>
void quicksort_rec( T* a, unsigned begin, unsigned end )
{
    if ( end-begin<6 ) {
        switch ( end-begin ) {
            case 5: quicksort_base_5( a+begin ); break;
            case 4: quicksort_base_4( a+begin ); break;
            case 3: quicksort_base_3( a+begin ); break;
            case 2: quicksort_base_2( a+begin ); break;
        }
        return;
    }

	unsigned q = partition(a,begin,end);
 	
	quicksort_rec(a,begin,q);
	quicksort_rec(a,q,end);
}

/* iterative */
#define STACK
#define xVECTOR
#define xPRIORITY_QUEUE 

#include <utility> // std::pair

template <typename T>
using triple = typename std::pair< T*, std::pair<unsigned,unsigned>>;

template< typename T>
struct compare_triples {
    bool operator() ( triple<T> const& op1, triple<T> const& op2 ) const {
        return op1.second.first > op2.second.first;
    }
};

#ifdef STACK
#include <stack>
template< typename T>
using Container = std::stack< triple<T>>;
#define PUSH push
#define TOP  top
#define POP  pop
#endif

#ifdef VECTOR
#include <vector>
template< typename T>
using Container = std::vector< triple<T>>;
#define PUSH push_back
#define TOP  back
#define POP  pop_back
#endif

#ifdef PRIORITY_QUEUE
#include <queue>
template< typename T>
using Container = std::priority_queue< triple<T>, std::vector<triple<T>>, compare_triples<T> >;
#define PUSH push
#define TOP  top
#define POP  pop
#endif

template< typename T>
void quicksort_iterative_aux( Container<T> & ranges );

template< typename T>
void quicksort_iterative( T* a, unsigned begin, unsigned end )
{
    Container<T> ranges;
    ranges.PUSH( std::make_pair( a, std::make_pair( begin,end ) ) );
    quicksort_iterative_aux( ranges );
}

template< typename T>
void quicksort_iterative_aux( Container<T> & ranges )
{
    while ( ! ranges.empty() ) {
        triple<T> r = ranges.TOP();
        ranges.POP();
        
        T*       a = r.first;
        unsigned b = r.second.first;
        unsigned e = r.second.second;
        
        //base case
        if (e-b<6) {
            switch ( e-b ) {
                case 5: quicksort_base_5( a+b ); break;
                case 4: quicksort_base_4( a+b ); break;
                case 3: quicksort_base_3( a+b ); break;
                case 2: quicksort_base_2( a+b ); break;
            }
            continue;
        }

        unsigned q = partition(a,b,e);

        ranges.PUSH( std::make_pair( a, std::make_pair( b,q ) ) );
        ranges.PUSH( std::make_pair( a, std::make_pair( q+1,e ) ) );
    }
}


#include <stack>
template<typename T>
class threadsafe_stack
{
    private:
        std::stack< triple<T>> data;
        mutable std::mutex m;
    public:
        threadsafe_stack(): data(), m(){}

        threadsafe_stack(const threadsafe_stack& other):data(),m()
        {
            std::lock_guard<std::mutex> lock(other.m);
            data=other.data;
        }
        
        threadsafe_stack& operator=(const threadsafe_stack&) = delete;
        
        void PUSH(triple<T> new_value)
        {
            std::lock_guard<std::mutex> lock(m);
            data.push(std::move(new_value));
        }
        
        std::shared_ptr<triple<T>> POP()
        {
            std::lock_guard<std::mutex> lock(m);
            if(data.empty()) return nullptr;
            std::shared_ptr<triple<T>> const res( std::make_shared<triple<T>>( std::move( data.top() ) ) );
            data.pop();
            return res;
        }
        
        bool empty() const
        {
            std::lock_guard<std::mutex> lock(m);
            return data.empty();
        }

        unsigned size() const
        {
            std::lock_guard<std::mutex> lock(m);
            return data.size();  
        }
};

std::atomic<int> num_correct;

/// general case for quicksort_aux (needed for compile)
template<typename T> void quicksort_aux( threadsafe_stack<T> & ranges, int size){}


/// auxiliary function for quicksort< T=int >
template<>
void quicksort_aux<int>( threadsafe_stack<int> & ranges, int size)
{

    while ( num_correct < size ) {
        std::shared_ptr<triple<int>> r = ranges.POP();
        if (r==nullptr) continue;
        
        int*       a = r.get()->first;
        unsigned b = r.get()->second.first;
        unsigned e = r.get()->second.second;
        
        //base case
        if (e-b<6) {
            switch ( e-b ) {
                case 5: quicksort_base_5( a+b ); num_correct+=5; break;
                case 4: quicksort_base_4( a+b ); num_correct+=4; break;
                case 3: quicksort_base_3( a+b ); num_correct+=3; break;
                case 2: quicksort_base_2( a+b ); num_correct+=2; break;
                case 1: num_correct+=1; break;
            }
            // std::cout << b << " " << e << "     :" << num_correct << std::endl;
            continue;
        }

        unsigned q = partition(a,b,e);

        num_correct+=1;
        // std::cout << b << " " << e << "     :" <<num_correct << std::endl;
        ranges.PUSH( std::make_pair( a, std::make_pair( b,q ) ) );
        ranges.PUSH( std::make_pair( a, std::make_pair( q+1,e ) ) );
    }
}

/// auxiliary function for quicksort< T=Ratio >
template<>
void quicksort_aux<Ratio>( threadsafe_stack<Ratio> & ranges, int size)
{
    while ( num_correct < size ) {
        std::shared_ptr<triple<Ratio>> r = ranges.POP();
        if (r==nullptr) continue;

        Ratio*       a = r.get()->first;
        unsigned b = r.get()->second.first;
        unsigned e = r.get()->second.second;
        
        //base case
        if (e-b<6) {
            switch ( e-b ) {
                case 5: quicksort_base_5( a+b ); num_correct+=5; break;
                case 4: quicksort_base_4( a+b ); num_correct+=4; break;
                case 3: quicksort_base_3( a+b ); num_correct+=3; break;
                case 2: quicksort_base_2( a+b ); num_correct+=2; break;
                case 1: num_correct+=1; break;
            }
            continue;
        }

        unsigned q = partition(a,b,e);

        num_correct+=1;
        ranges.PUSH( std::make_pair( a, std::make_pair( b,q ) ) );
        ranges.PUSH( std::make_pair( a, std::make_pair( q+1,e ) ) );
    }
}



template< typename T>
void quicksort(T* a, unsigned begin , unsigned end, int num_threads){
/// For array of 200 Ratios w/ delay, 
/// Generally, thread number: multiples of 8 are faster
///     Fastest: 64 threads ran in ~1.49 sec

/* ************Tested with following ************************
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
******************************************************** */
    std::atomic_init(&num_correct, 0); //initialize to 0
    threadsafe_stack<T> ranges;
    ranges.PUSH( std::make_pair( a, std::make_pair( begin,end ) ) );
    std::vector<std::thread> threads;
    for (int i=0; i<num_threads; ++i){
        threads.push_back(std::thread(quicksort_aux<T>, std::ref(ranges), static_cast<int>(end)));
    }

    for (auto& th : threads) th.join();
}
