//
// Filename     : mechanicalSpring.cc
// Description  : Classes describing updates due to mechanical spring interactions
// Author(s)    : Henrik Jonsson (henrik@thep.lu.se)
// Created      : September 2007
// Revision     : $Id:$
//
#include <utility>
#include <vector>
#include "baseReaction.h"
#include "mechanicalSpring.h"
#include "tissue.h"

//!Constructor for the SpringAsymmetric class
VertexFromWallSpringAsymmetric::
VertexFromWallSpringAsymmetric(std::vector<double> &paraValue, 
															 std::vector< std::vector<size_t> > 
															 &indValue ) 
{  
  // Do some checks on the parameters and variable indeces
  if( paraValue.size()!=2 ) {
    std::cerr << "VertexFromWallSpringAsymmetric::"
							<< "VertexFromWallSpringAsymmetric() "
							<< "Uses two parameters K_force frac_adhesion.\n";
    exit(0);
  }
  if( indValue.size() < 1 || indValue.size() > 2 
			|| indValue[0].size() != 1 
			|| (indValue.size()==2 && indValue[1].size() != 1) ) {
    std::cerr << "VertexFromWallSpringAsymmetric::"
							<< "VertexFromWallSpringAsymmetric() "
							<< "Wall length index given in first level,"
							<< " and optionally wall variable save index in second.\n";
    exit(0);
  }

  // Set the variable values
  setId("VertexFromWallSpringAsymmetric");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  // Set the parameter identities
  std::vector<std::string> tmp( numParameter() );
  tmp[0] = "K_force";
  tmp[1] = "frac_adh";
  setParameterId( tmp );
}

//! Derivative contribution for asymmetric wall springs on vertices
/*! 
*/
void VertexFromWallSpringAsymmetric::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  
  //Do the update for each wall
  size_t numWalls = T.numWall();
  size_t wallLengthIndex = variableIndex(0,0);
  
  for( size_t i=0 ; i<numWalls ; ++i ) {
    size_t v1 = T.wall(i).vertex1()->index();
    size_t v2 = T.wall(i).vertex2()->index();
    size_t dimension = vertexData[v1].size();
    assert( vertexData[v2].size()==dimension );
    //Calculate shared factors
    double distance=0.0;
    for( size_t d=0 ; d<dimension ; d++ )
      distance += (vertexData[v1][d]-vertexData[v2][d])*
	(vertexData[v1][d]-vertexData[v2][d]);
    distance = std::sqrt(distance);
    double wallLength=wallData[i][wallLengthIndex];
    double coeff = parameter(0)*((1.0/wallLength)-(1.0/distance));
    if( distance <= 0.0 && wallLength <=0.0 ) {
      //std::cerr << i << " - " << wallLength << " " << distance << std::endl;
      coeff = 0.0;
    }
    if( distance>wallLength )
      coeff *=parameter(1);

		//Save force in wall variable if appropriate
		if( numVariableIndexLevel()>1 )
			wallData[i][variableIndex(1,0)] = coeff*distance;
    
    //Update both vertices for each dimension
    for(size_t d=0 ; d<dimension ; d++ ) {
      double div = (vertexData[v1][d]-vertexData[v2][d])*coeff;
      vertexDerivs[v1][d] -= div;
      vertexDerivs[v2][d] += div;
    }
  }
}

VertexFromWallSpringMT::
VertexFromWallSpringMT(std::vector<double> &paraValue, 
													std::vector< std::vector<size_t> > 
													&indValue ) 
{  
  // Do some checks on the parameters and variable indeces
  if( paraValue.size()!=3 ) {
    std::cerr << "VertexFromWallSpringMT::"
							<< "VertexFromWallSpringMT() "
							<< "Uses three parameters K_force^min K_force^max "
							<< "frac_adhesion.\n";
    exit(0);
  }
  if( indValue.size() < 1 || indValue.size() > 2 
			|| indValue[0].size() != 2 
			|| (indValue.size()==2 && indValue[1].size() != 1) ) {
    std::cerr << "VertexFromWallSpringMT::"
							<< "VertexFromWallSpringMT() "
							<< "Wall length index and cell MT direction start index"
							<< "given at first level,"
							<< " and optionally wall variable save index in second.\n";
    exit(0);
  }
  //Set the variable values
  setId("VertexFromWallSpringMT");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  //Set the parameter identities
  std::vector<std::string> tmp( numParameter() );
  tmp[0] = "K_force^min";
  tmp[1] = "K_force^max";
  tmp[2] = "frac_adh";
  setParameterId( tmp );	
}

