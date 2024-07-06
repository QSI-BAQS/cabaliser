#include "consts.h"
#include "tableau.h"

void test_cache_assumptions()
{
    assert(0 == (CACHE_SIZE % sizeof(CHUNK_OBJ)));

}


int main()
{
    test_cache_assumptions();
    return 0;
}
