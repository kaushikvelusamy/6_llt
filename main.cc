/*
  MM = MEMORY MAP
  Memory Reference Map. Entry (i,j) is the number of references
  made by threads on nodelet i to nodelet j. i == j for local references.
  Sum of column j is number of references on nodelet j, excluding remotes.
  Sum of row i, excluding i == j, is number of migrations away from nodelet i.

  Each row in the memory map represents the source nodelet on which a memory
  instruction was encountered and the column represents the destination nodelet
  on which the memory instruction was ultimately executed. Values on the
  diagonal represent local memory accesses and non-diagonal values show the
  number of memory accesses that required a migration to another nodelet to
  complete.

  RM = REMOTES MAP
  Remotes Reference Map. Counts number of remote operations from
  threads on nodelet i to nodelet j.

  Each row in the remotes map is a source nodelet and each column is a
  destination nodelet for a remote memory operation. Note that the diagonal
  values are always zero because a remote update to a local value will be
  counted as a local memory operation. In this example, all the values are zero
  showing that no remote memory operations occurred.
 */
#include <assert.h>
#include <iostream>
#include <tuple>
#include <vector>

#include <cilk.h>
#include <memoryweb.h>

#include "algebra.hh"
#include "types.hh"


int main(int argc, char* argv[])
{
    starttiming();

    FILE *infile = fopen(argv[1], "r");
    if (!infile)
    {
        fprintf(stderr, "Unable to open file: %s\n", argv[1]);
        exit(1);
    }

    IndexArray_t iL, iU, iA;
    IndexArray_t jL, jU, jA;
    Index_t nedgesL = 0;
    Index_t nedgesU = 0;
    Index_t nedgesA = 0;
    Index_t max_id = 0;
    Index_t src, dst;

    while (!feof(infile))
    {
        fscanf(infile, "%ld %ld\n", &src, &dst);
        if (src > max_id) max_id = src;
        if (dst > max_id) max_id = dst;

        if (src < dst)
        {
            iA.push_back(src);
            jA.push_back(dst);

            iU.push_back(src);
            jU.push_back(dst);

            ++nedgesU;
        }
        else if (dst < src)
        {
            iA.push_back(src);
            jA.push_back(dst);

            iL.push_back(src);
            jL.push_back(dst);

            ++nedgesL;
        }
        ++nedgesA;
    }
    fclose(infile);

    std::cout << "Read " << nedgesL << " edges in L." << std::endl;
    Index_t nnodes = max_id + 1;
    std::cout << "#Nodes = " << nnodes << std::endl;
    std::vector<Index_t> v(iL.size(), 1); // matrix values of 1

    Matrix_t L(nnodes);
    L.build(iL.begin(), jL.begin(), v.begin(), nedgesL);

    Matrix_t C(nnodes);

    ABT_Mask_NoAccum_kernel(C, L, L, L);

    return 0;
}