void VertexFromWallSpringMT::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) 
{  
  //Do the update for each wall
  size_t numWalls = T.numWall();
  size_t wallLengthIndex = variableIndex(0,0);
	size_t directionIndex = variableIndex(0,1);
	size_t dimension = vertexData[0].size();
  
  for( size_t i=0 ; i<numWalls ; ++i ) {
    size_t v1 = T.wall(i).vertex1()->index();
    size_t v2 = T.wall(i).vertex2()->index();
    assert( vertexData[v2].size()==dimension );
    //Calculate shared factors
    double distance=0.0,c1Norm=0.0,c2Norm=0.0;
		std::vector<double> n_w(dimension),n_c1(dimension),n_c2(dimension);
    for( size_t d=0 ; d<dimension ; d++ ) {
			n_w[d] = vertexData[v2][d]-vertexData[v1][d];
			distance += n_w[d]*n_w[d];
			if( T.wall(i).cell1() != T.background() && 
					cellData[T.wall(i).cell1()->index()][directionIndex+dimension]>0.5 ) {
				n_c1[d] = cellData[T.wall(i).cell1()->index()][directionIndex+d];
				c1Norm += n_c1[d]*n_c1[d];
			}
			if( T.wall(i).cell2() != T.background() &&
					cellData[T.wall(i).cell2()->index()][directionIndex+dimension]>0.5 ) {
				n_c2[d] = cellData[T.wall(i).cell2()->index()][directionIndex+d];
				c2Norm += n_c2[d]*n_c2[d];			
			}
		}
    distance = std::sqrt( distance );
		c1Norm = std::sqrt( c1Norm );
		c2Norm = std::sqrt( c2Norm );
		double c1Fac=0.0,c2Fac=0.0;
		if( T.wall(i).cell1() != T.background() &&
				cellData[T.wall(i).cell1()->index()][directionIndex+dimension]>0.5 ) {
			for( size_t d=0 ; d<dimension ; d++ )		
				c1Fac += n_c1[d]*n_w[d]/(c1Norm*distance);
		}
		else
			c1Fac = 0.5;//1.0;
		if( T.wall(i).cell2() != T.background() &&
				cellData[T.wall(i).cell2()->index()][directionIndex+dimension]>0.5 ) {
			for( size_t d=0 ; d<dimension ; d++ )		
				c2Fac += n_c2[d]*n_w[d]/(c2Norm*distance);
		}
		else
			c2Fac = 0.5;//1.0;
		
    double wallLength=wallData[i][wallLengthIndex];
    double coeff = (parameter(0)+parameter(1)*(2.0-c1Fac-c2Fac))*
			((1.0/wallLength)-(1.0/distance));
    if( distance <= 0.0 && wallLength <=0.0 ) {
      //std::cerr << i << " - " << wallLength << " " << distance << std::endl;
      coeff = 0.0;
    }
    if( distance>wallLength )
      coeff *=parameter(2);
    
		//Save force in wall variable if appropriate
		if( numVariableIndexLevel()>1 )
			wallData[i][variableIndex(1,0)] = coeff*distance;
		
    //Update both vertices for each dimension
    for(size_t d=0 ; d<dimension ; d++ ) {
      double div = (vertexData[v1][d]-vertexData[v2][d])*coeff;
      vertexDerivs[v1][d] -= div;
      vertexDerivs[v2][d] += div;
    }
  }
}

void VertexFromWallSpringMT::
initiate(Tissue &T,
				 std::vector< std::vector<double> > &cellData,
				 std::vector< std::vector<double> > &wallData,
				 std::vector< std::vector<double> > &vertexData )
{
}

