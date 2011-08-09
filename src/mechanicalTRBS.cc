//
// Filename     : mechanicalTRBS.cc
// Description  : Classes describing updates due to mechanical triangular biquadratic springs
// Author(s)    : Henrik Jonsson (henrik@thep.lu.se)
// Created      : February 2011
// Revision     : $Id:$
//
#include <utility>
#include <vector>
#include "baseReaction.h"
#include "mechanicalTRBS.h"
#include "tissue.h"
#include <cmath>

VertexFromTRBS::
VertexFromTRBS(std::vector<double> &paraValue, 
	       std::vector< std::vector<size_t> > 
	       &indValue ) 
{  
  // Do some checks on the parameters and variable indeces
  if( paraValue.size()!=2 ) {
    std::cerr << "VertexFromTRBS::"
	      << "VertexFromTRBS() "
	      << "Uses two parameters young modulus and poisson coefficient.\n";
    exit(0);
  }
  if( indValue.size()!=1 || indValue[0].size()!=1 ) { 
    std::cerr << "VertexFromTRBS::"
	      << "VertexFromTRBS() "
	      << "Only wall length index given in first level." << std::endl;
    exit(0);
  }
  
  // Set the variable values
  setId("VertexFromTRBS");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  // Set the parameter identities
  std::vector<std::string> tmp( numParameter() );
  tmp[0] = "Y_mod";
  tmp[1] = "P_ratio";
  setParameterId( tmp );
}

void VertexFromTRBS::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  
  //Do the update for each cell
  size_t numCells = T.numCell();
  size_t wallLengthIndex = variableIndex(0,0);
  size_t numWalls = 3; // defined only for triangles at the moment
  
  for( size_t i=0 ; i<numCells ; ++i ) {
    if( T.cell(i).numWall() != numWalls ) {
      std::cerr << "VertexFromTRBS::derivs() only defined for triangular cells."
		<< " Not for cells with " << T.cell(i).numWall() << " walls!"
		<< std::endl;
      exit(-1);
    }
    
    double young = parameter(0);
    double poisson =parameter(1);
    size_t v1 = T.cell(i).vertex(0)->index();
    size_t v2 = T.cell(i).vertex(1)->index();
    size_t v3 = T.cell(i).vertex(2)->index();
    size_t w1 = T.cell(i).wall(0)->index();
    size_t w2 = T.cell(i).wall(1)->index();
    size_t w3 = T.cell(i).wall(2)->index();
    std::vector<double> restingLength(numWalls);
    restingLength[0] = wallData[w1][wallLengthIndex];
    restingLength[1] = wallData[w2][wallLengthIndex];
    restingLength[2] = wallData[w3][wallLengthIndex];

    std::vector< std::vector<double> > position(3,vertexData[v1]);
    position[1] = vertexData[v2];
    position[2] = vertexData[v3];
    //position[0][2] z for vertex 1 (of the cell)
   
    std::vector<double> length(numWalls);
    length[0] = T.wall(w1).lengthFromVertexPosition(vertexData);
    length[1] = T.wall(w2).lengthFromVertexPosition(vertexData);
    length[2] = T.wall(w3).lengthFromVertexPosition(vertexData);
    
    // Lame coefficients (can be defined out of loop)
    double lambda=young*poisson/(1-poisson*poisson);
    double mio=young/(1+poisson);
    
    // Area of the element (using Heron's formula)                                      
    double Area=std::sqrt( ( restingLength[0]+restingLength[1]+restingLength[2])*
                           (-restingLength[0]+restingLength[1]+restingLength[2])*
                           ( restingLength[0]-restingLength[1]+restingLength[2])*
                           ( restingLength[0]+restingLength[1]-restingLength[2])  )*0.25;
    
    //Angles of the element ( assuming the order: 0,L0,1,L1,2,L2 )
    std::vector<double> Angle(3);
    Angle[0]=std::asin(2*Area/(restingLength[0]*restingLength[2]));
    // can be ommited by cotan(A)=.25*sqrt(4*b*b*c*c/K-1)
    Angle[1]=std::asin(2*Area/(restingLength[0]*restingLength[1]));        
    Angle[2]=std::asin(2*Area/(restingLength[1]*restingLength[2]));
    
    //Tensile Stiffness
    double tensileStiffness[3];
    double const temp = 1.0/(Area*16);                                      
    double cotan[3] = {1.0/std::tan(Angle[0]),1.0/std::tan(Angle[1]),1.0/std::tan(Angle[2])};    
    tensileStiffness[0]=(2*cotan[2]*cotan[2]*(lambda+mio)+mio)*temp;
    tensileStiffness[1]=(2*cotan[0]*cotan[0]*(lambda+mio)+mio)*temp;
    tensileStiffness[2]=(2*cotan[1]*cotan[1]*(lambda+mio)+mio)*temp;
    
    //Angular Stiffness
    double angularStiffness[3];
    angularStiffness[0]=(2*cotan[1]*cotan[2]*(lambda+mio)-mio)*temp;
    angularStiffness[1]=(2*cotan[0]*cotan[2]*(lambda+mio)-mio)*temp;
    angularStiffness[2]=(2*cotan[0]*cotan[1]*(lambda+mio)-mio)*temp;
    
    //Calculate biquadratic strains  
    std::vector<double> Delta(3);
    Delta[0]=(length[0])*(length[0])-(restingLength[0])*(restingLength[0]);
    Delta[1]=(length[1])*(length[1])-(restingLength[1])*(restingLength[1]);
    Delta[2]=(length[2])*(length[2])-(restingLength[2])*(restingLength[2]);
    //Forces of vertices
    double Force[3][3];                                           
          
    Force[0][0]= (tensileStiffness[0]*Delta[0]+angularStiffness[1]*Delta[1]+angularStiffness[0]*Delta[2])*(position[1][0]-position[0][0])
                +(tensileStiffness[2]*Delta[2]+angularStiffness[2]*Delta[1]+angularStiffness[0]*Delta[0])*(position[2][0]-position[0][0]); 
    Force[0][1]= (tensileStiffness[0]*Delta[0]+angularStiffness[1]*Delta[1]+angularStiffness[0]*Delta[2])*(position[1][1]-position[0][1])
                +(tensileStiffness[2]*Delta[2]+angularStiffness[2]*Delta[1]+angularStiffness[0]*Delta[0])*(position[2][1]-position[0][1]); 
    Force[0][2]= (tensileStiffness[0]*Delta[0]+angularStiffness[1]*Delta[1]+angularStiffness[0]*Delta[2])*(position[1][2]-position[0][2])
                +(tensileStiffness[2]*Delta[2]+angularStiffness[2]*Delta[1]+angularStiffness[0]*Delta[0])*(position[2][2]-position[0][2]); 

    Force[1][0]= (tensileStiffness[0]*Delta[0]+angularStiffness[0]*Delta[2]+angularStiffness[1]*Delta[1])*(position[0][0]-position[1][0])
                +(tensileStiffness[1]*Delta[1]+angularStiffness[2]*Delta[2]+angularStiffness[1]*Delta[0])*(position[2][0]-position[1][0]); 
    Force[1][1]= (tensileStiffness[0]*Delta[0]+angularStiffness[0]*Delta[2]+angularStiffness[1]*Delta[1])*(position[0][1]-position[1][1])
                +(tensileStiffness[1]*Delta[1]+angularStiffness[2]*Delta[2]+angularStiffness[1]*Delta[0])*(position[2][1]-position[1][1]); 
    Force[1][2]= (tensileStiffness[0]*Delta[0]+angularStiffness[0]*Delta[2]+angularStiffness[1]*Delta[1])*(position[0][2]-position[1][2])
                +(tensileStiffness[1]*Delta[1]+angularStiffness[2]*Delta[2]+angularStiffness[1]*Delta[0])*(position[2][2]-position[1][2]); 

    Force[2][0]= (tensileStiffness[2]*Delta[2]+angularStiffness[0]*Delta[0]+angularStiffness[2]*Delta[1])*(position[0][0]-position[2][0])
                +(tensileStiffness[1]*Delta[1]+angularStiffness[1]*Delta[0]+angularStiffness[2]*Delta[2])*(position[1][0]-position[2][0]); 
    Force[2][1]= (tensileStiffness[2]*Delta[2]+angularStiffness[0]*Delta[0]+angularStiffness[2]*Delta[1])*(position[0][1]-position[2][1])
                +(tensileStiffness[1]*Delta[1]+angularStiffness[1]*Delta[0]+angularStiffness[2]*Delta[2])*(position[1][1]-position[2][1]); 
    Force[2][2]= (tensileStiffness[2]*Delta[2]+angularStiffness[0]*Delta[0]+angularStiffness[2]*Delta[1])*(position[0][2]-position[2][2])
                +(tensileStiffness[1]*Delta[1]+angularStiffness[1]*Delta[0]+angularStiffness[2]*Delta[2])*(position[1][2]-position[2][2]); 

    // adding TRBS forces to the total vertexDerivs
    
    vertexDerivs[v1][0]+= Force[0][0];
    vertexDerivs[v1][1]+= Force[0][1];
    vertexDerivs[v1][2]+= Force[0][2];
    
    vertexDerivs[v2][0]+= Force[1][0];
    vertexDerivs[v2][1]+= Force[1][1];
    vertexDerivs[v2][2]+= Force[1][2];
    
    vertexDerivs[v3][0]+= Force[2][0];
    vertexDerivs[v3][1]+= Force[2][1];
    vertexDerivs[v3][2]+= Force[2][2];
  }
}


