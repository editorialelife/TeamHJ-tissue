//
// Filename     : directionUpdate.cc
// Description  : Classes describing direction updates
// Author(s)    : Henrik Jonsson (henrik@thep.lu.se)
// Created      : June 2007
// Revision     : $Id:$
//
#include"directionUpdate.h"
#include"baseDirectionUpdate.h"
#include"myMath.h"

StaticDirection::
StaticDirection(std::vector<double> &paraValue, 
		std::vector< std::vector<size_t> > 
		&indValue ) 
{  
  // Do some checks on the parameters and variable indeces
  if( paraValue.size()!=0 ) {
    std::cerr << "StaticDirection::"
	      << "StaticDirection() "
	      << "No parameters used.\n";
    exit(0);
  }
  if( indValue.size() != 0 ) {
    std::cerr << "StaticDirection::"
	      << "StaticDirection() "
	      << "No variable index is used." << std::endl;
    exit(0);
  }
  
  // Set the variable values
  setId("StaticDirection");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  // Set the parameter identities
  std::vector<std::string> tmp( numParameter() );
  tmp.resize( numParameter() );
  //tmp[0] = "k_growth";
  setParameterId( tmp );
}

void StaticDirection::
initiate(Tissue &T,
	 DataMatrix &cellData,
	 DataMatrix &wallData,
	 DataMatrix &vertexData,
	 DataMatrix &cellDerivs,
	 DataMatrix &wallDerivs,
	 DataMatrix &vertexDerivs ) {
  
}

void StaticDirection::
update(Tissue &T, double h,
       DataMatrix &cellData,
       DataMatrix &wallData,
       DataMatrix &vertexData,
       DataMatrix &cellDerivs,
       DataMatrix &wallDerivs,
       DataMatrix &vertexDerivs ) {
  
}

//!Constructor
WallDirection::
WallDirection(std::vector<double> &paraValue, 
	      std::vector< std::vector<size_t> > 
	      &indValue ) 
{  
  // Do some checks on the parameters and variable indeces
  if( paraValue.size()!=0 ) {
    std::cerr << "WallDirection::"
	      << "WallDirection() "
	      << "No parameters used.\n";
    exit(0);
  }
  if( indValue.size() != 1 || indValue[0].size() != 1 ) {
    std::cerr << "WallDirection::"
	      << "WallDirection() "
	      << "One variable index is used (start of cell direction).\n";
    exit(0);
  }
  
  // Set the variable values
  setId("WallDirection");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  // Set the parameter identities
  std::vector<std::string> tmp( numParameter() );
  tmp.resize( numParameter() );
  //tmp[0] = "k_growth";
  setParameterId( tmp );
}

void WallDirection::
initiate(Tissue &T,
	 DataMatrix &cellData,
	 DataMatrix &wallData,
	 DataMatrix &vertexData,
	 DataMatrix &cellDerivs,
	 DataMatrix &wallDerivs,
	 DataMatrix &vertexDerivs ) {
  
  // Find walls with direction closest to given direction
  size_t dimension = T.numDimension();
  T.setNumDirectionalWall(T.numCell());
  std::vector<double> tmpN(dimension);
  
  for (size_t i=0; i<T.numCell(); ++i) {
    if ( T.cell(i).variable(variableIndex(0,0)+dimension) > 0.0 ) {
      double normW = 0.0;
      for (size_t dim=0; dim<dimension; ++dim) {
	tmpN[dim] = T.cell(i).wall(0)->vertex1()->position(dim) -
	  T.cell(i).wall(0)->vertex2()->position(dim);
	normW += tmpN[dim]*tmpN[dim];
      }
      normW = std::sqrt( normW );
      if (normW<=0.0) {
	std::cerr << "VertexFromWallSpringMT::initiate Normalization=0!"
		  << std::endl;
	exit(-1);
      }
      normW = 1.0/normW;
      double prod=0.0;
      for (size_t dim=0; dim<dimension; ++dim) {
	tmpN[dim] *= normW;
	prod += tmpN[dim]*T.cell(i).variable(dim+variableIndex(0,0));
      }
      size_t maxK=0;
      double maxProd = std::fabs(prod);
      
      for (size_t k=1; k<T.cell(i).numWall(); ++k) {
	normW = 0.0;
	for (size_t dim=0; dim<dimension; ++dim) {
	  tmpN[dim] = T.cell(i).wall(k)->vertex1()->position(dim) -
	    T.cell(i).wall(k)->vertex2()->position(dim);
	  normW += tmpN[dim]*tmpN[dim];
	}
	normW = std::sqrt( normW );
	if (normW<=0.0) {
	  std::cerr << "VertexFromWallSpringMT::initiate Normalization=0!"
		    << std::endl;
	  exit(-1);
	}
	normW = 1.0/normW;
	prod=0.0;
	for (size_t dim=0; dim<dimension; ++dim) {
	  tmpN[dim] *= normW;
	  prod += tmpN[dim]*T.cell(i).variable(dim+variableIndex(0,0));
	}
	prod = std::fabs(prod);
	if( prod>maxProd ) {
	  maxProd=prod;
	  maxK=k;
	}
      }
      T.setDirectionalWall(i,maxK);
      assert( T.cell(i).numWall()>T.directionalWall(i) );
    }
    else
      T.setDirectionalWall(i,static_cast<size_t>(-1));
  }	  
}

void WallDirection::
update(Tissue &T, double h,
       DataMatrix &cellData,
       DataMatrix &wallData,
       DataMatrix &vertexData,
       DataMatrix &cellDerivs,
       DataMatrix &wallDerivs,
       DataMatrix &vertexDerivs ) {
  
  size_t dimension=vertexData[0].size(); 
  for (size_t i=0; i<T.numDirectionalWall(); ++i) {
    if (T.directionalWall(i)<T.cell(i).numWall()) {
      std::vector<double> tmpN(dimension);
      size_t v1I = T.cell(i).wall(T.directionalWall(i))->vertex1()->index();
      size_t v2I = T.cell(i).wall(T.directionalWall(i))->vertex2()->index();
      double normW=0.0;
      for (size_t dim=0; dim<dimension; ++dim) {
	tmpN[dim] = vertexData[v2I][dim] - vertexData[v1I][dim];
	normW += tmpN[dim]*tmpN[dim];
      }
      normW = std::sqrt(normW);
      if( normW<=0.0 ) {
	std::cerr << "WallDirection::update() Wrong norm factor"
		  << std::endl;
	exit(-1);
      }
      normW = 1.0/normW;
      for (size_t dim=0; dim<dimension; ++dim) {
	tmpN[dim] *= normW;
	cellData[i][dim+variableIndex(0,0)] = tmpN[dim];
      }
    }
  }
}