VertexFromWallSpringMTHistory::
VertexFromWallSpringMTHistory(std::vector<double> &paraValue, 
															std::vector< std::vector<size_t> > 
															&indValue ) 
{  
  // Do some checks on the parameters and variable indeces
  if( paraValue.size()!=4 ) {
    std::cerr << "VertexFromWallSpringMTHistory::"
							<< "VertexFromWallSpringMTHistory() "
							<< "Uses four parameters K_force^min K_force^max "
							<< "frac_adhesion and k_rate.\n";
    exit(0);
  }
  if( indValue.size() < 2 || indValue.size()>3 || indValue[0].size() != 2 
			|| indValue[1].size() != 1 ||
			(indValue.size()==3 && indValue[2].size() != 1) ) {
		std::cerr << "VertexFromWallSpringMTHistory::"
							<< "VertexFromWallSpringMTHistory() "
							<< "Wall length index and cell MT direction start index"
							<< "given at first level,"
							<< " wall spring constant index at second, and optinally "
							<< " force saving index at level three." << std::endl;
    exit(0);
  }
  //Set the variable values
  setId("VertexFromWallSpringMTHistory");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  //Set the parameter identities
  std::vector<std::string> tmp( numParameter() );
  tmp[0] = "K_force^min";
  tmp[1] = "K_force^max";
  tmp[2] = "frac_adh";
	tmp[3] = "k_rate";
  setParameterId( tmp );	
}

void VertexFromWallSpringMTHistory::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) 
{  
  //Do the update for each wall
  size_t numWalls = T.numWall();
  size_t wallLengthIndex = variableIndex(0,0);
	size_t directionIndex = variableIndex(0,1);
	size_t dimension = vertexData[0].size();
  
  for( size_t i=0 ; i<numWalls ; ++i ) {
    size_t v1 = T.wall(i).vertex1()->index();
    size_t v2 = T.wall(i).vertex2()->index();
    assert( vertexData[v2].size()==dimension );
    //Calculate shared factors
    double distance=0.0,c1Norm=0.0,c2Norm=0.0;
		std::vector<double> n_w(dimension),n_c1(dimension),n_c2(dimension);
    for( size_t d=0 ; d<dimension ; d++ ) {
			n_w[d] = vertexData[v2][d]-vertexData[v1][d];
			distance += n_w[d]*n_w[d];
			if( T.wall(i).cell1() != T.background() && 
					cellData[T.wall(i).cell1()->index()][directionIndex+dimension]>0.5 ) {
				n_c1[d] = cellData[T.wall(i).cell1()->index()][directionIndex+d];
				c1Norm += n_c1[d]*n_c1[d];
			}
			if( T.wall(i).cell2() != T.background() &&
					cellData[T.wall(i).cell2()->index()][directionIndex+dimension]>0.5 ) {
				n_c2[d] = cellData[T.wall(i).cell2()->index()][directionIndex+d];
				c2Norm += n_c2[d]*n_c2[d];			
			}
		}
    distance = std::sqrt( distance );
		c1Norm = std::sqrt( c1Norm );
		c2Norm = std::sqrt( c2Norm );
		double c1Fac=0.0,c2Fac=0.0;
		if( T.wall(i).cell1() != T.background() &&
				cellData[T.wall(i).cell1()->index()][directionIndex+dimension]>0.5 ) {
			for( size_t d=0 ; d<dimension ; d++ )		
				c1Fac += n_c1[d]*n_w[d]/(c1Norm*distance);
		}
		else
			c1Fac = 0.5;//1.0;
		if( T.wall(i).cell2() != T.background() &&
				cellData[T.wall(i).cell2()->index()][directionIndex+dimension]>0.5 ) {
			for( size_t d=0 ; d<dimension ; d++ )		
				c2Fac += n_c2[d]*n_w[d]/(c2Norm*distance);
		}
		else
			c2Fac = 0.5;//1.0;
		
    double wallLength=wallData[i][wallLengthIndex];
		double springConstant = (parameter(0)+parameter(1)*(2.0-c1Fac-c2Fac));
    double coeff = wallData[i][variableIndex(1,0)]*
			((1.0/wallLength)-(1.0/distance));
    if( distance <= 0.0 && wallLength <=0.0 ) {
      //std::cerr << i << " - " << wallLength << " " << distance << std::endl;
      coeff = 0.0;
    }
    if( distance>wallLength )
      coeff *=parameter(2);
    
		//Save force in wall variable if appropriate
		if( numVariableIndexLevel()>2 )
			wallData[i][variableIndex(2,0)] = coeff*distance;
		
    // Update both vertices for each dimension
    for(size_t d=0 ; d<dimension ; d++ ) {
      double div = (vertexData[v1][d]-vertexData[v2][d])*coeff;
      vertexDerivs[v1][d] -= div;
      vertexDerivs[v2][d] += div;
    }
		// Update the spring constant
		wallDerivs[i][variableIndex(1,0)] += parameter(3)*
			(springConstant-wallData[i][variableIndex(1,0)]); 
  }
}

