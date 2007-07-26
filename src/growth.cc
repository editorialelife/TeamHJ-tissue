/**
 * Filename     : growth.cc
 * Description  : Classes describing growth updates
 * Author(s)    : Henrik Jonsson (henrik@thep.lu.se)
 * Created      : April 2006
 * Revision     : $Id:$
 */
#include"growth.h"
#include"baseReaction.h"

//!Constructor
WallGrowthExponentialTruncated::
WallGrowthExponentialTruncated(std::vector<double> &paraValue, 
			       std::vector< std::vector<size_t> > 
			       &indValue ) {
  
  //Do some checks on the parameters and variable indeces
  //////////////////////////////////////////////////////////////////////
  if( paraValue.size()!=2 ) {
    std::cerr << "WallGrowthExponentialTruncated::"
	      << "WallGrowthExponentialTruncated() "
	      << "Two parameters used  k_growth and L_trunc\n";
    exit(0);
  }
  if( indValue.size() != 1 || indValue[0].size() != 1 ) {
    std::cerr << "WallGrowthExponentialTruncated::"
	      << "WallGrowthExponentialTruncated() "
	      << "One variable index is used.\n";
    exit(0);
  }
  //Set the variable values
  //////////////////////////////////////////////////////////////////////
  setId("WallGrowthExponentialTruncated");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  //Set the parameter identities
  //////////////////////////////////////////////////////////////////////
  std::vector<std::string> tmp( numParameter() );
  tmp.resize( numParameter() );
  tmp[0] = "k_growth";
  tmp[1] = "L_trunc";
  setParameterId( tmp );
}

//! Derivative contribution for the growth
/*! Deriving the time derivative contribution for the growth for all
  walls in the tissue.
*/
void WallGrowthExponentialTruncated::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  
  size_t numWalls = T.numWall();
  size_t lengthIndex = variableIndex(0,0);
  
  for( size_t i=0 ; i<numWalls ; ++i )
    wallDerivs[i][lengthIndex] += parameter(0)*
      wallData[i][lengthIndex]*(1-wallData[i][lengthIndex]/parameter(1));
}

//!Constructor
WallGrowthExponentialStressTruncated::
WallGrowthExponentialStressTruncated(std::vector<double> &paraValue, 
			       std::vector< std::vector<size_t> > 
			       &indValue ) {
  
  //Do some checks on the parameters and variable indeces
  //////////////////////////////////////////////////////////////////////
  if( paraValue.size()!=2 ) {
    std::cerr << "WallGrowthExponentialStressTruncated::"
	      << "WallGrowthExponentialStressTruncated() "
	      << "Uses two parameters k_growth and L_trunc\n";
    exit(0);
  }
  if( indValue.size() != 1 || indValue[0].size() != 1 ) {
    std::cerr << "WallGrowthExponentialStressTruncated::"
	      << "WallGrowthExponentialStressTruncated() "
	      << "One variable index is used.\n";
    exit(0);
  }
  //Set the variable values
  //////////////////////////////////////////////////////////////////////
  setId("WallGrowthExponentialStressTruncated");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  //Set the parameter identities
  //////////////////////////////////////////////////////////////////////
  std::vector<std::string> tmp( numParameter() );
  tmp.resize( numParameter() );
  tmp[0] = "k_growth";
  tmp[1] = "L_trunc";
  setParameterId( tmp );
}

//! Derivative contribution for the growth
/*! Deriving the time derivative contribution for the growth for all
  walls in the tissue.
*/
void WallGrowthExponentialStressTruncated::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  
  size_t numWalls = T.numWall();
  size_t lengthIndex = variableIndex(0,0);
  
  for( size_t i=0 ; i<numWalls ; ++i ) {
    size_t v1 = T.wall(i).vertex1()->index();
    size_t v2 = T.wall(i).vertex2()->index();
    double distance=0.0;
    for( size_t d=0 ; d<vertexData[v1].size() ; d++ )
      distance += (vertexData[v1][d]-vertexData[v2][d])*
	(vertexData[v1][d]-vertexData[v2][d]);
    distance = std::sqrt(distance);
    if( distance>wallData[i][lengthIndex] )
      wallDerivs[i][lengthIndex] += parameter(0)*
	(distance-wallData[i][lengthIndex])*
	wallData[i][lengthIndex]*(1-wallData[i][lengthIndex]/parameter(1));
  }
}