VertexFromTRBScenterTriangulation::
VertexFromTRBScenterTriangulation(std::vector<double> &paraValue, 
	       std::vector< std::vector<size_t> > 
	       &indValue ) 
{  
  // Do some checks on the parameters and variable indeces
  if( paraValue.size()!=2 ) {
    std::cerr << "VertexFromTRBScenterTriangulation::"
	      << "VertexFromTRBScenterTriangulation() "
	      << "Uses two parameters young modulus and poisson coefficient.\n";
    exit(0);
  }
  if( indValue.size()!=2 || indValue[0].size()!=1 || indValue[1].size()!=1 ) { 
    std::cerr << "VertexFromTRBScenterTriangulation::"
	      << "VertexFromTRBScenterTriangulation() "
	      << "Wall length index is given in first level." << std::endl
	      << "Start of additional Cell variable indices (center(x,y,z) "
	      << "L_1,...,L_n, n=num vertex) is given in second level." 
	      << std::endl;
    exit(0);
  }
  
  // Set the variable values
  setId("VertexFromTRBScenterTriangulation");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  // Set the parameter identities
  std::vector<std::string> tmp( numParameter() );
  tmp[0] = "Y_mod";
  tmp[1] = "P_ratio";
  setParameterId( tmp );
}

void VertexFromTRBScenterTriangulation::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  
  //Do the update for each cell
  size_t dimension = 3;
  assert (dimension==vertexData[0].size());
  size_t numCells = T.numCell();
  size_t wallLengthIndex = variableIndex(0,0);
  size_t comIndex = variableIndex(1,0);
  size_t lengthInternalIndex = comIndex+dimension;
  
  
  for (size_t i=0 ; i<numCells ; ++i) {
    size_t numWalls = T.cell(i).numWall(); 
    
    if(  T.cell(i).numVertex()!= numWalls ) {
      std::cerr << "VertexFromTRBScenterTriangulation::derivs() same number of vertices and walls."
		<< " Not for cells with " << T.cell(i).numWall() << " walls and "
		<< T.cell(i).numVertex() << " vertices!"	
		<< std::endl;
      exit(-1);
    }
    
    double young = parameter(0);
    double poisson =parameter(1);
    
    // One triangle per 'vertex' in cyclic order
    for (size_t k=0; k<numWalls; ++k) { 
      size_t kPlusOneMod = (k+1)%numWalls;
      //size_t v1 = com;
      size_t v2 = T.cell(i).vertex(k)->index();
      size_t v3 = T.cell(i).vertex(kPlusOneMod)->index();
      //size_t w1 = internal k
      size_t w2 = T.cell(i).wall(k)->index();
      //size_t w3 = internal k+1

      // Position matrix holds in rows positions for com, vertex(k), vertex(k+1)
      std::vector< std::vector<double> > position(3,vertexData[v2]);
      for (size_t d=0; d<dimension; ++d)
	position[0][d] = cellData[i][comIndex+d]; // com position
      //position[1] = vertexData[v2]; // given by initiation
      position[2] = vertexData[v3];
      
      // Resting lengths are from com-vertex(k), vertex(k)-vertex(k+1) (wall(k)), com-vertex(k+1)
      std::vector<double> restingLength(numWalls);
      restingLength[0] = cellData[i][lengthInternalIndex + k];
      restingLength[1] = wallData[w2][wallLengthIndex];
      restingLength[2] = cellData[i][lengthInternalIndex + kPlusOneMod];
      
      // Lengths are from com-vertex(k), vertex(k)-vertex(k+1) (wall(k)), com-vertex(k+1)
      std::vector<double> length(numWalls);
      length[0] = std::sqrt( (position[0][0]-position[1][0])*(position[0][0]-position[1][0]) +
			     (position[0][1]-position[1][1])*(position[0][1]-position[1][1]) +
			     (position[0][2]-position[1][2])*(position[0][2]-position[1][2]) );

      length[1] = T.wall(w2).lengthFromVertexPosition(vertexData);

      length[2] = std::sqrt( (position[0][0]-position[2][0])*(position[0][0]-position[2][0]) +
			     (position[0][1]-position[2][1])*(position[0][1]-position[2][1]) +
			     (position[0][2]-position[2][2])*(position[0][2]-position[2][2]) );
      
      // Lame coefficients (can be defined out of loop)
      double lambda=young*poisson/(1-poisson*poisson);
      double mio=young/(1+poisson);
      
      // Area of the element (using Heron's formula)                                      
      double Area=std::sqrt( ( restingLength[0]+restingLength[1]+restingLength[2])*
			     (-restingLength[0]+restingLength[1]+restingLength[2])*
			     ( restingLength[0]-restingLength[1]+restingLength[2])*
			     ( restingLength[0]+restingLength[1]-restingLength[2])  )*0.25;
      
      //Angles of the element ( assuming the order: 0,L0,1,L1,2,L2 )
      std::vector<double> Angle(3);
      Angle[0]=std::asin(2*Area/(restingLength[0]*restingLength[2]));
      // can be ommited by cotan(A)=.25*sqrt(4*b*b*c*c/K-1)
      Angle[1]=std::asin(2*Area/(restingLength[0]*restingLength[1]));        
      Angle[2]=std::asin(2*Area/(restingLength[1]*restingLength[2]));
      
      //Tensile Stiffness
      double tensileStiffness[3];
      double const temp = 1.0/(Area*16);                                      
      double cotan[3] = {1.0/std::tan(Angle[0]),1.0/std::tan(Angle[1]),1.0/std::tan(Angle[2])};    
      tensileStiffness[0]=(2*cotan[2]*cotan[2]*(lambda+mio)+mio)*temp;
      tensileStiffness[1]=(2*cotan[0]*cotan[0]*(lambda+mio)+mio)*temp;
      tensileStiffness[2]=(2*cotan[1]*cotan[1]*(lambda+mio)+mio)*temp;
      
      //Angular Stiffness
      double angularStiffness[3];
      angularStiffness[0]=(2*cotan[1]*cotan[2]*(lambda+mio)-mio)*temp;
      angularStiffness[1]=(2*cotan[0]*cotan[2]*(lambda+mio)-mio)*temp;
      angularStiffness[2]=(2*cotan[0]*cotan[1]*(lambda+mio)-mio)*temp;
      
      //Calculate biquadratic strains  
      std::vector<double> Delta(3);
      Delta[0]=(length[0])*(length[0])-(restingLength[0])*(restingLength[0]);
      Delta[1]=(length[1])*(length[1])-(restingLength[1])*(restingLength[1]);
      Delta[2]=(length[2])*(length[2])-(restingLength[2])*(restingLength[2]);
      //Forces of vertices
      double Force[3][3];                                           
      
      Force[0][0]= (tensileStiffness[0]*Delta[0]+angularStiffness[1]*Delta[1]+angularStiffness[0]*Delta[2])*(position[1][0]-position[0][0])
               	  +(tensileStiffness[2]*Delta[2]+angularStiffness[2]*Delta[1]+angularStiffness[0]*Delta[0])*(position[2][0]-position[0][0]); 
      Force[0][1]= (tensileStiffness[0]*Delta[0]+angularStiffness[1]*Delta[1]+angularStiffness[0]*Delta[2])*(position[1][1]-position[0][1])
	          +(tensileStiffness[2]*Delta[2]+angularStiffness[2]*Delta[1]+angularStiffness[0]*Delta[0])*(position[2][1]-position[0][1]); 
      Force[0][2]= (tensileStiffness[0]*Delta[0]+angularStiffness[1]*Delta[1]+angularStiffness[0]*Delta[2])*(position[1][2]-position[0][2])
	          +(tensileStiffness[2]*Delta[2]+angularStiffness[2]*Delta[1]+angularStiffness[0]*Delta[0])*(position[2][2]-position[0][2]); 
      
      Force[1][0]= (tensileStiffness[0]*Delta[0]+angularStiffness[0]*Delta[2]+angularStiffness[1]*Delta[1])*(position[0][0]-position[1][0])
	          +(tensileStiffness[1]*Delta[1]+angularStiffness[2]*Delta[2]+angularStiffness[1]*Delta[0])*(position[2][0]-position[1][0]); 
      Force[1][1]= (tensileStiffness[0]*Delta[0]+angularStiffness[0]*Delta[2]+angularStiffness[1]*Delta[1])*(position[0][1]-position[1][1])
	          +(tensileStiffness[1]*Delta[1]+angularStiffness[2]*Delta[2]+angularStiffness[1]*Delta[0])*(position[2][1]-position[1][1]); 
      Force[1][2]= (tensileStiffness[0]*Delta[0]+angularStiffness[0]*Delta[2]+angularStiffness[1]*Delta[1])*(position[0][2]-position[1][2])
	          +(tensileStiffness[1]*Delta[1]+angularStiffness[2]*Delta[2]+angularStiffness[1]*Delta[0])*(position[2][2]-position[1][2]); 
      
      Force[2][0]= (tensileStiffness[2]*Delta[2]+angularStiffness[0]*Delta[0]+angularStiffness[2]*Delta[1])*(position[0][0]-position[2][0])
	          +(tensileStiffness[1]*Delta[1]+angularStiffness[1]*Delta[0]+angularStiffness[2]*Delta[2])*(position[1][0]-position[2][0]); 
      Force[2][1]= (tensileStiffness[2]*Delta[2]+angularStiffness[0]*Delta[0]+angularStiffness[2]*Delta[1])*(position[0][1]-position[2][1])
	          +(tensileStiffness[1]*Delta[1]+angularStiffness[1]*Delta[0]+angularStiffness[2]*Delta[2])*(position[1][1]-position[2][1]); 
      Force[2][2]= (tensileStiffness[2]*Delta[2]+angularStiffness[0]*Delta[0]+angularStiffness[2]*Delta[1])*(position[0][2]-position[2][2])
	          +(tensileStiffness[1]*Delta[1]+angularStiffness[1]*Delta[0]+angularStiffness[2]*Delta[2])*(position[1][2]-position[2][2]); 
      
      // adding forces to the total vertexDerivs
      
      cellDerivs[i][comIndex  ]+= Force[0][0];
      cellDerivs[i][comIndex+1]+= Force[0][1];
      cellDerivs[i][comIndex+2]+= Force[0][2];
      
      vertexDerivs[v2][0]+= Force[1][0];
      vertexDerivs[v2][1]+= Force[1][1];
      vertexDerivs[v2][2]+= Force[1][2];
      
      vertexDerivs[v3][0]+= Force[2][0];
      vertexDerivs[v3][1]+= Force[2][1];
      vertexDerivs[v3][2]+= Force[2][2];
    }
  }
}