void VertexFromWallSpringMTHistory::
initiate(Tissue &T,
				 std::vector< std::vector<double> > &cellData,
				 std::vector< std::vector<double> > &wallData,
				 std::vector< std::vector<double> > &vertexData)
{
  //Do the initiation for each wall
  size_t numWalls = T.numWall();
	size_t directionIndex = variableIndex(0,1);
	size_t dimension = T.vertex(0).numPosition();
  
  for( size_t i=0 ; i<numWalls ; ++i ) {
    Vertex* v1 = T.wall(i).vertex1();
    Vertex* v2 = T.wall(i).vertex2();
		
    //Calculate shared factors
    double distance=0.0,c1Norm=0.0,c2Norm=0.0;
		std::vector<double> n_w(dimension),n_c1(dimension),n_c2(dimension);
    for( size_t d=0 ; d<dimension ; d++ ) {
			n_w[d] = v2->position(d)-v1->position(d);
			distance += n_w[d]*n_w[d];
			if( T.wall(i).cell1() != T.background() && 
					T.wall(i).cell1()->variable(directionIndex+dimension)>0.5 ) {
				n_c1[d] = T.wall(i).cell1()->variable(directionIndex+d);
				c1Norm += n_c1[d]*n_c1[d];
			}
			if( T.wall(i).cell2() != T.background() &&
					T.wall(i).cell2()->variable(directionIndex+dimension)>0.5 ) {
				n_c2[d] = T.wall(i).cell2()->variable(directionIndex+d);
				c2Norm += n_c2[d]*n_c2[d];			
			}
		}
    distance = std::sqrt( distance );
		c1Norm = std::sqrt( c1Norm );
		c2Norm = std::sqrt( c2Norm );
		double c1Fac=0.0,c2Fac=0.0;
		if( T.wall(i).cell1() != T.background() &&
				T.wall(i).cell1()->variable(directionIndex+dimension)>0.5 ) {
			for( size_t d=0 ; d<dimension ; d++ )		
				c1Fac += n_c1[d]*n_w[d]/(c1Norm*distance);
		}
		else
			c1Fac = 0.5;//1.0;
		if( T.wall(i).cell2() != T.background() &&
				T.wall(i).cell2()->variable(directionIndex+dimension)>0.5 ) {
			for( size_t d=0 ; d<dimension ; d++ )		
				c2Fac += n_c2[d]*n_w[d]/(c2Norm*distance);
		}
		else
			c2Fac = 0.5;//1.0;
		
		wallData[i][variableIndex(1,0)] = parameter(0)+parameter(1) *
			(2.0-c1Fac-c2Fac);
	}
}

VertexFromEpidermalWallSpringAsymmetric::
VertexFromEpidermalWallSpringAsymmetric(std::vector<double> &paraValue, 
					std::vector< std::vector<size_t> > 
					&indValue ) {
  
  //Do some checks on the parameters and variable indeces
  //////////////////////////////////////////////////////////////////////
  if( paraValue.size()!=2 ) {
    std::cerr << "VertexFromEpidermalWallSpringAsymmetric::"
	      << "VertexFromEpidermalWallSpringAsymmetric() "
	      << "Uses two parameters K_force frac_adhesion.\n";
    exit(0);
  }

  if( indValue.size() < 1 || indValue.size() > 2 
			|| indValue[0].size() != 1 
			|| (indValue.size()==2 && indValue[1].size() != 1) ) {
    std::cerr << "VertexFromEpidermalWallSpringAsymmetric::"
							<< "VertexFromEpidermalWallSpringAsymmetric() "
							<< "Wall length index given in first level,"
							<< " and optionally wall variable save index in second.\n";
    exit(0);
  }
  //Set the variable values
  //////////////////////////////////////////////////////////////////////
  setId("VertexFromEpidermalWallSpringAsymmetric");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  //Set the parameter identities
  //////////////////////////////////////////////////////////////////////
  std::vector<std::string> tmp( numParameter() );
  tmp[0] = "K_force";
  tmp[1] = "frac_adh";
  setParameterId( tmp );
}