//!Constructor
WallGrowthConstantStress::
WallGrowthConstantStress(std::vector<double> &paraValue, 
			       std::vector< std::vector<size_t> > 
			       &indValue ) {
  
  //Do some checks on the parameters and variable indeces
  //////////////////////////////////////////////////////////////////////
  if( paraValue.size()!=4 ) {
    std::cerr << "WallGrowthConstantStress::"
							<< "WallGrowthConstantStress() "
							<< "Uses four parameters k_growth, stretch_threshold "
							<< "stretch_flag and linear_flag" << std::endl;
    exit(0);
  }
	if( paraValue[2] != 0.0 && paraValue[2] != 1.0 ) {
    std::cerr << "WallGrowthConstantStress::"
							<< "WallGrowthConstantStress() "
							<< "stretch_flag parameter must be 0 (stress used) or " 
							<< "1 (stretch used)." << std::endl;
    exit(0);
  }
	if( paraValue[3] != 0.0 && paraValue[3] != 1.0 ) {
    std::cerr << "WallGrowthConstantStress::"
							<< "WallGrowthConstantStress() "
							<< "linear_flag parameter must be 0 (constant growth) or " 
							<< "1 (length dependent growth)." << std::endl;
    exit(0);
  }
	
  if( indValue.size() != 2 || indValue[0].size() != 1 ) {
    std::cerr << "WallGrowthConstantStress::"
							<< "WallGrowthConstantStress() "
							<< "One variable index is used (wall length) at first "
							<< "level, and spring constant variable indices at second"
							<< std::endl;
    exit(0);
  }
  //Set the variable values
  //////////////////////////////////////////////////////////////////////
  setId("WallGrowthConstantStress");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  //Set the parameter identities
  //////////////////////////////////////////////////////////////////////
  std::vector<std::string> tmp( numParameter() );
  tmp.resize( numParameter() );
  tmp[0] = "k_growth";
	tmp[1] = "stress_threshold";
	tmp[2] = "stretch_flag";
	tmp[3] = "linear_flag";
  setParameterId( tmp );
}

//! Derivative contribution for the growth
/*! Deriving the time derivative contribution for the growth for all
  walls in the tissue.
*/
void WallGrowthConstantStress::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  
  size_t numWalls = T.numWall();
  size_t lengthIndex = variableIndex(0,0);
  
  for( size_t i=0 ; i<numWalls ; ++i ) {
    size_t v1 = T.wall(i).vertex1()->index();
    size_t v2 = T.wall(i).vertex2()->index();
		double stress=0.0;
		if (!parameter(2)) {
			for (size_t k=0; k<numVariableIndex(1); ++k)
				stress += wallData[i][variableIndex(1,k)];
		}
		else {
			double distance=0.0;
			for( size_t d=0 ; d<vertexData[v1].size() ; d++ )
				distance += (vertexData[v1][d]-vertexData[v2][d])*
					(vertexData[v1][d]-vertexData[v2][d]);
			distance = std::sqrt(distance);
			stress = (distance-wallData[i][lengthIndex]) /
				wallData[i][lengthIndex];
		}
    if (stress > parameter(1)) {
			double growthRate = parameter(0)*(stress - parameter(1));
			if (parameter(3))
				growthRate *= wallData[i][lengthIndex];
      wallDerivs[i][lengthIndex] += growthRate;
		}
	}
}

//!Constructor
WallGrowthConstantStressEpidermalAsymmetric::
WallGrowthConstantStressEpidermalAsymmetric(std::vector<double> &paraValue, 
			       std::vector< std::vector<size_t> > 
			       &indValue ) {
  
  //Do some checks on the parameters and variable indeces
  //////////////////////////////////////////////////////////////////////
  if( paraValue.size()!=2 ) {
    std::cerr << "WallGrowthConstantStressEpidermalAsymmetric::"
	      << "WallGrowthConstantStressEpidermalAsymmetric() "
	      << "Uses two parameters k_growth and f_epi\n";
    exit(0);
  }  
  if( indValue.size() != 1 || indValue[0].size() != 1 ) {
    std::cerr << "WallGrowthConstantStressEpidermalAsymmetric::"
	      << "WallGrowthConstantStressEpidermalAsymmetric() "
	      << "One variable index is used.\n";
    exit(0);
  }
  //Set the variable values
  //////////////////////////////////////////////////////////////////////
  setId("WallGrowthConstantStressEpidermalAsymmetric");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  //Set the parameter identities
  //////////////////////////////////////////////////////////////////////
  std::vector<std::string> tmp( numParameter() );
  tmp.resize( numParameter() );
  tmp[0] = "k_growth";
  tmp[0] = "f_epi";
  setParameterId( tmp );
}