void VertexFromTRBScenterTriangulation::
initiate(Tissue &T,
	 std::vector< std::vector<double> > &cellData,
	 std::vector< std::vector<double> > &wallData,
	 std::vector< std::vector<double> > &vertexData,
	 std::vector< std::vector<double> > &cellDerivs,
	 std::vector< std::vector<double> > &wallDerivs,
	 std::vector< std::vector<double> > &vertexDerivs)
{
  size_t dimension=3; //Only implemented for 3D models
  assert (dimension==vertexData[0].size());
  size_t numVariable = T.cell(0).numVariable();
  assert (numVariable==cellData[0].size());
  // Create the new variables
  if (variableIndex(1,0) != numVariable) {
    std::cerr << "VertexFromTRBScenterTriangulation::initiate() "
	      << "Wrong index given as start index for additional variables."
	      << std::endl;
    exit(-1);
  }
  size_t numCell = cellData.size();
  assert (numCell==T.numCell());
  std::vector<double> com(dimension);

  for (size_t i=0; i<numCell; ++i) {
    size_t numInternalWall = T.cell(i).numVertex();
    cellData[i].resize(numVariable+dimension+numInternalWall);
    cellDerivs[i].resize(numVariable+dimension+numInternalWall);
    com = T.cell(i).positionFromVertex(vertexData);
    // Set center position to com of the cell
    for (size_t d=0; d<dimension; ++d)
      cellData[i][numVariable+d] = com[d];    
    // Set internal wall lengths to the distance btw com and the vertex
    for (size_t k=0; k<numInternalWall; ++k) {
      Vertex *tmpVertex = T.cell(i).vertex(k); 
      size_t vertexIndex = tmpVertex->index();
      double distance = std::sqrt( (com[0]-vertexData[vertexIndex][0])*(com[0]-vertexData[vertexIndex][0])+
				   (com[1]-vertexData[vertexIndex][1])*(com[1]-vertexData[vertexIndex][1])+
				   (com[2]-vertexData[vertexIndex][2])*(com[2]-vertexData[vertexIndex][2]) );   
      cellData[i][numVariable+dimension+k] = distance;
    }
  }
}

