#ifndef _MESH_SEGMENTATION
#define _MESH_SEGMENTATION

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/mesh_segmentation.h>

#include <CGAL/Polyhedron_traits_with_normals_3.h>

#include <CGAL/property_map.h>
//CGAL::angle
//#include <CGAL/global_functions.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
typedef CGAL::Point_3<Kernel> Point;
typedef Polyhedron::Facet Facet;
typedef Polyhedron::Vertex_iterator Vertex_iterator;
typedef Polyhedron::Halfedge_iterator HE_iterator;
typedef Polyhedron::Halfedge_handle HE_handle;
typedef Polyhedron::Facet_handle Facet_handle;
typedef Polyhedron::Edge_iterator Edge_iterator;
typedef Polyhedron::Halfedge_around_vertex_const_circulator  HV_circulator;

typedef Kernel::Vector_3 Vector_3;

typedef std::map<Polyhedron::Edge_iterator, double> Edge_double_map;

Vector_3 normal(Facet& f);

class MeshSegmentation {
 public:
  MeshSegmentation(Polyhedron& _p);
  ~MeshSegmentation(){};

  void compute_sod();
 private:
  Polyhedron mesh;
  const double ratio;
  Edge_double_map sod;
};

#endif