//! Derivative contribution for the growth
/*! Deriving the time derivative contribution for the growth for all
  walls in the tissue.
*/
void WallGrowthConstantStressEpidermalAsymmetric::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  
  size_t numWalls = T.numWall();
  size_t lengthIndex = variableIndex(0,0);
  
  for( size_t i=0 ; i<numWalls ; ++i ) {
    size_t v1 = T.wall(i).vertex1()->index();
    size_t v2 = T.wall(i).vertex2()->index();
    double kGrowth = parameter(0);
    if( T.wall(i).cell1() == T.background() || 
	T.wall(i).cell2() == T.background() )
      kGrowth *= parameter(1);
    double distance=0.0;
    for( size_t d=0 ; d<vertexData[v1].size() ; d++ )
      distance += (vertexData[v1][d]-vertexData[v2][d])*
	(vertexData[v1][d]-vertexData[v2][d]);
    distance = std::sqrt(distance);
    if( distance>wallData[i][lengthIndex] )
      wallDerivs[i][lengthIndex] += kGrowth*
	(distance-wallData[i][lengthIndex]);
  }
}

//!Constructor
MoveVertexRadially::
MoveVertexRadially(std::vector<double> &paraValue, 
			       std::vector< std::vector<size_t> > 
			       &indValue ) {
  
  //Do some checks on the parameters and variable indeces
  //////////////////////////////////////////////////////////////////////
  if( paraValue.size()!=2 || ( paraValue[1]!=0 && paraValue[1]!=1) ) {
    std::cerr << "MoveVertexRadially::"
							<< "MoveVertexRadially() "
							<< "Uses two parameters k_growth and r_pow (0,1)\n";
    exit(0);
  }  
  if( indValue.size() != 0 ) {
    std::cerr << "MoveVertexRadially::"
							<< "MoveVertexRadially() "
							<< "No variable index is used.\n";
    exit(0);
  }
  //Set the variable values
  //////////////////////////////////////////////////////////////////////
  setId("MoveVertexRadially");
  setParameter(paraValue);  
  setVariableIndex(indValue);
  
  //Set the parameter identities
  //////////////////////////////////////////////////////////////////////
  std::vector<std::string> tmp( numParameter() );
  tmp.resize( numParameter() );
  tmp[0] = "k_growth";
  tmp[0] = "r_pow";
  setParameterId( tmp );
}

//! Derivative contribution for the growth
/*! Deriving the time derivative contribution for the growth for all
  walls in the tissue.
*/
void MoveVertexRadially::
derivs(Tissue &T,
       std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &wallData,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellDerivs,
       std::vector< std::vector<double> > &wallDerivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  
  size_t numVertices = T.numVertex();
  size_t dimension=vertexData[0].size();

  for( size_t i=0 ; i<numVertices ; ++i ) {
		double fac=parameter(0);
		if( parameter(1)==0.0 ) {
			double r=0.0;
			for( size_t d=0 ; d<dimension ; ++d )
				r += vertexData[i][d]*vertexData[i][d];
			if( r>0.0 )
				r = std::sqrt(r);
			if( r>0.0 )
				fac /= r;
			else
				fac=0.0;
		}
		for( size_t d=0 ; d<dimension ; ++d )
			vertexDerivs[i][d] += fac*vertexData[i][d];
	}
}

WallLengthGrowExperimental::WallLengthGrowExperimental(std::vector<double> &paraValue,
                                                       std::vector< std::vector<size_t> > &indValue)
{
     if (paraValue.size() != 2) {
		std::cerr << "WallLengthGrowExperimental::WallLengthGrowExperimental() "
                    << "Uses two parameters: k_L and phi" << std::endl;
          exit(EXIT_FAILURE);
     }

     if (indValue.size() != 2 || indValue[0].size() != 1) {
		std::cerr << "WallLengthGrowExperimental::WallLengthGrowExperimental() "
                    << "Wall length index must be given in first level.\n"
                    << "Wall force indices must be given in second level.\n";
          exit(EXIT_FAILURE);
     }

     setId("VertexFromWallSpringExperimental");
     setParameter(paraValue);
     setVariableIndex(indValue);

	std::vector<std::string> tmp(numParameter());
     tmp[0] = "k_L";
     tmp[1] = "phi";

     setParameterId(tmp);
}

