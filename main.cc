/*
author: Daniel Nichols
date: February 2022
*/
// stl includes
#include <iostream>

// local includes
#include "bitvector.h"

int main(int argc, char** argv) {
    
    bitvector::BitVector bitvector(100);
    bitvector::RankSupport rank(bitvector);
    bitvector::SelectSupport select(bitvector);

}