StrainDirection::
StrainDirection(std::vector<double> &paraValue, 
		std::vector< std::vector<size_t> > 
		&indValue ) 
{  
  //
  // Do some checks on the parameters and variable indeces
  //
  if( paraValue.size()!=1 ) {
    std::cerr << "StrainDirection::"
	      << "StrainDirection() "
	      << "One parameter used flag_perpendicular."
	      << std::endl;
    exit(0);
  }
  if( indValue.size() != 1 || indValue[0].size() != 1 ) {
    std::cerr << "StrainDirection::"
	      << "StrainDirection() "
	      << "One variable index is used (start of cell direction).\n";
    exit(0);
  }
  //Set the variable values
  //////////////////////////////////////////////////////////////////////
  setId("StrainDirection");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  //Set the parameter identities
  //////////////////////////////////////////////////////////////////////
  std::vector<std::string> tmp( numParameter() );
  tmp.resize( numParameter() );
  tmp[0] = "flag_perpendicular";
  setParameterId( tmp );
}

void StrainDirection::
initiate(Tissue &T,
	 DataMatrix &cellData,
	 DataMatrix &wallData,
	 DataMatrix &vertexData,
	 DataMatrix &cellDerivs,
	 DataMatrix &wallDerivs,
	 DataMatrix &vertexDerivs ) {
  
}

void StrainDirection::
update(Tissue &T, double h,
       DataMatrix &cellData,
       DataMatrix &wallData,
       DataMatrix &vertexData,
       DataMatrix &cellDerivs,
       DataMatrix &wallDerivs,
       DataMatrix &vertexDerivs ) 
{
  size_t dimension = vertexData[0].size();
  assert( dimension==2 );
  
  //
  //Calculate strain directions and print walls and strain vectors
  //by using x,x+dt*dx/dt as two points
  //
  T.derivs(cellData,wallData,vertexData,
	   cellDerivs,wallDerivs,vertexDerivs);
  
  //
  // Update all cells
  //
  for (size_t cellI=0; cellI<T.numCell(); ++cellI) {
    //Create temporary x,y,dx positions
    size_t numV = T.cell(cellI).numVertex(); 
    DataMatrix x(numV),y(numV),dx(numV),
      xM(numV),yM(numV),dxM(numV);
    
    double dt=1.0;
    // 		std::vector<double> xMean(dimension),yMean(dimension),
    // 			dxMean(dimension);
    std::vector<double> xMean(dimension),yMean(dimension);
    
    for( size_t i=0 ; i<numV ; ++i ) {
      size_t vI = T.cell(cellI).vertex(i)->index();
      x[i] = vertexData[vI];
      dx[i] = vertexDerivs[vI];
      std::vector<double> tmp(dimension);
      tmp[0] = x[i][0]+dt*dx[i][0];
      tmp[1] = x[i][1]+dt*dx[i][1];
      y[i] = tmp;
      xMean[0] += x[i][0];
      xMean[1] += x[i][1];
      yMean[0] += y[i][0];
      yMean[1] += y[i][1];
      // 			dxMean[0] += dx[i][0];			
      // 			dxMean[1] += dx[i][1];			
    }
    xMean[0] /=numV;
    xMean[1] /=numV;
    yMean[0] /=numV;
    yMean[1] /=numV;
    // 		dxMean[0] /=numV;
    // 		dxMean[1] /=numV;
    for( size_t i=0 ; i<numV ; ++i ) {
      xM[i].resize(dimension);
      xM[i][0] =x[i][0]-xMean[0];
      xM[i][1] =x[i][1]-xMean[1];
      yM[i].resize(dimension);
      yM[i][0] =y[i][0]-yMean[0];
      yM[i][1] =y[i][1]-yMean[1];
      dxM[i].resize(dimension);
      // 			dxM[i][0] =dx[i][0]-dxMean[0];
      // 			dxM[i][1] =dx[i][1]-dxMean[1];
      dxM[i][0] =dx[i][0];
      dxM[i][1] =dx[i][1];
      
    }
    
    //Calculate A = (x^t x)^{-1} (x^t y)
    std::vector<std::vector<double> > xTx(dimension),xTy(dimension),
      xTxM(dimension),A(dimension);
    for( size_t i=0 ; i<dimension ; ++i ) {
      xTx[i].resize(dimension);
      xTy[i].resize(dimension);
      xTxM[i].resize(dimension);
      A[i].resize(dimension);
    }
    
    for( size_t i=0 ; i<dimension ; ++i ) {
      for( size_t j=0 ; j<dimension ; ++j ) {
	for( size_t v=0 ; v<numV ; ++v ) {
	  xTx[i][j] += xM[v][i]*xM[v][j];
	  xTy[i][j] += xM[v][i]*yM[v][j];
	}
      }
    }
    double detM = xTx[0][0]*xTx[1][1]-xTx[0][1]*xTx[1][0];
    detM = 1.0/detM;
    xTxM[0][0] = detM*xTx[1][1];
    xTxM[1][1] = detM*xTx[0][0];
    // 		xTxM[0][1] = -detM*xTx[1][0];
    // 		xTxM[1][0] = -detM*xTx[0][1];
    xTxM[0][1] = -detM*xTx[0][1];
    xTxM[1][0] = -detM*xTx[1][0];
    
    //Calculate A
    A[0][0] = xTxM[0][0]*xTy[0][0] + xTxM[0][1]*xTy[1][0];
    A[0][1] = xTxM[0][0]*xTy[0][1] + xTxM[0][1]*xTy[1][1];
    A[1][0] = xTxM[1][0]*xTy[0][0] + xTxM[1][1]*xTy[1][0];
    A[1][1] = xTxM[1][0]*xTy[0][1] + xTxM[1][1]*xTy[1][1];
    
    //Apply SVD to A
    //
    
    //Make sure determinant is non-zero
    double detA = A[0][0]*A[1][1] - A[0][1]*A[1][0];
    if( detA==0 ) {
      std::cerr << "StrainDirection::update() Determinant zero\n";
      exit(-1);
    }
    //double tau = std::atan2( A[0][0]-A[1][1],A[0][1]+A[1][0] );
    //double omega = std::atan2( A[0][0]+A[1][1],A[0][1]-A[1][0] );
    double tau = std::atan2( A[0][1]+A[1][0],A[0][0]-A[1][1] );
    double omega = std::atan2( A[0][1]-A[1][0],A[0][0]+A[1][1] );
    double theta = 0.5*(tau-omega);
    //double phi = 0.5*(tau+omega);
    
    //Create direction for update
    std::vector<double> n(dimension);
    double v = theta;
    if( parameter(0)==1.0 )		
      v = v - 0.5 * myMath::pi();
    
    // 		double a = A[0][0];
    // 		double b = A[0][1];
    // 		double c = A[1][0];
    // 		double d = A[1][1];
    
    // 		double t = std::sqrt((a + d) * (a + d) + (b - c) * (b - c));
    // 		double w = std::sqrt((a - d) * (a - d) + (b + c) * (b + c));
    
    // 		double p = 0.5 * (t + w);
    // 		double q = 0.5 * (t - w);
    
    // 		assert(std::abs(p) >= std::abs(q));
    n[0]=std::cos(v);
    n[1]=std::sin(v);
    for (size_t dim=0; dim<dimension; ++dim) 
      cellData[cellI][variableIndex(0,0)+dim] = n[dim];
  }
} 