void WallLengthGrowExperimental::
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
		
		double F = 0.0;
		for (size_t j = 0; j < numVariableIndex(1); ++j)
			F += wallData[T.wall(i).index()][variableIndex(1, j)];
		
		double arg = F - parameter(1);
		if (arg > 0)
			wallDerivs[T.wall(i).index()][variableIndex(0, 0)] 
				+= parameter(0) * arg; 
		//*wallData[T.wall(i).index()][variableIndex(0, 0)];
	}
}

WaterVolumeFromTurgor::
WaterVolumeFromTurgor(std::vector<double> &paraValue,
											std::vector< std::vector<size_t> > &indValue)
{
	if (paraValue.size() != 5) {
		std::cerr << "WaterVolumeFromTurgor::WaterVolumeFromTurgor() "
							<< "Uses five parameters: k_p, P_max, k_pp and "
							<< "denyShrink_flag allowNegTurgor_flag." << std::endl;
		exit(EXIT_FAILURE);
	}
	
	if (indValue.size() < 1 || indValue.size() > 2
			|| indValue[0].size() != 1 
			|| (indValue.size()==2 && indValue[1].size() != 1 ) ) {
		std::cerr << "WaterVolumeFromTurgor::WaterVolumeFromTurgor() "
							<< "Water volume index must be given in "
							<< "first level.\n"
							<< "Optionally index for saving the turgor pressure can be"
							<< " given at second level." << std::endl; 		
		exit(EXIT_FAILURE);
	}
	
	setId("WaterVolumeFromTurgor");
	setParameter(paraValue);
	setVariableIndex(indValue);
	
	std::vector<std::string> tmp(numParameter());
	tmp[0] = "k_p";
	tmp[1] = "P_max";
	tmp[2] = "k_pp";
	tmp[3] = "denyShrink_flag";
	tmp[4] = "allowNegTurgor_flag";
	setParameterId(tmp);
}

void WaterVolumeFromTurgor::
derivs(Tissue &T,
			 std::vector< std::vector<double> > &cellData,
			 std::vector< std::vector<double> > &wallData,
			 std::vector< std::vector<double> > &vertexData,
			 std::vector< std::vector<double> > &cellDerivs,
			 std::vector< std::vector<double> > &wallDerivs,
			 std::vector< std::vector<double> > &vertexDerivs)
{
	for (size_t n = 0; n < T.numCell(); ++n) {
		Cell cell = T.cell(n);
		
		double P = 0.0;
		double totalLength = 0.0;
		for (size_t i = 0; i < cell.numWall(); ++i) {
			size_t vertex1Index = cell.wall(i)->vertex1()->index();
			size_t vertex2Index = cell.wall(i)->vertex2()->index();
			size_t dimensions = vertexData[vertex1Index].size();
			
			double distance = 0.0;
			for (size_t d = 0; d < dimensions; ++d) {
				distance += (vertexData[vertex1Index][d] - vertexData[vertex2Index][d])
					* (vertexData[vertex1Index][d] - vertexData[vertex2Index][d]);
			}
			distance = std::sqrt(distance);
			totalLength += distance;
			
			// Old turgor measure from wall extensions
			//for (size_t j = 0; j < numVariableIndex(1); ++j)
			//P += wallData[cell.wall(i)->index()][variableIndex(1, j)]/distance;
		}
		
		// Calculate turgor measure from volume and 'water volume'
		// P ~ p_2(V_w-V)
		double cellVolume =cell.calculateVolume(vertexData);
		P = (cellData[cell.index()][variableIndex(0,0)]-cellVolume) /
			cellVolume;
		if (P<0.0 && !parameter(4))
			P=0.0;
		
		P *= parameter(2);
		
		if (numVariableIndexLevel()==2)
			cellData[n][variableIndex(1,0)]=P;
		
		if( !parameter(3) || parameter(1)-P>0.0 )
			cellDerivs[cell.index()][variableIndex(0,0)] += 
				parameter(0) * (parameter(1) - P) * totalLength;
	}
}