//! Derivative contribution for asymmetric wall springs on vertices
/*! 
*/
void VertexFromEpidermalWallSpringAsymmetric::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  
  //Do the update for each wall
  size_t numWalls = T.numWall();
  size_t wallLengthIndex = variableIndex(0,0);
  
  for( size_t i=0 ; i<numWalls ; ++i ) {
    if( !( T.wall(i).cell1() != T.background() &&
					 T.wall(i).cell2() != T.background() ) ) {
      size_t v1 = T.wall(i).vertex1()->index();
      size_t v2 = T.wall(i).vertex2()->index();
      size_t dimension = vertexData[v1].size();
      assert( vertexData[v2].size()==dimension );
      //Calculate shared factors
      double distance=0.0;
      for( size_t d=0 ; d<dimension ; d++ )
				distance += (vertexData[v1][d]-vertexData[v2][d])*
					(vertexData[v1][d]-vertexData[v2][d]);
      distance = std::sqrt(distance);
      double wallLength=wallData[i][wallLengthIndex];
      double coeff = parameter(0)*((1.0/wallLength)-(1.0/distance));
      if( distance <= 0.0 && wallLength <=0.0 ) {
				//std::cerr << i << " - " << wallLength << " " << distance << std::endl;
				coeff = 0.0;
      }
      if( distance>wallLength )
				coeff *=parameter(1);

			//Save force in wall variable if appropriate
			if( numVariableIndexLevel()>1 )
				wallData[i][variableIndex(1,0)] = coeff*distance;
      
      //Update both vertices for each dimension
      for(size_t d=0 ; d<dimension ; d++ ) {
				double div = (vertexData[v1][d]-vertexData[v2][d])*coeff;
				vertexDerivs[v1][d] -= div;
				vertexDerivs[v2][d] += div;
      }
    }
		else if( numVariableIndexLevel()>1 )
			wallData[i][variableIndex(1,0)] = 0.0;
  }
}

VertexFromEpidermalCellWallSpringAsymmetric::
VertexFromEpidermalCellWallSpringAsymmetric(std::vector<double> &paraValue, 
																						std::vector< std::vector<size_t> > 
																						&indValue ) 
{  
  //Do some checks on the parameters and variable indeces
  //////////////////////////////////////////////////////////////////////
  if( paraValue.size()!=2 ) {
    std::cerr << "VertexFromEpidermalCellWallSpringAsymmetric::"
							<< "VertexFromEpidermalCellWallSpringAsymmetric() "
							<< "Uses two parameters K_force frac_adhesion.\n";
    exit(0);
  }
  if( indValue.size() < 1 || indValue.size() > 2 
			|| indValue[0].size() != 1 
			|| (indValue.size()==2 && indValue[1].size() != 1) ) {
    std::cerr << "VertexFromEpidermalCellWallSpringAsymmetric::"
							<< "VertexFromEpidermalCellWallSpringAsymmetric() "
							<< "Wall length index given in first level,"
							<< " and optionally wall variable save index in second.\n";
    exit(0);
  }
  //Set the variable values
  //////////////////////////////////////////////////////////////////////
  setId("VertexFromEpidermalCellWallSpringAsymmetric");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  //Set the parameter identities
  //////////////////////////////////////////////////////////////////////
  std::vector<std::string> tmp( numParameter() );
  tmp[0] = "K_force";
  tmp[1] = "frac_adh";
  setParameterId( tmp );
}

void VertexFromEpidermalCellWallSpringAsymmetric::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) 
{
  
  //Do the update for each wall
  size_t numWalls = T.numWall();
  size_t wallLengthIndex = variableIndex(0,0);
  
  for( size_t i=0 ; i<numWalls ; ++i ) {
    if( T.wall(i).cell1() == T.background() ||
				T.wall(i).cell1()->isNeighbor(T.background()) ||
				T.wall(i).cell2() == T.background() || 
				T.wall(i).cell2()->isNeighbor(T.background()) ) {
      size_t v1 = T.wall(i).vertex1()->index();
      size_t v2 = T.wall(i).vertex2()->index();
      size_t dimension = vertexData[v1].size();
      assert( vertexData[v2].size()==dimension );
      //Calculate shared factors
      double distance=0.0;
      for( size_t d=0 ; d<dimension ; d++ )
				distance += (vertexData[v1][d]-vertexData[v2][d])*
					(vertexData[v1][d]-vertexData[v2][d]);
      distance = std::sqrt(distance);
      double wallLength=wallData[i][wallLengthIndex];
      double coeff = parameter(0)*((1.0/wallLength)-(1.0/distance));
      if( distance <= 0.0 && wallLength <=0.0 ) {
				//std::cerr << i << " - " << wallLength << " " << distance << std::endl;
				coeff = 0.0;
      }
      if( distance>wallLength )
				coeff *=parameter(1);
      
			//Save force in wall variable if appropriate
			if( numVariableIndexLevel()>1 )
				wallData[i][variableIndex(1,0)] = coeff*distance;
			
      //Update both vertices for each dimension
      for(size_t d=0 ; d<dimension ; d++ ) {
				double div = (vertexData[v1][d]-vertexData[v2][d])*coeff;
				vertexDerivs[v1][d] -= div;
				vertexDerivs[v2][d] += div;
      }
    }
		else if( numVariableIndexLevel()>1 )
			wallData[i][variableIndex(1,0)] = 0.0;
  }
}