VertexFromTRBScenterTriangulationConcentrationHill::
VertexFromTRBScenterTriangulationConcentrationHill(std::vector<double> &paraValue, 
						   std::vector< std::vector<size_t> > 
						   &indValue ) 
{  
  // Do some checks on the parameters and variable indeces
  if( paraValue.size()!=5 ) {
    std::cerr << "VertexFromTRBScenterTriangulationConcentrationHill::"
	      << "VertexFromTRBScenterTriangulationConcentrationHill() "
	      << "Uses two parameters Young_modulus_min, Young_modulus_max, "
	      << " poisson coefficient, K_hill, and n_hill.\n";
    exit(0);
  }
  if( indValue.size()!=2 || indValue[0].size()!=2 || indValue[1].size()!=1 ) { 
    std::cerr << "VertexFromTRBScenterTriangulationConcentrationHill::"
	      << "VertexFromTRBScenterTriangulationConcentrationHill() "
	      << "Wall length and concentration indices given in first level." << std::endl
	      << "Start of additional Cell variable indices (center(x,y,z) "
	      << "L_1,...,L_n, n=num vertex) is given in second level." 
	      << std::endl;
    exit(0);
  }
  
  // Set the variable values
  setId("VertexFromTRBScenterTriangulationConcentrationHill");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  // Set the parameter identities
  std::vector<std::string> tmp( numParameter() );
  tmp[0] = "Y_mod_min";
  tmp[1] = "Y_mod_max";
  tmp[2] = "P_ratio";
  tmp[3] = "K_hill";
  tmp[4] = "n_hill";
  setParameterId( tmp );
}

void VertexFromTRBScenterTriangulationConcentrationHill::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  
  //Do the update for each cell
  size_t dimension = 3;
  assert (dimension==vertexData[0].size());
  size_t numCells = T.numCell();
  size_t wallLengthIndex = variableIndex(0,0);
  size_t concIndex = variableIndex(0,1);
  size_t comIndex = variableIndex(1,0);
  size_t lengthInternalIndex = comIndex+dimension;
  double Kpow = std::pow(parameter(3),parameter(4));
  
  for (size_t i=0 ; i<numCells ; ++i) {
    size_t numWalls = T.cell(i).numWall(); 
    
    if(  T.cell(i).numVertex()!= numWalls ) {
      std::cerr << "VertexFromTRBScenterTriangulationConcentrationHill::derivs() "
		<< "same number of vertices and walls."
		<< " Not for cells with " << T.cell(i).numWall() << " walls and "
		<< T.cell(i).numVertex() << " vertices!"	
		<< std::endl;
      exit(-1);
    }
    
    double young = parameter(0) + 
      parameter(1)*Kpow/( Kpow+std::pow(cellData[i][concIndex],parameter(4)) );
    double poisson =parameter(1);
    
    // One triangle per 'vertex' in cyclic order
    for (size_t k=0; k<numWalls; ++k) { 
      size_t kPlusOneMod = (k+1)%numWalls;
      //size_t v1 = com;
      size_t v2 = T.cell(i).vertex(k)->index();
      size_t v3 = T.cell(i).vertex(kPlusOneMod)->index();
      //size_t w1 = internal k
      size_t w2 = T.cell(i).wall(k)->index();
      //size_t w3 = internal k+1
      
      // Position matrix holds in rows positions for com, vertex(k), vertex(k+1)
      std::vector< std::vector<double> > position(3,vertexData[v2]);
      for (size_t d=0; d<dimension; ++d)
	position[0][d] = cellData[i][comIndex+d]; // com position
      //position[1] = vertexData[v2]; // given by initiation
      position[2] = vertexData[v3];
      
      // Resting lengths are from com-vertex(k), vertex(k)-vertex(k+1) (wall(k)), com-vertex(k+1)
      std::vector<double> restingLength(numWalls);
      restingLength[0] = cellData[i][lengthInternalIndex + k];
      restingLength[1] = wallData[w2][wallLengthIndex];
      restingLength[2] = cellData[i][lengthInternalIndex + kPlusOneMod];
      
      // Lengths are from com-vertex(k), vertex(k)-vertex(k+1) (wall(k)), com-vertex(k+1)
      std::vector<double> length(numWalls);
      length[0] = std::sqrt( (position[0][0]-position[1][0])*
			     (position[0][0]-position[1][0]) +
			     (position[0][1]-position[1][1])*
			     (position[0][1]-position[1][1]) +
			     (position[0][2]-position[1][2])*
			     (position[0][2]-position[1][2]) );
      length[1] = T.wall(w2).lengthFromVertexPosition(vertexData);
      length[2] = std::sqrt( (position[0][0]-position[2][0])*
			     (position[0][0]-position[2][0]) +
			     (position[0][1]-position[2][1])*
			     (position[0][1]-position[2][1]) +
			     (position[0][2]-position[2][2])*
			     (position[0][2]-position[2][2]) );
      
      // Lame coefficients (can be defined out of loop)
      double lambda=young*poisson/(1-poisson*poisson);
      double mio=young/(1+poisson);
      
      // Area of the element (using Heron's formula)                                      
      double Area=std::sqrt( ( restingLength[0]+restingLength[1]+restingLength[2])*
			     (-restingLength[0]+restingLength[1]+restingLength[2])*
			     ( restingLength[0]-restingLength[1]+restingLength[2])*
			     ( restingLength[0]+restingLength[1]-restingLength[2])  )*0.25;
      
      //Angles of the element ( assuming the order: 0,L0,1,L1,2,L2 )
      std::vector<double> Angle(3);
      Angle[0]=std::asin(2*Area/(restingLength[0]*restingLength[2]));
      // can be ommited by cotan(A)=.25*sqrt(4*b*b*c*c/K-1)
      Angle[1]=std::asin(2*Area/(restingLength[0]*restingLength[1]));        
      Angle[2]=std::asin(2*Area/(restingLength[1]*restingLength[2]));
      
      //Tensile Stiffness
      double tensileStiffness[3];
      double const temp = 1.0/(Area*16);                                      
      double cotan[3] = {1.0/std::tan(Angle[0]),1.0/std::tan(Angle[1]),1.0/std::tan(Angle[2])};    
      tensileStiffness[0]=(2*cotan[2]*cotan[2]*(lambda+mio)+mio)*temp;
      tensileStiffness[1]=(2*cotan[0]*cotan[0]*(lambda+mio)+mio)*temp;
      tensileStiffness[2]=(2*cotan[1]*cotan[1]*(lambda+mio)+mio)*temp;
      
      //Angular Stiffness
      double angularStiffness[3];
      angularStiffness[0]=(2*cotan[1]*cotan[2]*(lambda+mio)-mio)*temp;
      angularStiffness[1]=(2*cotan[0]*cotan[2]*(lambda+mio)-mio)*temp;
      angularStiffness[2]=(2*cotan[0]*cotan[1]*(lambda+mio)-mio)*temp;
      
      //Calculate biquadratic strains  
      std::vector<double> Delta(3);
      Delta[0]=(length[0])*(length[0])-(restingLength[0])*(restingLength[0]);
      Delta[1]=(length[1])*(length[1])-(restingLength[1])*(restingLength[1]);
      Delta[2]=(length[2])*(length[2])-(restingLength[2])*(restingLength[2]);
      //Forces of vertices
      double Force[3][3];                                           
      
      Force[0][0]= (tensileStiffness[0]*Delta[0]+angularStiffness[1]*Delta[1]+angularStiffness[0]*Delta[2])*(position[1][0]-position[0][0])
	+(tensileStiffness[2]*Delta[2]+angularStiffness[2]*Delta[1]+angularStiffness[0]*Delta[0])*(position[2][0]-position[0][0]); 
      Force[0][1]= (tensileStiffness[0]*Delta[0]+angularStiffness[1]*Delta[1]+angularStiffness[0]*Delta[2])*(position[1][1]-position[0][1])
	+(tensileStiffness[2]*Delta[2]+angularStiffness[2]*Delta[1]+angularStiffness[0]*Delta[0])*(position[2][1]-position[0][1]); 
      Force[0][2]= (tensileStiffness[0]*Delta[0]+angularStiffness[1]*Delta[1]+angularStiffness[0]*Delta[2])*(position[1][2]-position[0][2])
	+(tensileStiffness[2]*Delta[2]+angularStiffness[2]*Delta[1]+angularStiffness[0]*Delta[0])*(position[2][2]-position[0][2]); 
      
      Force[1][0]= (tensileStiffness[0]*Delta[0]+angularStiffness[0]*Delta[2]+angularStiffness[1]*Delta[1])*(position[0][0]-position[1][0])
	+(tensileStiffness[1]*Delta[1]+angularStiffness[2]*Delta[2]+angularStiffness[1]*Delta[0])*(position[2][0]-position[1][0]); 
      Force[1][1]= (tensileStiffness[0]*Delta[0]+angularStiffness[0]*Delta[2]+angularStiffness[1]*Delta[1])*(position[0][1]-position[1][1])
	+(tensileStiffness[1]*Delta[1]+angularStiffness[2]*Delta[2]+angularStiffness[1]*Delta[0])*(position[2][1]-position[1][1]); 
      Force[1][2]= (tensileStiffness[0]*Delta[0]+angularStiffness[0]*Delta[2]+angularStiffness[1]*Delta[1])*(position[0][2]-position[1][2])
	+(tensileStiffness[1]*Delta[1]+angularStiffness[2]*Delta[2]+angularStiffness[1]*Delta[0])*(position[2][2]-position[1][2]); 
      
      Force[2][0]= (tensileStiffness[2]*Delta[2]+angularStiffness[0]*Delta[0]+angularStiffness[2]*Delta[1])*(position[0][0]-position[2][0])
	+(tensileStiffness[1]*Delta[1]+angularStiffness[1]*Delta[0]+angularStiffness[2]*Delta[2])*(position[1][0]-position[2][0]); 
      Force[2][1]= (tensileStiffness[2]*Delta[2]+angularStiffness[0]*Delta[0]+angularStiffness[2]*Delta[1])*(position[0][1]-position[2][1])
	+(tensileStiffness[1]*Delta[1]+angularStiffness[1]*Delta[0]+angularStiffness[2]*Delta[2])*(position[1][1]-position[2][1]); 
      Force[2][2]= (tensileStiffness[2]*Delta[2]+angularStiffness[0]*Delta[0]+angularStiffness[2]*Delta[1])*(position[0][2]-position[2][2])
	+(tensileStiffness[1]*Delta[1]+angularStiffness[1]*Delta[0]+angularStiffness[2]*Delta[2])*(position[1][2]-position[2][2]); 
      
      // adding forces to the total vertexDerivs
      
      cellDerivs[i][comIndex]+= Force[0][0];
      cellDerivs[i][comIndex+1]+= Force[0][1];
      cellDerivs[i][comIndex+2]+= Force[0][2];
      
      vertexDerivs[v2][0]+= Force[1][0];
      vertexDerivs[v2][1]+= Force[1][1];
      vertexDerivs[v2][2]+= Force[1][2];
      
      vertexDerivs[v3][0]+= Force[2][0];
      vertexDerivs[v3][1]+= Force[2][1];
      vertexDerivs[v3][2]+= Force[2][2];
    }
  }
}