GradientDirection::
GradientDirection(std::vector<double> &paraValue, 
		  std::vector< std::vector<size_t> > 
							&indValue ) 
{  
	//
  // Do some checks on the parameters and variable indeces
  //
  if( paraValue.size()!=0 ) {
    std::cerr << "GradientDirection::"
							<< "GradientDirection() "
							<< "No parameters used.\n";
    exit(0);
  }
  if( indValue.size() != 1 || indValue[0].size() != 2 ) {
    std::cerr << "GradientDirection::"
							<< "GradientDirection() "
							<< "Two variable indices are used (start of cell direction,"
							<< " gradient variable).\n";
    exit(0);
  }
  //Set the variable values
  //////////////////////////////////////////////////////////////////////
  setId("GradientDirection");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  //Set the parameter identities
  //////////////////////////////////////////////////////////////////////
  std::vector<std::string> tmp( numParameter() );
  tmp.resize( numParameter() );
  //tmp[0] = "k_growth";
  setParameterId( tmp );
}

void GradientDirection::
initiate(Tissue &T,
				 DataMatrix &cellData,
				 DataMatrix &wallData,
				 DataMatrix &vertexData,
				 DataMatrix &cellDerivs,
				 DataMatrix &wallDerivs,
				 DataMatrix &vertexDerivs ) {
  
}

void GradientDirection::
update(Tissue &T, double h,
			 DataMatrix &cellData,
			 DataMatrix &wallData,
			 DataMatrix &vertexData,
			 DataMatrix &cellDerivs,
			 DataMatrix &wallDerivs,
			 DataMatrix &vertexDerivs ) {
  
}

WallStressDirection::
WallStressDirection(std::vector<double> &paraValue, std::vector< std::vector<size_t> > &indValue)
{
	if (paraValue.size() != 1) {
	  std::cerr << "WallStressDirection::WallStressDirection() " 
		    << "One parameter is used orientation_flag (0 for direction parallel with "
		    << "force, 1 for direction perpendicular to force)" << std::endl;
	  exit(EXIT_FAILURE);
	}

	if (indValue.size() != 2 || indValue[0].size() != 1) {
	  std::cerr << "WallStressDirection::WallStressDirection() \n"
		    << "First level: Start of cell direction index are used.\n"
		    << "Second level: Wall force indices\n";
		exit(EXIT_FAILURE);
	}

	setId("WallStressDirection");
	setParameter(paraValue);  
	setVariableIndex(indValue);
	
	std::vector<std::string> tmp(numParameter());
	tmp.resize(numParameter());
	tmp[0] = "orientation_flag";
	setParameterId(tmp);
}
  
void WallStressDirection::initiate(Tissue &T,
						DataMatrix &cellData,
						DataMatrix &wallData,
						DataMatrix &vertexData,
						DataMatrix &cellDerivs,
						DataMatrix &wallDerivs,
						DataMatrix &vertexDerivs)
{
	// No initialization
}