VertexFromWallSpringExperimental::
VertexFromWallSpringExperimental(std::vector<double> &paraValue,
																 std::vector< std::vector<size_t> > &indValue)
{
	if (paraValue.size() != 1) {
		std::cerr << "VertexFromWallSpringExperimental::VertexFromWallSpringExperimental() "
				<< "Uses one parameter: k" << std::endl;
		exit(EXIT_FAILURE);
	}

     if (indValue.size() == 0 || indValue.size() > 2 || indValue[0].size() != 1) {
		std::cerr << "VertexFromWallSpringExperimental::VertexFromWallSpringExperimental() "
                    << "Wall length index given.\n";
          exit(EXIT_FAILURE);
     }

     if (indValue.size() == 1 && indValue[0].size() != 1) {
		std::cerr << "VertexFromWallSpringExperimental::VertexFromWallSpringExperimental() -"
                    << "Second level of indices gives index for storage of force.\n";
          exit(EXIT_FAILURE);
     }

	setId("VertexFromWallSpringExperimental");
	setParameter(paraValue);  
	setVariableIndex(indValue);
	
	std::vector<std::string> tmp(numParameter());
	tmp[0] = "k";
	setParameterId(tmp);
}

void VertexFromWallSpringExperimental::
derivs(Tissue &T,
			 std::vector< std::vector<double> > &cellData,
			 std::vector< std::vector<double> > &wallData,
			 std::vector< std::vector<double> > &vertexData,
			 std::vector< std::vector<double> > &cellDerivs,
			 std::vector< std::vector<double> > &wallDerivs,
			 std::vector< std::vector<double> > &vertexDerivs)
{
	for (size_t i = 0; i < T.numWall(); ++i) {
		size_t vertex1Index = T.wall(i).vertex1()->index();
		size_t vertex2Index = T.wall(i).vertex2()->index();
		size_t dimensions = vertexData[vertex1Index].size();
		
		double distance = 0.0;
		for (size_t d = 0; d < dimensions; ++d) {
			distance += (vertexData[vertex1Index][d] - vertexData[vertex2Index][d])
				* (vertexData[vertex1Index][d] - vertexData[vertex2Index][d]);
		}
		distance = std::sqrt(distance);

 		for (size_t d = 0; d < dimensions; ++d) {
			double dx1dt = parameter(0) * (vertexData[vertex2Index][d] - vertexData[vertex1Index][d])
				* (1.0/wallData[i][variableIndex(0, 0)] - 1.0/distance);

			vertexDerivs[vertex1Index][d] += dx1dt;
			vertexDerivs[vertex2Index][d] -= dx1dt;
		}
          if (numVariableIndexLevel() == 2) {
               wallData[T.wall(i).index()][variableIndex(1, 0)] = (parameter(0) / wallData[T.wall(i).index()][variableIndex(0, 0)]);
			wallData[T.wall(i).index()][variableIndex(1, 0)] *= (distance - wallData[T.wall(i).index()][variableIndex(0, 0)]);
          }
 	}
}