void VertexFromTRBScenterTriangulationConcentrationHill::
initiate(Tissue &T,
	 std::vector< std::vector<double> > &cellData,
	 std::vector< std::vector<double> > &wallData,
	 std::vector< std::vector<double> > &vertexData,
	 std::vector< std::vector<double> > &cellDerivs,
	 std::vector< std::vector<double> > &wallDerivs,
	 std::vector< std::vector<double> > &vertexDerivs)
{
  size_t dimension=3; //Only implemented for 3D models
  assert (dimension==vertexData[0].size());
  size_t numVariable = T.cell(0).numVariable();
  assert (numVariable==cellData[0].size());
  // Create the new variables
  if (variableIndex(1,0) != numVariable) {
    std::cerr << "VertexFromTRBScenterTriangulationConcentrationHill::initiate() "
	      << "Wrong index given as start index for additional variables."
	      << std::endl;
    exit(-1);
  }
  size_t numCell = cellData.size();
  assert (numCell==T.numCell());
  std::vector<double> com(dimension);
  
  for (size_t i=0; i<numCell; ++i) {
    size_t numInternalWall = T.cell(i).numVertex();
    cellData[i].resize(numVariable+dimension+numInternalWall);
    cellDerivs[i].resize(numVariable+dimension+numInternalWall);
    com = T.cell(i).positionFromVertex(vertexData);
    // Set center position to com of the cell
    for (size_t d=0; d<dimension; ++d)
      cellData[i][numVariable+d] = com[d];    
    // Set internal wall lengths to the distance btw com and the vertex
    for (size_t k=0; k<numInternalWall; ++k) {
      Vertex *tmpVertex = T.cell(i).vertex(k); 
      size_t vertexIndex = tmpVertex->index();
      double distance = std::sqrt( (com[0]-vertexData[vertexIndex][0])*
				   (com[0]-vertexData[vertexIndex][0])+
				   (com[1]-vertexData[vertexIndex][1])*
				   (com[1]-vertexData[vertexIndex][1])+
				   (com[2]-vertexData[vertexIndex][2])*
				   (com[2]-vertexData[vertexIndex][2]) );   
      cellData[i][numVariable+dimension+k] = distance;
    }
  }
}

VertexFromTRBSMT::
VertexFromTRBSMT(std::vector<double> &paraValue, 
	       std::vector< std::vector<size_t> > 
	       &indValue ) 
{  
  // Do some checks on the parameters and variable indeces
  if( paraValue.size()!=4 ) {
    std::cerr << "VertexFromTRBSMT::"
	      << "VertexFromTRBSMT() "
	      << "Uses four parameters young modulus and poisson coefficients in "
	      << "longitudinal and transverse directions." << std::endl;
    exit(0);
  }
  if( indValue.size()!=1 || indValue[0].size()!=2 ) { 
    std::cerr << "VertexFromTRBSMT::"
	      << "VertexFromTRBSMT() "
	      << "Wall length index and cell MT direction start index"
	      << "given at first level." << std::endl;
    exit(0);
  }
  
  // Set the variable values
  setId("VertexFromTRBSMT");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  // Set the parameter identities
  std::vector<std::string> tmp( numParameter() );
  tmp[0] = "Y_mod_L";// Longitudinal components of parameters
  tmp[1] = "P_ratio_L";
  tmp[2] = "Y_mod_T";// Transverse components of parameters
  tmp[3] = "P_ratio_T";

  setParameterId( tmp );
}

