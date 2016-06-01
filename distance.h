#ifndef FGRAPH_DISTANCE_H_
#define FGRAPH_DISTANCE_H_
#include <stdlib.h>
#include <vector>
#include <set>

template<typename T>
struct Candidate {
    size_t row_id;
    T distance;
    Candidate(const size_t row_id, const T distance): row_id(row_id), distance(distance) { }

    bool operator >(const Candidate& rhs) const {
        if (this->distance == rhs.distance) {
            return this->row_id > rhs.row_id;
        }
        return this->distance > rhs.distance;
    }
};

template<typename T>
class Distance {
public:
    virtual float compare(const T* a, const T* b, size_t length) const = 0;
    virtual ~Distance() {}
};

template<typename T>
class L2Distance: public Distance<T> {
public:
    typedef T ResultType;
    /**
     * Copied from flann
     * We do not want msse intrinstic here to avoid misalign problems.
     */
    ResultType compare(const T* a, const T* b, size_t size) const {
        ResultType result = ResultType();
        ResultType diff0, diff1, diff2, diff3;
        const T* last = a + size;
        const T* lastgroup = last - 3;

        /* Process 4 items with each loop for efficiency. */
        while (a < lastgroup) {
            diff0 = (ResultType)(a[0] - b[0]);
            diff1 = (ResultType)(a[1] - b[1]);
            diff2 = (ResultType)(a[2] - b[2]);
            diff3 = (ResultType)(a[3] - b[3]);
            result += diff0 * diff0 + diff1 * diff1 + diff2 * diff2 + diff3 * diff3;
            a += 4;
            b += 4;
        }
        /* Process last 0-3 pixels.  Not needed for standard vector lengths. */
        while (a < last) {
            diff0 = (ResultType)(*a++ - *b++);
            result += diff0 * diff0;
        }
        return result;
    }
};


#endif