VertexFromWallSpringConcentrationHill::
VertexFromWallSpringConcentrationHill(std::vector<double> &paraValue, 
															 std::vector< std::vector<size_t> > 
															 &indValue ) 
{  
  // Do some checks on the parameters and variable indeces
  if( paraValue.size()!=5 ) {
    std::cerr << "VertexFromWallSpringConcentrationHill::"
							<< "VertexFromWallSpringConcentrationHill() "
							<< "Uses five parameters K_min, K_max, K_Hill, n_Hill and frac_adhesion.\n";
    exit(0);
  }
  if (indValue.size() < 1 || indValue.size() > 2 
			|| indValue[0].size() != 2 
			|| (indValue.size()==2 && indValue[1].size() != 1) ) {
    std::cerr << "VertexFromWallSpringConcentrationHill::"
							<< "VertexFromWallSpringConcentrationHill() "
							<< "Wall length index and cell concentration index given in first level,"
							<< " and optionally wall force save index in second.\n";
    exit(0);
  }
	
  // Set the variable values
  setId("VertexFromWallSpringConcentrationHill");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  // Set the parameter identities
  std::vector<std::string> tmp( numParameter() );
  tmp[0] = "K_min";
  tmp[1] = "K_max";
  tmp[2] = "K_Hill";
  tmp[3] = "n_Hill";
  tmp[4] = "frac_adh";
  setParameterId( tmp );
}

//! Derivative contribution for asymmetric wall springs on vertices
/*! 
*/
void VertexFromWallSpringConcentrationHill::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  
  //Do the update for each wall
  size_t numWalls = T.numWall();
  size_t wallLengthIndex = variableIndex(0,0);
	size_t concentrationIndex = variableIndex(0,1);
	size_t dimension = vertexData[0].size();
  
  for( size_t i=0 ; i<numWalls ; ++i ) {
    size_t v1 = T.wall(i).vertex1()->index();
    size_t v2 = T.wall(i).vertex2()->index();
    assert( vertexData[v2].size()==dimension );
    //Calculate shared factors
    double distance=0.0;
		std::vector<double> n_w(dimension),n_c1(dimension),n_c2(dimension);
    for( size_t d=0 ; d<dimension ; d++ ) {
			n_w[d] = vertexData[v2][d]-vertexData[v1][d];
			distance += n_w[d]*n_w[d];
		}
    distance = std::sqrt( distance );
		double c1Fac=0.0,c2Fac=0.0,KPow = std::pow(parameter(2),parameter(3));
		if( T.wall(i).cell1() != T.background() ) {
			double conc = cellData[T.wall(i).cell1()->index()][concentrationIndex];
			c1Fac = KPow/(KPow+std::pow(conc,parameter(3)));
		}
		if( T.wall(i).cell2() != T.background() ) {
			double conc = cellData[T.wall(i).cell2()->index()][concentrationIndex];
			c1Fac = KPow/(KPow+std::pow(conc,parameter(3)));
		}
		
    double wallLength=wallData[i][wallLengthIndex];
    double coeff = (parameter(0)+parameter(1)*(c1Fac+c2Fac))*
			((1.0/wallLength)-(1.0/distance));
    if( distance <= 0.0 && wallLength <=0.0 ) {
      //std::cerr << i << " - " << wallLength << " " << distance << std::endl;
      coeff = 0.0;
    }
    if( distance>wallLength )
      coeff *=parameter(4);
    
		//Save force in wall variable if appropriate
		if( numVariableIndexLevel()>1 )
			wallData[i][variableIndex(1,0)] = coeff*distance;
		
    //Update both vertices for each dimension
    for(size_t d=0 ; d<dimension ; d++ ) {
      double div = (vertexData[v1][d]-vertexData[v2][d])*coeff;
      vertexDerivs[v1][d] -= div;
      vertexDerivs[v2][d] += div;
    }
  }
}

VertexFromWallSpringMTConcentrationHill::
VertexFromWallSpringMTConcentrationHill(std::vector<double> &paraValue, 
													std::vector< std::vector<size_t> > 
													&indValue ) 
{  
  // Do some checks on the parameters and variable indeces
  if( paraValue.size()!=6 ) {
    std::cerr << "VertexFromWallSpringMTConcentrationHill::"
							<< "VertexFromWallSpringMTConcentrationHill() "
							<< "Uses six parameters K_0 frac_MT frac_conc K_Hill n_Hill "
							<< "frac_adhesion.\n";
    exit(0);
  }
  if( indValue.size() < 2 || indValue.size() > 3 
			|| indValue[0].size() != 2
			|| indValue[1].size() != 1
			|| (indValue.size()==3 && indValue[2].size() != 1) ) {
    std::cerr << "VertexFromWallSpringMTConcentrationHill::"
							<< "VertexFromWallSpringMTConcentrationHill() "
							<< "Wall length index and cell MT direction start index"
							<< "given at first level, conc index at second,"
							<< " and optionally wall variable save index in second.\n";
    exit(0);
  }
  //Set the variable values
  setId("VertexFromWallSpringMTConcentrationHill");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  //Set the parameter identities
  std::vector<std::string> tmp( numParameter() );
  tmp[0] = "K_0";
  tmp[1] = "frac_MT";
  tmp[2] = "frac_conc";
  tmp[3] = "K_Hill";
  tmp[4] = "n_Hill";
  tmp[5] = "frac_adh";
  setParameterId( tmp );	
}

