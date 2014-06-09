#include "mesh_segmentation.h"

Vector_3 normal(Facet& f){
  typename Facet::Halfedge_handle h = f.halfedge();
  
  return CGAL::cross_product(
          h->next()->vertex()->point() - h->vertex()->point(),
          h->next()->next()->vertex()->point() - h->next()->vertex()->point());
}

MeshSegmentation::MeshSegmentation(Polyhedron& _p) :
  ratio(0.05) {
  //the ratio of edges which need to be extract from the mesh
  mesh = _p;
}

void MeshSegmentation::compute_sod() {
  for(Edge_iterator ei = mesh.edges_begin();
      ei != mesh.edges_end(); ei++){
    HE_handle he(ei);
    //get the two adjacent facets
    Facet_handle f1 = he -> facet();
    Facet_handle f2 = he -> opposite() -> facet();
    //first get it running, maybe
    Vector_3 n1 = normal(*f1);
    Vector_3 n2 = normal(*f2);

    sod[ei] = acos( (n1/sqrt(n1.squared_length())) * (n2/sqrt(n2.squared_length())) );
  }
}