void WallStressDirection::update(Tissue &T, double h,
					   DataMatrix &cellData,
					   DataMatrix &wallData,
					   DataMatrix &vertexData,
					   DataMatrix &cellDerivs,
					   DataMatrix &wallDerivs,
					   DataMatrix &vertexDerivs)
{
	size_t dimension = vertexData[0].size();
	if (dimension==2) { 
		for (size_t n = 0; n < T.numCell(); ++n) {
			Cell cell = T.cell(n);
			
			if (cellData[cell.index()][variableIndex(0, 0) + 2] == 0) {
				continue;
			}
			double enumerator = 0.0;
			double denominator = 0.0;
			
			for (size_t i = 0; i < cell.numWall(); ++i) {
				Wall *wall = cell.wall(i);
				
				double wx = (vertexData[wall->vertex1()->index()][0] - 
										 vertexData[wall->vertex2()->index()][0]); 
				double wy = (vertexData[wall->vertex1()->index()][1] - 
										 vertexData[wall->vertex2()->index()][1]); 
				
				// Dodgy error check. Might have to improve it.
				if (wx < 0) {
					wx *= -1.0;
					wy *= -1.0;
				}
				double sigma = std::atan2(wy, wx);
				
				double c = std::cos(2.0 * sigma);
				double s = std::sin(2.0 * sigma);
				
				double force = 0.0;
				for (size_t j = 0; j < numVariableIndex(1); ++j) {
					force += wallData[wall->index()][variableIndex(1, j)];
				}
				enumerator += force * s;
				denominator += force * c;
			}
			
			double angle = std::atan2(enumerator, denominator);
			
			double x = std::cos(0.5 * angle);
			double y = std::sin(0.5 * angle);
			
			if (parameter(0) == 0) {
				cellData[cell.index()][variableIndex(0, 0) + 0] = x;
				cellData[cell.index()][variableIndex(0, 0) + 1] = y;
			} else {
				cellData[cell.index()][variableIndex(0, 0) + 0] = - y;
				cellData[cell.index()][variableIndex(0, 0) + 1] = x;
			}
		}
	}
	else if (dimension==3) {
		for (size_t n = 0; n < T.numCell(); ++n) {
			Cell cell = T.cell(n);
			
			if (cellData[cell.index()][variableIndex(0, 0) + dimension] == 0) {
				continue;
			}
			
			// This calculation should now be done in reaction CalculatePCAPlane
			//cell.calculatePCAPlane(vertexData);
			DataMatrix axes = cell.getPCAPlane();
			std::vector< std::pair<double, double> > vertices = cell.projectVerticesOnPCAPlane(vertexData);
			
			double enumerator = 0.0;
			double denominator = 0.0;
			
			for (size_t i = 0; i < cell.numWall(); ++i) {
				size_t ii = (i+1)%cell.numWall();
				Wall *wall = cell.wall(i);
				
				double wx = vertices[i].first - vertices[ii].first;
				double wy = vertices[i].second - vertices[ii].second;
				
				// Dodgy error check. Might have to improve it.
				if (wx < 0) {
					wx *= -1.0;
					wy *= -1.0;
				}
				double sigma = std::atan2(wy, wx);
				
				double c = std::cos(2.0 * sigma);
				double s = std::sin(2.0 * sigma);
				
				double force = 0.0;

				for (size_t j = 0; j < numVariableIndex(1); ++j) {
					force += wallData[wall->index()][variableIndex(1, j)];
				}
				enumerator += force * s;
				denominator += force * c;
			}
			
			double angle = std::atan2(enumerator, denominator);
			
			double x = std::cos(0.5 * angle);
			double y = std::sin(0.5 * angle);
			
			
			if (parameter(0) == 1) {
				//Use perpendicular direction
				double tmp = -y;
				y = x;
				x = tmp;
			}
			std::vector<double> dir(dimension);
			double norm=0.0;
			for (size_t d=0; d<dimension; ++d) {
				dir[d] = x * axes[0][d] + y * axes[1][d];
				norm += dir[d]*dir[d];
			}
			norm = 1.0/std::sqrt(norm);
			for (size_t d=0; d<dimension; ++d)
				cellData[cell.index()][variableIndex(0, 0) + d] = dir[d] * norm;
		}		
	}
}