void VertexFromTRBSMT::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  
  //Do the update for each cell
  size_t numCells = T.numCell();
  size_t wallLengthIndex = variableIndex(0,0);
  size_t numWalls = 3; // defined only for triangles at the moment
  
  for( size_t cellIndex=0 ; cellIndex<numCells ; ++cellIndex ) {
    if( T.cell(cellIndex).numWall() != numWalls ) {
      std::cerr << "VertexFromTRBSMT::derivs() only defined for triangular cells."
		<< " Not for cells with " << T.cell(cellIndex).numWall() << " walls!"
		<< std::endl;
      exit(-1);
    }
    
    double youngL   = parameter(0);//two more parameters here 
    double poissonL = parameter(1);
    double youngT   = parameter(2);
    double poissonT = parameter(3);
    
    size_t v1 = T.cell(cellIndex).vertex(0)->index();
    size_t v2 = T.cell(cellIndex).vertex(1)->index();
    size_t v3 = T.cell(cellIndex).vertex(2)->index();
    size_t w1 = T.cell(cellIndex).wall(0)->index();
    size_t w2 = T.cell(cellIndex).wall(1)->index();
    size_t w3 = T.cell(cellIndex).wall(2)->index();
    std::vector<double> restingLength(numWalls);
    restingLength[0] = wallData[w1][wallLengthIndex];
    restingLength[1] = wallData[w2][wallLengthIndex];
    restingLength[2] = wallData[w3][wallLengthIndex];

    std::vector< std::vector<double> > position(3,vertexData[v1]);
    position[1] = vertexData[v2];
    position[2] = vertexData[v3];
    //position[0][2] z for vertex 1 (of the cell)
   
    std::vector<double> length(numWalls);
    length[0] = T.wall(w1).lengthFromVertexPosition(vertexData);
    length[1] = T.wall(w2).lengthFromVertexPosition(vertexData);
    length[2] = T.wall(w3).lengthFromVertexPosition(vertexData);
    
    // Lame coefficients (can be defined out of loop)
    double lambdaL=youngL*poissonL/(1-poissonL*poissonL);
    double mioL=youngL/(1+poissonL);
    double lambdaT=youngT*poissonT/(1-poissonT*poissonT);
    double mioT=youngT/(1+poissonT);
    
    //Resting area of the element (using Heron's formula)                                      
    double restingArea=std::sqrt( ( restingLength[0]+restingLength[1]+restingLength[2])*
                                  (-restingLength[0]+restingLength[1]+restingLength[2])*
                                  ( restingLength[0]-restingLength[1]+restingLength[2])*
                                  ( restingLength[0]+restingLength[1]-restingLength[2])  )*0.25;
    
    //Angles of the element ( assuming the order: 0,L0,1,L1,2,L2 )
    std::vector<double> Angle(3);
    Angle[0]=std::asin(2*restingArea/(restingLength[0]*restingLength[2]));
    // can be ommited by cotan(A)=.25*sqrt(4*b*b*c*c/K-1)
    Angle[1]=std::asin(2*restingArea/(restingLength[0]*restingLength[1]));        
    Angle[2]=std::asin(2*restingArea/(restingLength[1]*restingLength[2]));
    
    //Tensile Stiffness
    double tensileStiffness[3];
    double temp = 1.0/(restingArea*16);                                      
    std::vector<double> cotan(3);
    cotan[0] = 1.0/std::tan(Angle[0]);
    cotan[1] = 1.0/std::tan(Angle[1]);
    cotan[2] = 1.0/std::tan(Angle[2]);    
    //the force is calculated based on Transverse coefficients
    //Longitudinal coefficients are considered in deltaF
    tensileStiffness[0]=(2*cotan[2]*cotan[2]*(lambdaT+mioT)+mioT)*temp;
    tensileStiffness[1]=(2*cotan[0]*cotan[0]*(lambdaT+mioT)+mioT)*temp;
    tensileStiffness[2]=(2*cotan[1]*cotan[1]*(lambdaT+mioT)+mioT)*temp;
    
    //Angular Stiffness
    std::vector<double> angularStiffness(3);
    angularStiffness[0]=(2*cotan[1]*cotan[2]*(lambdaT+mioT)-mioT)*temp;                          
    angularStiffness[1]=(2*cotan[0]*cotan[2]*(lambdaT+mioT)-mioT)*temp;
    angularStiffness[2]=(2*cotan[0]*cotan[1]*(lambdaT+mioT)-mioT)*temp;
    
    //Calculate biquadratic strains  
    std::vector<double> Delta(3);
    Delta[0]=(length[0])*(length[0])-(restingLength[0])*(restingLength[0]);
    Delta[1]=(length[1])*(length[1])-(restingLength[1])*(restingLength[1]);
    Delta[2]=(length[2])*(length[2])-(restingLength[2])*(restingLength[2]);

    //Area of the element (using Heron's formula)                                      
    double Area=std::sqrt( ( length[0]+length[1]+length[2])*
                           (-length[0]+length[1]+length[2])*
                           ( length[0]-length[1]+length[2])*
                           ( length[0]+length[1]-length[2])  )*0.25;
    
    //---- Anisotropic Correction-------------------------------
    double deltaLam=lambdaL-lambdaT;
    double deltaMio=mioL-mioT;
    std::vector<double> teta(3);
    
    //double ez[3]={0,0,1};
    std::vector<double> La(3);// Q2-Q1
    La[0]=position[1][0]-position[0][0];
    La[1] = position[1][1]-position[0][1];
    La[2] = position[1][2]-position[0][2];  

    double Lb[3]={position[2][0]-position[0][0],position[2][1]-position[0][1],position[2][2]-position[0][2]};  // Q3-Q1
    double normal[3]={La[1]*Lb[2]-La[2]*Lb[1] , La[2]*Lb[0]-La[0]*Lb[2] , La[0]*Lb[1]-La[1]*Lb[0]};            // cross(Q2-Q1,Q3-Q1)
    double alpha=-std::acos(normal[2]/std::sqrt(normal[0]*normal[0]+normal[1]*normal[1]+normal[2]*normal[2])); //-acos((dot(k,normal))/norm(normal))
    double u[3]={-normal[1]/std::sqrt(normal[0]*normal[0]+normal[1]*normal[1]),normal[0]/std::sqrt(normal[0]*normal[0]+normal[1]*normal[1]),0};                                                                                                                           // cross(k,normal)/norm(cross(k,normal))
    double cc=std::cos(alpha);
    double ss=std::sin(alpha);
    
    double Rot[3][3]={ {cc+u[0]*u[0]*(1-cc)      ,  u[0]*u[1]*(1-cc)-u[2]*ss ,  u[0]*u[2]*(1-cc)+u[1]*ss}, 
		       {u[0]*u[1]*(1-cc)+u[2]*ss ,  cc+u[1]*u[1]*(1-cc)      ,  u[1]*u[2]*(1-cc)-u[0]*ss},       
		       {u[0]*u[2]*(1-cc)-u[1]*ss ,  u[1]*u[2]*(1-cc)+u[0]*ss ,  cc+u[2]*u[2]*(1-cc)     }  };

    double Q1[3]={Rot[0][0]*position[0][0]+Rot[0][1]*position[0][1]+Rot[0][2]*position[0][2],
		  Rot[1][0]*position[0][0]+Rot[1][1]*position[0][1]+Rot[1][2]*position[0][2],
		  Rot[2][0]*position[0][0]+Rot[2][1]*position[0][1]+Rot[2][2]*position[0][2]};                                    //Rot*Q1

    double Q2[3]={Rot[0][0]*position[1][0]+Rot[0][1]*position[1][1]+Rot[0][2]*position[1][2],
		  Rot[1][0]*position[1][0]+Rot[1][1]*position[1][1]+Rot[1][2]*position[1][2],
		  Rot[2][0]*position[1][0]+Rot[2][1]*position[1][1]+Rot[2][2]*position[1][2]};                                    //Rot*Q2
    
    double Q3[3]={Rot[0][0]*position[2][0]+Rot[0][1]*position[2][1]+Rot[0][2]*position[2][2],
		  Rot[1][0]*position[2][0]+Rot[1][1]*position[2][1]+Rot[1][2]*position[2][2],
		  Rot[2][0]*position[2][0]+Rot[2][1]*position[2][1]+Rot[2][2]*position[2][2]};                                    //Rot*Q3

    double CMcurr[3]={(Q1[0]+Q2[0]+Q3[0])/3,(Q1[1]+Q2[1]+Q3[1])/3,(Q1[2]+Q2[2]+Q3[2])/3};


    double RotAnisocurr[3]={ Rot[0][0]*cellData[cellIndex][variableIndex(0,2)]+
			     Rot[0][1]*cellData[cellIndex][variableIndex(0,2)+1]+
			     Rot[0][2]*cellData[cellIndex][variableIndex(0,2)+2],
			     Rot[1][0]*cellData[cellIndex][variableIndex(0,2)]+
			     Rot[1][1]*cellData[cellIndex][variableIndex(0,2)+1]+
			     Rot[1][2]*cellData[cellIndex][variableIndex(0,2)+2],
			     Rot[2][0]*cellData[cellIndex][variableIndex(0,2)]+
			     Rot[2][1]*cellData[cellIndex][variableIndex(0,2)+1]+
			     Rot[2][2]*cellData[cellIndex][variableIndex(0,2)+2] };//Rot*Anisocurr;
   
    double Acurr[3]={RotAnisocurr[0]+CMcurr[0],RotAnisocurr[1]+CMcurr[1],RotAnisocurr[2]+CMcurr[2]};//[temp(1) temp(2)]'+CMcurr'

    //double Bari[3][3]={ {Q1[0], Q2[0], Q3[0]} , {Q1[1], Q2[1], Q3[1]} , {1, 1, 1} };
   
 
    temp=1/(Q1[0]*Q2[1]-Q1[1]*Q2[0]+Q1[1]*Q3[0]-Q1[0]*Q3[1]+Q2[0]*Q3[1]-Q2[1]*Q3[0]); //1/(determinant of Bari)
    double invBari[3][3]={ {temp*(Q2[1]-Q3[1]), temp*(Q3[0]-Q2[0]), temp*(Q2[0]*Q3[1]-Q2[1]*Q3[0])}, // inverse of Bari
			   {temp*(Q3[1]-Q1[1]), temp*(Q1[0]-Q3[0]), temp*(Q1[1]*Q3[0]-Q1[0]*Q3[1])},
			   {temp*(Q1[1]-Q2[1]), temp*(Q2[0]-Q1[0]), temp*(Q1[0]*Q2[1]-Q1[1]*Q2[0])} };
    
    double Abari[3]={invBari[0][0]*Acurr[0]+invBari[0][1]*Acurr[1]+invBari[0][2],
		     invBari[1][0]*Acurr[0]+invBari[1][1]*Acurr[1]+invBari[1][2],
		     invBari[2][0]*Acurr[0]+invBari[2][1]*Acurr[1]+invBari[2][2]  };// invBari*[Acurr(1) Acurr(2) 1]'
    
    // A=norm(cross(Q1-Q2,Q1-Q3))
    // AA=norm(cross(Q11-Q21,Q11-Q31))
    
    
    // providing P1 , P2 and P3 assuming : 1, L1, 2, L2, 3, L3, counterclockwise
    
    double P1[2]={0,0};
    double P2[2]={0,restingLength[0]};
    double P3[2]={restingLength[2]*std::sin(Angle[0]),restingLength[2]*std::cos(Angle[0])};
    
    double Arest[2]={P1[0]*Abari[0]+P2[0]*Abari[1]+P3[0]*Abari[2] ,  P1[1]*Abari[0]+ P2[1]*Abari[1]+ P3[1]*Abari[2]};
    //[P1(0) P2(0) P3(0) ; P1(1) P2(1) P3(1) ; 1 1 1 ]*Abari;
    double CMrest[2]={(P1[1]+P2[1]+P3[1])/3,(P1[2]+P2[2]+P3[2])/3};
    double Anisorest[2]={Arest[0]-CMrest[0],Arest[1]-CMrest[1]};
    
    temp=1/restingArea;
    double D1[2]={temp*(P2[1]-P3[1]), -temp*(P2[0]-P3[0])};
    double D2[2]={temp*(P3[1]-P1[1]), -temp*(P3[0]-P1[0])};
    double D3[2]={temp*(P1[1]-P2[1]), -temp*(P1[0]-P2[0])};
    
    //teta = acos((dot(Anisorest,Dk))/(norm(Anisorest)*norm(Dk))),
    teta[0] = std::acos((Anisorest[0]*D1[0]+Anisorest[1]*D1[1])/
			std::sqrt((Anisorest[0]*Anisorest[0]+Anisorest[1]*Anisorest[1])*
				  (D1[0]*D1[0]+D1[1]*D1[1])));
    teta[1] = std::acos((Anisorest[0]*D2[0]+Anisorest[1]*D2[1])/
			std::sqrt((Anisorest[0]*Anisorest[0]+Anisorest[1]*Anisorest[1])*
				  (D2[0]*D2[0]+D2[1]*D2[1])));
    teta[2] = std::acos((Anisorest[0]*D3[0]+Anisorest[1]*D3[1])/
			std::sqrt((Anisorest[0]*Anisorest[0]+Anisorest[1]*Anisorest[1])*
				  (D3[0]*D3[0]+D3[1]*D3[1])));
    
    //------------------------------------------------------------------------
    
    //---- Anisotropic Correction-------------------------------
    
    double  Rcirc2=(0.25*length[0]*length[1]*length[2]/Area)*
      (0.25*length[0]*length[1]*length[2]/Area);  // square of radius of circumstancing circle
    
    double derIprim1[3][3];         // Invariants and their derivatives
    double derIprim4[3][3];
    double derIprim5[3][3];
    
    double DiDm;                    // inner products between shape vectors
    double DnDr;
    double DsDp;
    
    double QiQj;                    // inner products between position vectors of vertices
    double QrQs;
    
    double aDi;                     // inner products between shape vectors and anisotropy vector(direction)
    double aDj;
    double aDm;
    double aDp;
    double aDr;
    double aDs;
    double aDn;
    
    int k;
    for ( int m=0 ; m<3 ; ++m )
      {
        for ( int coor=0 ; coor<3 ; ++coor ) derIprim1[m][coor]=0;
	for ( int i=0 ; i<3 ; ++i )
	  {            
	    if ((i==0 && m==1)||(i==1 && m==0)) k=2;
	    if ((i==0 && m==2)||(i==2 && m==0)) k=1;
	    if ((i==1 && m==2)||(i==2 && m==1)) k=0; 
	    if (i!=m) DiDm=-0.5*cotan[k]/restingArea;
	    if (i==m) DiDm=0.25*restingLength[i]*restingLength[i] / (restingArea*restingArea);
	    for ( int coor=0 ; coor<3 ; ++coor ) 
	      derIprim1[m][coor]=derIprim1[m][coor]+2*DiDm*position[m][coor];
	  }
      }
    
    double Iprim4=0;
    for ( int i=0 ; i<3 ; ++i )
      { 
	for ( int j=0 ; j<3 ; ++j )
	  {
	    if ((i==0 && j==1)||(i==1 && j==0)) k=2; 
	    if ((i==0 && j==2)||(i==2 && j==0)) k=1; 
	    if ((i==1 && j==2)||(i==2 && j==1)) k=0;  
	    if (i!=j) QiQj=Rcirc2-(length[k]*length[k])*0.5;
	    if (i==j) QiQj=Rcirc2;
	    
	    aDi=0.5*cos(teta[i])*restingLength[i]/restingArea;
	    aDj=0.5*cos(teta[j])*restingLength[j]/restingArea;
	    
	    Iprim4=Iprim4+ QiQj*aDi*aDj;
	  }
      }
    
    for ( int p=0 ; p<3 ; ++p )
      {
      for ( int coor=0 ; coor<3 ; ++coor ) derIprim4[p][coor]=0;
      for ( int m=0 ; m<3 ; ++m )
        {
	  aDm=0.5*cos(teta[m])*restingLength[m]/restingArea;
	  for ( int coor=0 ; coor<3 ; ++coor ) derIprim4[p][coor]=derIprim4[p][coor]+aDm*position[m][coor];
        }
      aDp=0.5*cos(teta[p])*restingLength[p]/restingArea;
      for ( int coor=0 ; coor<3 ; ++coor ) derIprim4[p][coor]=2*aDp*derIprim4[p][coor];
      }
    
    for ( int p=0 ; p<3 ; ++p )                                                                             
      { 
	for ( int coor=0 ; coor<3 ; ++coor ) derIprim5[p][coor]=0;
	for ( int n=0 ; n<3 ; ++n )
	  { 
	    for ( int r=0 ; r<3 ; ++r )
	      { for ( int s=0 ; s<3 ; ++s )
                  {     
		    if ((r==0 && s==1)||(r==1 && s==0)) k=2;
		    if ((r==0 && s==2)||(r==2 && s==0)) k=1;
		    if ((r==1 && s==2)||(r==2 && s==1)) k=0; 
		    if ( s!=r ) QrQs=Rcirc2-(length[k]*length[k])*0.5;
		    if ( s==r ) QrQs=Rcirc2;
		    
		    if ((n==0 && r==1)||(n==1 && r==0)) k=2; 
		    if ((n==0 && r==2)||(n==2 && r==0)) k=1; 
		    if ((n==1 && r==2)||(n==2 && r==1)) k=0;    
		    if ( n!=r )  DnDr=-0.5*cotan[k]/restingArea;
		    if ( n==r )  DnDr=0.25*restingLength[n]*restingLength[n] / (restingArea*restingArea);
		    
		    if ((s==0 && p==1)||(s==1 && p==0)) k=2; 
		    if ((s==0 && p==2)||(s==2 && p==0)) k=1; 
		    if ((s==1 && p==2)||(s==2 && p==1)) k=0;   
		    if ( s!=p ) DsDp=-0.5*cotan[k]/restingArea;
		    if ( s==p ) DsDp=0.25*restingLength[s]*restingLength[s] / (restingArea*restingArea);
		    
		    aDs=0.5*cos(teta[s])*restingLength[s]/restingArea;
		    aDp=0.5*cos(teta[p])*restingLength[p]/restingArea;
		    aDr=0.5*cos(teta[r])*restingLength[r]/restingArea;
		    aDn=0.5*cos(teta[n])*restingLength[n]/restingArea;
		    
		    for ( int coor=0 ; coor<3 ; ++coor ) 
		      derIprim5[p][coor] = derIprim5[p][coor] +
			2*(DnDr*aDs*aDp+DsDp*aDr*aDn)*QrQs*position[n][coor];
		  }
	      }
	  }
      }		       
    
    double derI1[3][3];             // Invariants and their derivatives
    double derI4[3][3];
    double derI5[3][3];
    
    
    double I1=( Delta[0]*cotan[0]+ Delta[1]*cotan[1]+Delta[2]*cotan[2])/(2*restingArea);
    double I4=0.5*Iprim4-0.5;
    
    for ( int i=0 ; i<3 ; ++i ) 
      for ( int j=0 ; j<3 ; ++j ) {
	derI1[i][j]=0.5*derIprim1[i][j];
	derI4[i][j]=0.5*derIprim4[i][j];
	derI5[i][j]=0.25*derIprim5[i][j]-derI4[i][j];
      }   
    double deltaF[3][3]; 
    
    for ( int i=0 ; i<3 ; ++i ) 
      for ( int j=0 ; j<3 ; ++j )
	deltaF[i][j]=(-deltaLam*(I4*derI1[i][j]+I1*derI4[i][j])-
		      deltaMio*derI5[i][j]+(deltaMio+deltaLam)*I4*derI4[i][j])*restingArea;
      
    //Forces of vertices   
    double Force[3][3];                                           
    
    Force[0][0]= (tensileStiffness[0]*Delta[0]+angularStiffness[1]*Delta[1]+angularStiffness[0]*Delta[2])*(position[1][0]-position[0][0])
      +(tensileStiffness[2]*Delta[2]+angularStiffness[2]*Delta[1]+angularStiffness[0]*Delta[0])*(position[2][0]-position[0][0])
      + deltaF[0][0]; 
    Force[0][1]= (tensileStiffness[0]*Delta[0]+angularStiffness[1]*Delta[1]+angularStiffness[0]*Delta[2])*(position[1][1]-position[0][1])
      +(tensileStiffness[2]*Delta[2]+angularStiffness[2]*Delta[1]+angularStiffness[0]*Delta[0])*(position[2][1]-position[0][1])
      + deltaF[0][1];  
    Force[0][2]= (tensileStiffness[0]*Delta[0]+angularStiffness[1]*Delta[1]+angularStiffness[0]*Delta[2])*(position[1][2]-position[0][2])
      +(tensileStiffness[2]*Delta[2]+angularStiffness[2]*Delta[1]+angularStiffness[0]*Delta[0])*(position[2][2]-position[0][2])
      + deltaF[0][2]; 
    Force[1][0]= (tensileStiffness[0]*Delta[0]+angularStiffness[0]*Delta[2]+angularStiffness[1]*Delta[1])*(position[0][0]-position[1][0])
      +(tensileStiffness[1]*Delta[1]+angularStiffness[2]*Delta[2]+angularStiffness[1]*Delta[0])*(position[2][0]-position[1][0])
      + deltaF[1][0];  
    Force[1][1]= (tensileStiffness[0]*Delta[0]+angularStiffness[0]*Delta[2]+angularStiffness[1]*Delta[1])*(position[0][1]-position[1][1])
      +(tensileStiffness[1]*Delta[1]+angularStiffness[2]*Delta[2]+angularStiffness[1]*Delta[0])*(position[2][1]-position[1][1])
      + deltaF[1][1];  
    Force[1][2]= (tensileStiffness[0]*Delta[0]+angularStiffness[0]*Delta[2]+angularStiffness[1]*Delta[1])*(position[0][2]-position[1][2])
      +(tensileStiffness[1]*Delta[1]+angularStiffness[2]*Delta[2]+angularStiffness[1]*Delta[0])*(position[2][2]-position[1][2])
      + deltaF[1][2];  
    
    Force[2][0]= (tensileStiffness[2]*Delta[2]+angularStiffness[0]*Delta[0]+angularStiffness[2]*Delta[1])*(position[0][0]-position[2][0])
      +(tensileStiffness[1]*Delta[1]+angularStiffness[1]*Delta[0]+angularStiffness[2]*Delta[2])*(position[1][0]-position[2][0])
      + deltaF[2][0];  
    Force[2][1]= (tensileStiffness[2]*Delta[2]+angularStiffness[0]*Delta[0]+angularStiffness[2]*Delta[1])*(position[0][1]-position[2][1])
      +(tensileStiffness[1]*Delta[1]+angularStiffness[1]*Delta[0]+angularStiffness[2]*Delta[2])*(position[1][1]-position[2][1])
      + deltaF[2][1];  
    Force[2][2]= (tensileStiffness[2]*Delta[2]+angularStiffness[0]*Delta[0]+angularStiffness[2]*Delta[1])*(position[0][2]-position[2][2])
      +(tensileStiffness[1]*Delta[1]+angularStiffness[1]*Delta[0]+angularStiffness[2]*Delta[2])*(position[1][2]-position[2][2])
      + deltaF[2][2];  
    
    // adding TRBSMT forces to the total vertexDerivs
    
    vertexDerivs[v1][0]+= Force[0][0];
    vertexDerivs[v1][1]+= Force[0][1];
    vertexDerivs[v1][2]+= Force[0][2];
    
    vertexDerivs[v2][0]+= Force[1][0];
    vertexDerivs[v2][1]+= Force[1][1];
    vertexDerivs[v2][2]+= Force[1][2];
    
    vertexDerivs[v3][0]+= Force[2][0];
    vertexDerivs[v3][1]+= Force[2][1];
    vertexDerivs[v3][2]+= Force[2][2];
  }
}
