#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>

#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>


#include "mesh_segmentation.h"


int main(){
    Polyhedron mesh;
    std::ifstream input("bunny.off");
    if ( !input || !(input >> mesh) || mesh.empty() ) {
        std::cerr << "Not a valid off file." << std::endl;
        return EXIT_FAILURE;
    }

    MeshSegmentation ms(mesh);
    ms.compute_sod();
}