DoubleWallStressDirection::
DoubleWallStressDirection(std::vector<double> &paraValue, std::vector< std::vector<size_t> > &indValue)
{
	if (paraValue.size() != 1) {
		std::cerr << "DoubleWallStressDirection::DoubleWallStressDirection() " 
							<< "One parameter is used orientation_flag (0 for direction parallel with "
							<< "force, 1 for direction perpendicular to force)" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (indValue.size() != 2 || indValue[0].size() != 1 || indValue[1].size() != 3 ) {
		std::cerr << "DoubleWallStressDirection::DoubleWallStressDirection() \n"
							<< "First level: Start of cell direction index is used.\n"
							<< "Second level: Wall force index, and two wall segment k indices.\n";
		exit(EXIT_FAILURE);
	}

	setId("DoubleWallStressDirection");
	setParameter(paraValue);  
	setVariableIndex(indValue);
	
	std::vector<std::string> tmp(numParameter());
	tmp.resize(numParameter());
	tmp[0] = "orientation_flag";
	setParameterId(tmp);
}

void DoubleWallStressDirection::
initiate(Tissue &T,
				 DataMatrix &cellData,
				 DataMatrix &wallData,
				 DataMatrix &vertexData,
				 DataMatrix &cellDerivs,
				 DataMatrix &wallDerivs,
				 DataMatrix &vertexDerivs)
{
	// No initialization
}

void DoubleWallStressDirection::
update(Tissue &T, double h,
			 DataMatrix &cellData,
			 DataMatrix &wallData,
			 DataMatrix &vertexData,
			 DataMatrix &cellDerivs,
			 DataMatrix &wallDerivs,
			 DataMatrix &vertexDerivs)
{
	size_t dimension = vertexData[0].size();
	if (dimension==2) { 
		for (size_t n = 0; n < T.numCell(); ++n) {
			Cell cell = T.cell(n);
			
			if (cellData[cell.index()][variableIndex(0, 0) + 2] == 0) {
				continue;
			}
			double enumerator = 0.0;
			double denominator = 0.0;
			
			for (size_t i = 0; i < cell.numWall(); ++i) {
				Wall *wall = cell.wall(i);
				
				double wx = (vertexData[wall->vertex1()->index()][0] - 
										 vertexData[wall->vertex2()->index()][0]); 
				double wy = (vertexData[wall->vertex1()->index()][1] - 
										 vertexData[wall->vertex2()->index()][1]); 
				
				// Dodgy error check. Might have to improve it.
				if (wx < 0) {
					wx *= -1.0;
					wy *= -1.0;
				}
				double sigma = std::atan2(wy, wx);
				
				double c = std::cos(2.0 * sigma);
				double s = std::sin(2.0 * sigma);
				
				double force = wallData[wall->index()][variableIndex(1, 0)];
				//extract force belonging to current wall segment
				size_t kI = variableIndex(1,1);
				if (wall->cell1()->index() != cell.index()) 
					kI = variableIndex(1,2);
				force *= wallData[wall->index()][kI] /
					(wallData[wall->index()][variableIndex(1,1)] + 
					 wallData[wall->index()][variableIndex(1,2)]);
				
				enumerator += force * s;
				denominator += force * c;
			}
			
			double angle = std::atan2(enumerator, denominator);
			
			double x = std::cos(0.5 * angle);
			double y = std::sin(0.5 * angle);
			
			if (parameter(0) == 0) {
				cellData[cell.index()][variableIndex(0, 0) + 0] = x;
				cellData[cell.index()][variableIndex(0, 0) + 1] = y;
			} else {
				cellData[cell.index()][variableIndex(0, 0) + 0] = - y;
				cellData[cell.index()][variableIndex(0, 0) + 1] = x;
			}
		}
	}
	else if (dimension==3) {
		for (size_t n = 0; n < T.numCell(); ++n) {
			Cell cell = T.cell(n);
			
			if (cellData[cell.index()][variableIndex(0, 0) + dimension] == 0) {
				continue;
			}
			
			// This calculation should now be done in reaction CalculatePCAPlane
			//cell.calculatePCAPlane(vertexData);
			DataMatrix axes = cell.getPCAPlane();
			std::vector< std::pair<double, double> > vertices = cell.projectVerticesOnPCAPlane(vertexData);
			
			double enumerator = 0.0;
			double denominator = 0.0;
			
			for (size_t i = 0; i < cell.numWall(); ++i) {
				size_t ii = (i+1)%cell.numWall();
				Wall *wall = cell.wall(i);
				
				double wx = vertices[i].first - vertices[ii].first;
				double wy = vertices[i].second - vertices[ii].second;
				
				// Dodgy error check. Might have to improve it.
				if (wx < 0) {
					wx *= -1.0;
					wy *= -1.0;
				}
				double sigma = std::atan2(wy, wx);
				
				double c = std::cos(2.0 * sigma);
				double s = std::sin(2.0 * sigma);
				
				double force = wallData[wall->index()][variableIndex(1, 0)];
				//extract force belonging to current wall segment
				size_t kI = variableIndex(1,1);
				if (wall->cell1()->index() != cell.index()) 
					kI = variableIndex(1,2);
				force *= wallData[wall->index()][kI] /
					(wallData[wall->index()][variableIndex(1,1)] + 
					 wallData[wall->index()][variableIndex(1,2)]);
				
				enumerator += force * s;
				denominator += force * c;
			}
			
			double angle = std::atan2(enumerator, denominator);
			
			double x = std::cos(0.5 * angle);
			double y = std::sin(0.5 * angle);
			
			
			if (parameter(0) == 1) {
				//Use perpendicular direction
				double tmp = -y;
				y = x;
				x = tmp;
			}
			std::vector<double> dir(dimension);
			double norm=0.0;
			for (size_t d=0; d<dimension; ++d) {
				dir[d] = x * axes[0][d] + y * axes[1][d];
				norm += dir[d]*dir[d];
			}
			norm = 1.0/std::sqrt(norm);
			for (size_t d=0; d<dimension; ++d)
				cellData[cell.index()][variableIndex(0, 0) + d] = dir[d] * norm;
		}		
	}
}

StretchDirection::
StretchDirection(std::vector<double> &paraValue, std::vector< std::vector<size_t> > &indValue)
{
	if (paraValue.size() != 1) {
		std::cerr << "StretchDirection::StretchDirection() " 
							<< "One parameter is used orientation_flag (0 for direction parallel with "
							<< "stretch, 1 for direction perpendicular to stretch)" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	if (indValue.size() != 2 || indValue[0].size() != 1 || 
			indValue[1].size() != 1) {
		std::cerr << "StretchDirection::StretchDirection() \n"
							<< "First level: Start of cell direction index is used.\n"
							<< "Second level: Wall length index." << std::endl;
		exit(EXIT_FAILURE);
	}
	
	setId("StretchDirection");
	setParameter(paraValue);  
	setVariableIndex(indValue);
	
	std::vector<std::string> tmp(numParameter());
	tmp.resize(numParameter());
	tmp[0] = "orientation_flag";
	setParameterId(tmp);
}

void StretchDirection::
initiate(Tissue &T,
				 DataMatrix &cellData,
				 DataMatrix &wallData,
				 DataMatrix &vertexData,
				 DataMatrix &cellDerivs,
				 DataMatrix &wallDerivs,
				 DataMatrix &vertexDerivs)
{
	// No initialization
}

void StretchDirection::
update(Tissue &T, double h,
			 DataMatrix &cellData,
			 DataMatrix &wallData,
			 DataMatrix &vertexData,
			 DataMatrix &cellDerivs,
			 DataMatrix &wallDerivs,
			 DataMatrix &vertexDerivs)
{
	size_t dimension = vertexData[0].size();
	if (dimension==2) { 
		for (size_t n = 0; n < T.numCell(); ++n) {
			Cell &cell = T.cell(n);

			if (cellData[cell.index()][variableIndex(0, 0) + 2] == 0) {
				continue;
			}

			double enumerator = 0.0;
			double denominator = 0.0;
			
			for (size_t i = 0; i < cell.numWall(); ++i) {
				Wall *wall = cell.wall(i);

				double wx = (vertexData[wall->vertex1()->index()][0] - 
										 vertexData[wall->vertex2()->index()][0]); 
				double wy = (vertexData[wall->vertex1()->index()][1] - 
										 vertexData[wall->vertex2()->index()][1]); 


				// Dodgy error check. Might have to improve it.
				if (wx < 0) {
					wx *= -1.0;
					wy *= -1.0;
				}
				double sigma = std::atan2(wy, wx);
				
				double c = std::cos(2.0 * sigma);
				double s = std::sin(2.0 * sigma);
				
				double force = 0.0, distance=0.0;
				for (size_t d=0; d<dimension; ++d)
					distance += ((vertexData[wall->vertex1()->index()][d] - 
												vertexData[wall->vertex2()->index()][d]) *
											 (vertexData[wall->vertex1()->index()][d] - 
												vertexData[wall->vertex2()->index()][d]));
				distance = std::sqrt(distance);
				force = ((distance-wallData[wall->index()][variableIndex(1, 0)]) /
								 wallData[wall->index()][variableIndex(1, 0)]);
				enumerator += force * s;
				denominator += force * c;
			}
			
			double angle = std::atan2(enumerator, denominator);
			
			double x = std::cos(0.5 * angle);
			double y = std::sin(0.5 * angle);
		
			if (parameter(0) == 0) {
				cellData[cell.index()][variableIndex(0, 0) + 0] = x;
				cellData[cell.index()][variableIndex(0, 0) + 1] = y;
			} else {
				cellData[cell.index()][variableIndex(0, 0) + 0] = - y;
				cellData[cell.index()][variableIndex(0, 0) + 1] = x;
			}
		}
	}
	else if (dimension==3) {
		for (size_t n = 0; n < T.numCell(); ++n) {
			Cell cell = T.cell(n);
			
			if (cellData[cell.index()][variableIndex(0, 0) + dimension] == 0) {
				continue;
			}
			// This calculation should now be done in reaction CalculatePCAPlane
			//cell.calculatePCAPlane(vertexData);
			DataMatrix axes = cell.getPCAPlane();
			std::vector< std::pair<double, double> > vertices = cell.projectVerticesOnPCAPlane(vertexData);
			
			double enumerator = 0.0;
			double denominator = 0.0;
			
			for (size_t i = 0; i < cell.numWall(); ++i) {
				size_t ii = (i+1)%cell.numWall();
				Wall *wall = cell.wall(i);

				double wx = vertices[i].first - vertices[ii].first;
				double wy = vertices[i].second - vertices[ii].second;
				
				// Dodgy error check. Might have to improve it.
				if (wx < 0) {
					wx *= -1.0;
					wy *= -1.0;
				}
				double sigma = std::atan2(wy, wx);
				
				double c = std::cos(2.0 * sigma);
				double s = std::sin(2.0 * sigma);
				
				double force = 0.0, distance=0.0;
				for (size_t d=0; d<dimension; ++d)
					distance += ((vertexData[wall->vertex1()->index()][d] - 
												vertexData[wall->vertex2()->index()][d]) *
											 (vertexData[wall->vertex1()->index()][d] - 
												vertexData[wall->vertex2()->index()][d]));
				distance = std::sqrt(distance);
				force = ((distance-wallData[wall->index()][variableIndex(1, 0)]) /
								 wallData[wall->index()][variableIndex(1, 0)]);
				enumerator += force * s;
				denominator += force * c;
			}
			
			double angle = std::atan2(enumerator, denominator);
			
			double x = std::cos(0.5 * angle);
			double y = std::sin(0.5 * angle);
			
			
			if (parameter(0) == 1) {
				//Use perpendicular direction
				double tmp = -y;
				y = x;
				x = tmp;
			}
			std::vector<double> dir(dimension);
			double norm=0.0;
			for (size_t d=0; d<dimension; ++d) {
				dir[d] = x * axes[0][d] + y * axes[1][d];
				norm += dir[d]*dir[d];
			}
			norm = 1.0/std::sqrt(norm);
			for (size_t d=0; d<dimension; ++d)
				cellData[cell.index()][variableIndex(0, 0) + d] = dir[d] * norm;
		}		
	}
}

StrainDirectionWall::
StrainDirectionWall(std::vector<double> &paraValue, std::vector< std::vector<size_t> > &indValue)
{
	if (paraValue.size() != 1) {
		std::cerr << "StrainDirectionWall::StrainDirectionWall() " 
							<< "One parameter is used orientation_flag (0 for direction parallel with "
							<< "strain, 1 for direction perpendicular to strain)" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	if (indValue.size() != 1 || indValue[0].size() != 1 ) {
		std::cerr << "StrainDirectionWall::StrainDirectionWall() \n"
							<< "First level: Start of cell direction index is used."
							<< std::endl;
		exit(EXIT_FAILURE);
	}
	
	setId("StrainDirectionWall");
	setParameter(paraValue);  
	setVariableIndex(indValue);
	
	std::vector<std::string> tmp(numParameter());
	tmp.resize(numParameter());
	tmp[0] = "orientation_flag";
	setParameterId(tmp);
}

void StrainDirectionWall::
initiate(Tissue &T,
				 DataMatrix &cellData,
				 DataMatrix &wallData,
				 DataMatrix &vertexData,
				 DataMatrix &cellDerivs,
				 DataMatrix &wallDerivs,
				 DataMatrix &vertexDerivs)
{
	// No initialization
}

void StrainDirectionWall::
update(Tissue &T, double h,
			 DataMatrix &cellData,
			 DataMatrix &wallData,
			 DataMatrix &vertexData,
			 DataMatrix &cellDerivs,
			 DataMatrix &wallDerivs,
			 DataMatrix &vertexDerivs)
{
	T.derivs(cellData,wallData,vertexData,cellDerivs,wallDerivs,vertexDerivs);
	size_t dimension = vertexData[0].size();
	if (dimension==2) { 
		for (size_t n = 0; n < T.numCell(); ++n) {
			Cell &cell = T.cell(n);
			
			if (cellData[cell.index()][variableIndex(0, 0) + dimension] == 0) {
				continue;
			}
			
			double enumerator = 0.0;
			double denominator = 0.0;
			
			for (size_t i = 0; i < cell.numWall(); ++i) {
				Wall *wall = cell.wall(i);
				size_t v1I=wall->vertex1()->index();
				size_t v2I=wall->vertex2()->index();
				
				double wx = vertexData[v1I][0] - vertexData[v2I][0];
				double wy = vertexData[v1I][1] - vertexData[v2I][1];				
				
				// Dodgy error check. Might have to improve it.
				if (wx < 0) {
					wx *= -1.0;
					wy *= -1.0;
				}
				double sigma = std::atan2(wy, wx);
				
				double c = std::cos(2.0 * sigma);
				double s = std::sin(2.0 * sigma);
				
				double strain = 0.0, distance=0.0, distance2=0.0;
				for (size_t d=0; d<dimension; ++d) {
					distance += ((vertexData[v1I][d] - vertexData[v2I][d]) *
											 (vertexData[v1I][d] - vertexData[v2I][d]));
					distance2 += ((vertexData[v1I][d] + vertexDerivs[v1I][d]
												 - vertexData[v2I][d] - vertexDerivs[v2I][d]) *
												(vertexData[v1I][d] + vertexDerivs[v1I][d]
												 - vertexData[v2I][d] - vertexDerivs[v2I][d]));
				}
				distance = std::sqrt(distance);
				distance2 = std::sqrt(distance2);
				strain = (distance2-distance) / distance;
				
				enumerator += strain * s;
				denominator += strain * c;
			}
			
			double angle = std::atan2(enumerator, denominator);
			
			double x = std::cos(0.5 * angle);
			double y = std::sin(0.5 * angle);
			if (parameter(0) == 0) {
				cellData[cell.index()][variableIndex(0, 0) + 0] = x;
				cellData[cell.index()][variableIndex(0, 0) + 1] = y;
			} else {
				cellData[cell.index()][variableIndex(0, 0) + 0] = - y;
				cellData[cell.index()][variableIndex(0, 0) + 1] = x;
			}
		}
	}
	else if (dimension==3) {
		for (size_t n = 0; n < T.numCell(); ++n) {
			Cell cell = T.cell(n);
			
			if (cellData[cell.index()][variableIndex(0, 0) + dimension] == 0) {
				continue;
			}
			// This calculation should now be done in reaction CalculatePCAPlane
			//cell.calculatePCAPlane(vertexData);
			DataMatrix axes = cell.getPCAPlane();
			std::vector< std::pair<double, double> > vertices = cell.projectVerticesOnPCAPlane(vertexData);
			
			double enumerator = 0.0;
			double denominator = 0.0;
			
			for (size_t i = 0; i < cell.numWall(); ++i) {
				size_t ii = (i+1)%cell.numWall();
				Wall *wall = cell.wall(i);
				
				double wx = vertices[i].first - vertices[ii].first;
				double wy = vertices[i].second - vertices[ii].second;
				
				// Dodgy error check. Might have to improve it.
				if (wx < 0) {
					wx *= -1.0;
					wy *= -1.0;
				}
				double sigma = std::atan2(wy, wx);
				
				double c = std::cos(2.0 * sigma);
				double s = std::sin(2.0 * sigma);
				
				size_t v1I=wall->vertex1()->index();
				size_t v2I=wall->vertex2()->index();
				double strain = 0.0, distance=0.0, distance2=0.0;
				for (size_t d=0; d<dimension; ++d) {
					distance += ((vertexData[v1I][d] - 
												vertexData[v2I][d]) *
											 (vertexData[v1I][d] - 
												vertexData[v2I][d]));
					distance2 += ((vertexData[v1I][d] + vertexDerivs[v1I][d] - 
												 vertexData[v2I][d] - vertexDerivs[v2I][d]) *
												(vertexData[v1I][d] + vertexDerivs[v1I][d] - 
												 vertexData[v2I][d] - vertexDerivs[v2I][d]));
				}
				distance = std::sqrt(distance);
				distance2 = std::sqrt(distance2);
				strain = (distance2-distance) / distance;
				enumerator += strain * s;
				denominator += strain * c;
			}
			
			double angle = std::atan2(enumerator, denominator);
			
			double x = std::cos(0.5 * angle);
			double y = std::sin(0.5 * angle);
			
			
			if (parameter(0) == 1) {
				//Use perpendicular direction
				double tmp = -y;
				y = x;
				x = tmp;
			}
			std::vector<double> dir(dimension);
			double norm=0.0;
			for (size_t d=0; d<dimension; ++d) {
				dir[d] = x * axes[0][d] + y * axes[1][d];
				norm += dir[d]*dir[d];
			}
			norm = 1.0/std::sqrt(norm);
			for (size_t d=0; d<dimension; ++d)
				cellData[cell.index()][variableIndex(0, 0) + d] = dir[d] * norm;
		}		
	}
}

PCAPlaneDirection::
PCAPlaneDirection(std::vector<double> &paraValue, std::vector< std::vector<size_t> > &indValue)
{
	if (paraValue.size() != 1) {
		std::cerr << "PCAPlaneDirection::PCAPlaneDirection() " 
							<< "One parameter is used orientation_flag (0 for direction parallel with "
							<< "first PCA vector, 1 for normal to the plane)" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	if (indValue.size() != 1 || indValue[0].size() != 1 ) { 
		std::cerr << "PCAPlaneDirection::PCAPlaneDirection() \n"
							<< "First level: Start of cell direction index is used." << std::endl;			
		exit(EXIT_FAILURE);
	}
	
	setId("PCAPlaneDirection");
	setParameter(paraValue);  
	setVariableIndex(indValue);
	
	std::vector<std::string> tmp(numParameter());
	tmp.resize(numParameter());
	tmp[0] = "normal_flag";
	setParameterId(tmp);
}

void PCAPlaneDirection::
initiate(Tissue &T,
				 DataMatrix &cellData,
				 DataMatrix &wallData,
				 DataMatrix &vertexData,
				 DataMatrix &cellDerivs,
				 DataMatrix &wallDerivs,
				 DataMatrix &vertexDerivs)
{
	// No initialization
}

void PCAPlaneDirection::
update(Tissue &T, double h,
			 DataMatrix &cellData,
			 DataMatrix &wallData,
			 DataMatrix &vertexData,
			 DataMatrix &cellDerivs,
			 DataMatrix &wallDerivs,
			 DataMatrix &vertexDerivs)
{
	size_t dimension = vertexData[0].size();
	if (dimension==3) {
		for (size_t n = 0; n < T.numCell(); ++n) {
			Cell cell = T.cell(n);
			
			if (cellData[cell.index()][variableIndex(0, 0) + dimension] == 0) {
				continue;
			}
			
			// This calculation should now be done in reaction CalculatePCAPlane
			//cell.calculatePCAPlane(vertexData);
			DataMatrix axes = cell.getPCAPlane();
			std::vector<double> normal = cell.getNormalToPCAPlane();
			
			double axesNorm=0.0,normalNorm=0.0;
			for (size_t d=0; d<dimension; ++d) {
				axesNorm += axes[0][d]*axes[0][d];
				normalNorm += normal[d]*normal[d];
			}
			assert(axesNorm>0.0);
			assert(normalNorm>0.0);
			axesNorm = 1.0/std::sqrt(axesNorm);
			normalNorm = 1.0/std::sqrt(normalNorm);
			for (size_t d=0; d<dimension; ++d) {
				axes[0][d] *= axesNorm;
				normal[d] *= normalNorm;
			}

			std::vector<double> &dir = axes[0];
			if (parameter(0) == 1) {
				dir = normal;
			}

			for (size_t d=0; d<dimension; ++d)
				cellData[cell.index()][variableIndex(0, 0) + d] = dir[d];
		}		
	}
	else {
		std::cerr << "PCAPlaneDirection::derivs() Only implemented for three dimensions."
							<< std::endl;
		exit(EXIT_FAILURE);
	}
}

VertexStressDirection::
VertexStressDirection(std::vector<double> &paraValue, std::vector< std::vector<size_t> > &indValue)
{
	if (paraValue.size() != 1) {
		std::cerr << "VertexStressDirection::VertexStressDirection() " 
		<< "One parameter is used orientation_flag (0 for direction parallel with "
		<< "stress, 1 for direction perpendicular to stress)" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	if (indValue.size() != 1 || indValue[0].size() != 1) {
		std::cerr << "VertexStressDirection::VertexStressDirection() \n"
		<< "First level: Start of cell direction index are used.\n";
		exit(EXIT_FAILURE);
	}
	
	setId("VertexStressDirection");
	setParameter(paraValue);  
	setVariableIndex(indValue);
	
	std::vector<std::string> tmp(numParameter());
	tmp.resize(numParameter());
	tmp[0] = "orientation_flag";
	setParameterId(tmp);
}
  
void VertexStressDirection::initiate(Tissue &T,
	DataMatrix &cellData,
	DataMatrix &wallData,
	DataMatrix &vertexData,
	DataMatrix &cellDerivs,
	DataMatrix &wallDerivs,
	DataMatrix &vertexDerivs)
{
	// No initialization
}

void VertexStressDirection::update(Tissue &T, double h,
	DataMatrix &cellData,
	DataMatrix &wallData,
	DataMatrix &vertexData,
	DataMatrix &cellDerivs,
	DataMatrix &wallDerivs,
	DataMatrix &vertexDerivs)
{
 	size_t dimensions = vertexData[0].size();

	for (size_t n = 0; n < T.numCell(); ++n) {
		Cell cell = T.cell(n);

		double S = 0.0;
		double C = 0.0;

		// Check direction flag. Do we have a direction?
		if (cellData[cell.index()][variableIndex(0, 0) + dimensions] == 0) {
			continue;
		}

		DataMatrix E = cell.getPCAPlane();
		
		for (size_t i = 0; i < cell.numVertex(); ++i) {
			Vertex *vertex = cell.vertex(i);
			
			std::vector<double> stressDirection = vertex->stressDirection();

			double x = 0.0;
			double y = 0.0;
			double weight = 0.0;

			for (size_t j = 0; j < dimensions; ++j) {
				x += stressDirection[j] * E[0][j];
				y += stressDirection[j] * E[1][j];
			}
			
			double angle = 2.0 * std::atan2(y, x);
			weight = std::sqrt((x * x) + (y * y));

			S += weight * std::sin(angle);
			C += weight * std::cos(angle);
		}

		double direction = 0.5 * std::atan2(S, C);

		if (parameter(0) == 1) {
			direction += 0.5 * myMath::pi();
		}

		for (size_t i = 0; i < dimensions; ++i) {
			cellData[cell.index()][variableIndex(0, 0) + i] = std::cos(direction) * E[0][i] + std::sin(direction) * E[1][i];
		}
	}
}

CellVectorDirection::
CellVectorDirection(std::vector<double> &paraValue, std::vector< std::vector<size_t> > &indValue)
{
  if (paraValue.size() != 1) {
    std::cerr << "CellVectorDirection::CellVectorDirection() " 
	      << "One parameter is used orientation_flag (0 for direction parallel with "
	      << "given cell direction, 1 for perpendicular direction)." << std::endl;
    exit(EXIT_FAILURE);
  }
  
  if (indValue.size() != 2 || indValue[0].size() != 1 || indValue[1].size() != 1) {
    std::cerr << "CellVectorDirection::CellVectorDirection() \n"
	      << "First level: Start of cell direction (to be updated) index is used."
	      << std::endl
	      << "Second level: Start of cell direction index (to be updated from)"
	      << " is used." << std::endl;
    exit(EXIT_FAILURE);
  }
  
  setId("CellVectorDirection");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  std::vector<std::string> tmp(numParameter());
  tmp.resize(numParameter());
  tmp[0] = "orientation_flag";
  setParameterId(tmp);
}
  
void CellVectorDirection::initiate(Tissue &T,
	DataMatrix &cellData,
	DataMatrix &wallData,
	DataMatrix &vertexData,
	DataMatrix &cellDerivs,
	DataMatrix &wallDerivs,
	DataMatrix &vertexDerivs)
{
	// No initialization
}

void CellVectorDirection::update(Tissue &T, double h,
	DataMatrix &cellData,
	DataMatrix &wallData,
	DataMatrix &vertexData,
	DataMatrix &cellDerivs,
	DataMatrix &wallDerivs,
	DataMatrix &vertexDerivs)
{
  if( parameter(0)!=0. ) {
    std::cerr << "CellVectorDirection::update() Only parallell direction (p_0=0)"
	      << " allowed at the moment." << std::endl;
    exit(EXIT_FAILURE);
  }
  size_t dimensions = vertexData[0].size();
  
  for (size_t n = 0; n < T.numCell(); ++n) {
    size_t i = T.cell(n).index();
    for (size_t d = 0; d<dimensions; ++d) {
      cellData[i][variableIndex(0,0)+d] = cellData[i][variableIndex(1,0)+d]; 
    }
  }
}