void VertexFromWallSpringMTConcentrationHill::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) 
{  
  //Do the update for each wall
  size_t numWalls = T.numWall();
  size_t wallLengthIndex = variableIndex(0,0);
	size_t directionIndex = variableIndex(0,1);
	size_t dimension = vertexData[0].size();
  double KPow = std::pow(parameter(3),parameter(4));

  for( size_t i=0 ; i<numWalls ; ++i ) {
    size_t v1 = T.wall(i).vertex1()->index();
    size_t v2 = T.wall(i).vertex2()->index();
    assert( vertexData[v2].size()==dimension );
    //Calculate shared factors
    double distance=0.0,c1Norm=0.0,c2Norm=0.0;
		std::vector<double> n_w(dimension),n_c1(dimension),n_c2(dimension);
    for( size_t d=0 ; d<dimension ; d++ ) {
			n_w[d] = vertexData[v2][d]-vertexData[v1][d];
			distance += n_w[d]*n_w[d];
			if( T.wall(i).cell1() != T.background() && 
					cellData[T.wall(i).cell1()->index()][directionIndex+dimension]>0.5 ) {
				n_c1[d] = cellData[T.wall(i).cell1()->index()][directionIndex+d];
				c1Norm += n_c1[d]*n_c1[d];
			}
			if( T.wall(i).cell2() != T.background() &&
					cellData[T.wall(i).cell2()->index()][directionIndex+dimension]>0.5 ) {
				n_c2[d] = cellData[T.wall(i).cell2()->index()][directionIndex+d];
				c2Norm += n_c2[d]*n_c2[d];			
			}
		}
    distance = std::sqrt( distance );
		c1Norm = std::sqrt( c1Norm );
		c2Norm = std::sqrt( c2Norm );
		double c1Fac=0.0,c2Fac=0.0,c1FacConc=0.0,c2FacConc=0.0;
		if( T.wall(i).cell1() != T.background() &&
				cellData[T.wall(i).cell1()->index()][directionIndex+dimension]>0.5 ) {
			for( size_t d=0 ; d<dimension ; d++ )		
				c1Fac += n_c1[d]*n_w[d]/(c1Norm*distance);
			double conc=cellData[T.wall(i).cell1()->index()][variableIndex(1,0)];
			c1FacConc = KPow/(KPow+std::pow(conc,parameter(4)));
		}
		else
			c1Fac = 0.5;//1.0;
		if( T.wall(i).cell2() != T.background() &&
				cellData[T.wall(i).cell2()->index()][directionIndex+dimension]>0.5 ) {
			for( size_t d=0 ; d<dimension ; d++ )		
				c2Fac += n_c2[d]*n_w[d]/(c2Norm*distance);
			double conc=cellData[T.wall(i).cell2()->index()][variableIndex(1,0)];
			c1FacConc = KPow/(KPow+std::pow(conc,parameter(4)));
		}
		else
			c2Fac = 0.5;//1.0;
		
    double wallLength=wallData[i][wallLengthIndex];
    double coeff = parameter(0)*((1.0-parameter(1))+parameter(1)*0.5*(2.0-c1Fac-c2Fac))*
			((1.0-parameter(2))+parameter(2)*0.5*(c1FacConc+c2FacConc))*
			((1.0/wallLength)-(1.0/distance));
    if( distance <= 0.0 && wallLength <=0.0 ) {
      //std::cerr << i << " - " << wallLength << " " << distance << std::endl;
      coeff = 0.0;
    }
    if( distance>wallLength )
      coeff *=parameter(5);
    
		//Save force in wall variable if appropriate
		if( numVariableIndexLevel()>1 )
			wallData[i][variableIndex(1,0)] = coeff*distance;
		
    //Update both vertices for each dimension
    for(size_t d=0 ; d<dimension ; d++ ) {
      double div = (vertexData[v1][d]-vertexData[v2][d])*coeff;
      vertexDerivs[v1][d] -= div;
      vertexDerivs[v2][d] += div;
    }
  }
}