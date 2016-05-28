#ifndef CMULTRACK_H
#define CMULTRACK_H
#define ARMA_USE_LAPACK
#define ARMA_USE_BLAS
#define ARMA_BLAS_UNDERSCORE
#ifdef _WIN32
#include <armadilloWin32/armadillo>
#else
#include <armadilloLinux/armadillo>
#endif
using namespace std;
using namespace arma;
typedef struct  {
    float          az ,rg,daz,drg;
    short          size;
    float          x,y;
    //bool           selected;
    float          p;
}m_mark;
class CMultrack
{
public:
    CMultrack();
};

#endif // CMULTRACK_H
