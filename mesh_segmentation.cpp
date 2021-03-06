#include "mesh_segmentation.h"

Vector_3 normal(Facet& f){
  typename Facet::Halfedge_handle h = f.halfedge();
  
  return CGAL::cross_product(
			     h->next()->vertex()->point() - h->vertex()->point(),
			     h->next()->next()->vertex()->point() - h->next()->vertex()->point());
}
//direction of the halfedge
Vector_3 direction(HE_handle he) {
  return (he -> vertex() -> point() -
	  he -> prev() -> vertex() -> point() ); 
}

MeshSegmentation::MeshSegmentation(Polyhedron& _p) :
  ratio(0.05) {
  //the ratio of edges which need to be extract from the mesh
  mesh = _p;
  //label all the halfedges as not feature
  for (auto hei = mesh.halfedges_begin();
       hei != mesh.halfedges_end(); hei++)
    mesh_status[hei] = NOT_FEATURE;
}

void MeshSegmentation::compute_sod() {
  std::vector<double> sodvalues;
  
  for(auto ei = mesh.edges_begin();
      ei != mesh.edges_end(); ei++){
    HE_handle he(ei);
    //get the two adjacent facets
    Facet_handle f1 = he -> facet();
    Facet_handle f2 = he -> opposite() -> facet();
    //first get it running, maybe
    Vector_3 n1 = normal(*f1);
    Vector_3 n2 = normal(*f2);

    sod[ei] = acos( (n1/sqrt(n1.squared_length())) * (n2/sqrt(n2.squared_length())) );
    sodvalues.push_back(sod[ei]);
    //std::cout << direction(ei) << std::endl;
  }
  
  std::sort(sodvalues.begin(), sodvalues.end(),
	    [](double a, double  b) -> bool
	    {
	      return a > b;
	    });
  threshold = sodvalues[int(ratio * sodvalues.size())];

  for(Edge_iterator ei = mesh.edges_begin();
      ei != mesh.edges_end(); ei++){
    if(sod[ei] > threshold)
      edges_with_high_sod.push_back(ei);
  }
}

double MeshSegmentation::sharpness(std::vector<HE_handle>& s) {
  double sum;
  for (auto he : s)
    sum += sod[he];

  return sum;
}

//Levy02's method to expand feature curve
void MeshSegmentation::expand_feature_curve(HE_handle start) {
  const int MAX_STRING_LENGTH = 5;
  const int MIN_FEATURE_LENGTH = 15;

  std::vector<HE_handle> detected_feature;
  
  for (auto he : {start, start -> opposite()}) {
    HE_handle heprime = he;

    std::vector<HE_handle> path;
    //use dfs to determine S
    do {
      path.erase( path.begin(), path.end() );
      std::stack<HE_handle> s;
      //path to store the maximum value path

      enum seach_status {DISCOVERED, NOT_DISCOVERED};
      std::map<HE_handle, seach_status> dfs_status;
      //keep the parent node of the half edge
      std::map<HE_handle, HE_handle> parent_map;
      std::map<HE_handle, double> sharpness_map;
      //keep the deapth at the half edge
      std::map<HE_handle, int> deapth_map;
      //mark all the edges as not discoverd
      for (HE_iterator he = mesh.halfedges_begin();
      	   he != mesh.halfedges_end(); he++)
      	dfs_status[he] = NOT_DISCOVERED;
      
      s.push(heprime);
      deapth_map[heprime] = 0;
      sharpness_map[heprime] = sod[heprime];
      HE_handle max_he;
      double max_sharpness = 0;

      //dfs loop
      while( !s.empty() ) {
	auto he1 = s.top();
	s.pop();
	if (dfs_status[he1] == NOT_DISCOVERED &&
	    deapth_map[he1] < MAX_STRING_LENGTH) {
	  dfs_status[he1] = DISCOVERED;
	  //push all the adjacent halfedges to stack
	  Vertex_handle v = he1 -> opposite() -> vertex();
	  
	  //for(HV_circulator hc = v -> vertex_begin();
	  //hc != v -> vertex_begin(); hc++) {
	  HV_circulator hc = v -> vertex_begin();
	  //loop through the neighbour of the vertex
	  do{
	    hc++;
	    //Polyhedron::Halfedge halfedge = *hc;
	    HE_handle hh = hc;
	    //ToDo Direction of the new edge is almost in the same
	    //direction as hprime
	    if(dfs_status[hh] == NOT_DISCOVERED &&
	       mesh_status[hh] == NOT_FEATURE ) {
	      s.push(hh);
	      parent_map[hh] = he1;
	      deapth_map[hh] = deapth_map[he1] + 1;
	      double current_sharpness = sharpness_map[he1] + sod[hh];
	      sharpness_map[hh] = current_sharpness;
	      if(current_sharpness > max_sharpness) {
		max_sharpness = current_sharpness;
		max_he = hh;
	      }
	      
	    }else{
	      continue;
	    }
	  }while(hc != v -> vertex_begin());
	  
	}else{
	  dfs_status[he1] = DISCOVERED;
	  continue;
	}
      }//end while
      //make the path from the parent map
      HE_handle he_tmp = max_he;
      
      while (parent_map.count(he_tmp) != 0 ) {
	path.push_back(he_tmp);
	he_tmp = parent_map[he_tmp]; 
      }
      size_t path_sz = path.size();
      
      //only for testing
      //std::cout << path_sz << std::endl;
      if(path.size() == 0) break;
      
      heprime = path.back();
      detected_feature.push_back(heprime);
      
    } while( sharpness(path) > MAX_STRING_LENGTH * threshold );
    //std::cout << detected_feature.size() << std::endl;
  }//endfor

  if(detected_feature.size() > MIN_FEATURE_LENGTH ) {
    for(auto h : detected_feature) {
      mesh_status[h] = FEATURE;
    
      Vertex_handle v = h -> opposite() -> vertex();
      //it's neighbour marked as feature neighbour
      for(HV_circulator hc = v -> vertex_begin();
	  hc != v -> vertex_begin(); hc++) {
	Polyhedron::Halfedge halfedge = *hc;
	HE_handle hh = &halfedge;
	
	if(mesh_status[hh] == NOT_FEATURE) {
	  mesh_status[hh] = FEATURE_NEIGHBOUR;
	}
      }
    }
  }
}//end function

void MeshSegmentation::compute_feature() {
  for(auto h : edges_with_high_sod){
    HE_handle he = h;
    std::cout << sod[h] << '\t' << sod[he] << std::endl;
    expand_feature_curve(h);
  }
}

