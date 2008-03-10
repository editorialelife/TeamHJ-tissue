/**
 * Filename     : baseCompartmentChange.cc
 * Description  : A base class describing variable updates
 * Author(s)    : Henrik Jonsson (henrik@thep.lu.se)
 * Created      : October 2003
 * Revision     : $Id: baseCompartmentChange.cc,v 1.25 2006/03/18 00:05:14 henrik Exp $
 */
#include<vector>

#include"baseCompartmentChange.h"
#include"compartmentDivision.h"
#include"compartmentRemoval.h"

BaseCompartmentChange::~BaseCompartmentChange(){}

//!Factory creator, all creation should be mapped onto this one 
/*! Given the idValue a compartmentChange of the defined type is returned
  (using new Class).*/
BaseCompartmentChange *
BaseCompartmentChange::
createCompartmentChange(std::vector<double> &paraValue,
												std::vector< std::vector<size_t> > &indValue, 
												std::string idValue ) {
  
  //Cell divisions
  //compartmentDivision.h,compartmentDivision.cc
  if(idValue=="DivisionVolumeViaLongestWall")
    return new DivisionVolumeViaLongestWall(paraValue,indValue);
  if(idValue=="DivisionVolumeViaLongestWallSpatial")
    return new DivisionVolumeViaLongestWallSpatial(paraValue,indValue);
  else if(idValue=="DivisionVolumeViaLongestWall3D")
    return new DivisionVolumeViaLongestWall3D(paraValue,indValue);
  else if(idValue=="DivisionVolumeViaLongestWall3DSpatial")
    return new DivisionVolumeViaLongestWall3DSpatial(paraValue,indValue);
	else if(idValue=="DivisionVolumeViaStrain")
    return new DivisionVolumeViaStrain(paraValue,indValue);
	else if(idValue=="DivisionVolumeViaDirection")
    return new DivisionVolumeViaDirection(paraValue,indValue);
	else if(idValue=="DivisionVolumeRandomDirection")
    return new DivisionVolumeRandomDirection(paraValue,indValue);
	else if(idValue=="DivisionVolumeViaShortestPath")
    return new DivisionVolumeViaShortestPath(paraValue,indValue);
  else if (idValue == "DivisionForceDirection")
	  return new DivisionForceDirection(paraValue, indValue);
  else if (idValue == "DivisionShortestPath")
	  return new DivisionShortestPath(paraValue, indValue);
	//compartmentRemoval.h,compartmentRemoval.cc
  else if(idValue=="RemovalOutsideRadius")
    return new RemovalOutsideRadius(paraValue,indValue);
  else if(idValue=="RemovalOutsideRadiusEpidermis")
    return new RemovalOutsideRadiusEpidermis(paraValue,indValue);
  else if(idValue=="RemovalOutsideMaxDistanceEpidermis")
    return new RemovalOutsideMaxDistanceEpidermis(paraValue,indValue);
  else if(idValue=="RemovalOutsidePosition")
    return new RemovalOutsidePosition(paraValue,indValue);

  //Default, if nothing found
  else {
    std::cerr << "\nBaseCompartmentChange::createCompartmentChange()"
							<< " WARNING: CompartmentChangetype " 
							<< idValue << " not known, no compartmentChange created.\n\7";
    exit(-1);
  }
}

//!This creator reads from an open file and then calls for the main creator
BaseCompartmentChange* 
BaseCompartmentChange::createCompartmentChange(std::istream &IN ) {
  
  std::string idVal;
  size_t pNum,levelNum;
  IN >> idVal;
  IN >> pNum;
  IN >> levelNum;
  std::vector<size_t> varIndexNum( levelNum );
  for( size_t i=0 ; i<levelNum ; i++ )
    IN >> varIndexNum[i];
  
  std::vector<double> pVal( pNum );
  for( size_t i=0 ; i<pNum ; i++ )
    IN >> pVal[i];
  
  std::vector< std::vector<size_t> > varIndexVal( levelNum );
  for( size_t i=0 ; i<levelNum ; i++ )
    varIndexVal[i].resize( varIndexNum[i] );
  
  for( size_t i=0 ; i<levelNum ; i++ )
    for( size_t j=0 ; j<varIndexNum[i] ; j++ )
      IN >> varIndexVal[i][j];
  
  return createCompartmentChange(pVal,varIndexVal,idVal);
}

int BaseCompartmentChange::
flag(Tissue *T,size_t i,std::vector< std::vector<double> > &cellData,
     std::vector< std::vector<double> > &walldata,
     std::vector< std::vector<double> > &vertexData,
     std::vector< std::vector<double> > &cellderivs, 
     std::vector< std::vector<double> > &wallderivs,
     std::vector< std::vector<double> > &vertexDerivs ) {
  std::cerr << "BaseCompartmentChange::flag() should not be used. "
						<< "Should always be mapped onto one of the real types.\n";
  exit(0);
}  

void BaseCompartmentChange::
update(Tissue *T,size_t i,std::vector< std::vector<double> > &cellData,
       std::vector< std::vector<double> > &walldata,
       std::vector< std::vector<double> > &vertexData,
       std::vector< std::vector<double> > &cellderivs, 
       std::vector< std::vector<double> > &wallderivs,
       std::vector< std::vector<double> > &vertexDerivs ) {
  std::cerr << "BaseCompartmentChange::update() should not be used. "
						<< "Should always be mapped onto one of the real types.\n";
  exit(0);
}  

void BaseCompartmentChange::
printCellWallError(std::vector< std::vector<double> > &vertexData,
									 Cell *divCell, 
									 std::vector<size_t> &w3Tmp, 
									 size_t &wI, 
									 size_t &w3I,
									 std::vector<double> &point,
									 std::vector<double> &normal,
									 std::ostream &os) {
	
	size_t dimension=vertexData[0].size();
	assert( dimension==2 || dimension==3); 
	// Print all walls
	for (size_t k=0; k<divCell->numWall(); ++k) {
		os << "0 "; 
		for (size_t d=0; d<dimension; ++d)
			os << vertexData[divCell->wall(k)->vertex1()->index()][d] << " ";
		os << std::endl << "0 "; 
		for (size_t d=0; d<dimension; ++d)
			os << vertexData[divCell->wall(k)->vertex2()->index()][d] << " ";
		os << std::endl << std::endl << std::endl;
	}
	// Print potential second walls
	for( size_t kk=0 ; kk<w3Tmp.size() ; ++kk ) {
		size_t k = w3Tmp[kk];
		os << "1 "; 
		for (size_t d=0; d<dimension; ++d)
			os << vertexData[divCell->wall(k)->vertex1()->index()][d] << " ";
		os << std::endl << "1 "; 
		for (size_t d=0; d<dimension; ++d)
			os << vertexData[divCell->wall(k)->vertex2()->index()][d] << " ";
		os << std::endl << std::endl << std::endl;		
	}
	// Print chosen first wall 
	os << "2 "; 
	for (size_t d=0; d<dimension; ++d)
		os << vertexData[divCell->wall(wI)->vertex1()->index()][d] << " ";
	os << std::endl << "2 "; 
	for (size_t d=0; d<dimension; ++d)
		os << vertexData[divCell->wall(wI)->vertex2()->index()][d] << " ";
	os << std::endl << std::endl << std::endl;		
	// Print chosen second wall 
	os << "3 "; 
	for (size_t d=0; d<dimension; ++d)
		os << vertexData[divCell->wall(w3I)->vertex1()->index()][d] << " ";
	os << std::endl << "3 "; 
	for (size_t d=0; d<dimension; ++d)
		os << vertexData[divCell->wall(w3I)->vertex2()->index()][d] << " ";
	os << std::endl << std::endl << std::endl;			
	// Print wall between the two chosen walls
	os << "4 "; 
	for (size_t d=0; d<dimension; ++d)
		os << 0.5*(vertexData[divCell->wall(wI)->vertex1()->index()][d]+
							 vertexData[divCell->wall(wI)->vertex2()->index()][d])
			 << " "; 
	os << std::endl << "4 "; 
	for (size_t d=0; d<dimension; ++d)
		os << 0.5*(vertexData[divCell->wall(w3I)->vertex1()->index()][d]+
							 vertexData[divCell->wall(w3I)->vertex2()->index()][d])
			 << " "; 
	os << std::endl << std::endl << std::endl;		
	// Print direction from the initial point
	os << "5 "; 
	for (size_t d=0; d<dimension; ++d)
		os << point[d] << " ";
	os << std::endl << "5 "; 
	for (size_t d=0; d<dimension; ++d)
		os << point[d]+normal[d] << " ";
	os << std::endl << std::endl << std::endl;			
}

int BaseCompartmentChange::
findTwoDivisionWalls(std::vector< std::vector<double> > &vertexData, 
										 Cell *divCell, std::vector<size_t> &wI,
										 std::vector<double> &point, 
										 std::vector<double> &n, 
										 std::vector<double> &v1Pos, 
										 std::vector<double> &v2Pos)
{
	size_t dimension=vertexData[0].size();
	size_t cellI=divCell->index();
	std::vector<double> s(2);
  wI[0]=0;
	wI[1]=divCell->numWall();
	s[0]=s[1]=-1.0;
	
	std::vector<size_t> w3Tmp;
	std::vector<double> w3tTmp;
  int flag=0, vertexFlag=0;
	if (dimension==2) {
		for( size_t k=0 ; k<divCell->numWall() ; ++k ) {
			size_t v1Tmp = divCell->wall(k)->vertex1()->index();
			size_t v2Tmp = divCell->wall(k)->vertex2()->index();
			std::vector<double> w3(dimension),w0(dimension);
			for( size_t dim=0 ; dim<dimension ; ++dim ) {
				w3[dim] = vertexData[v2Tmp][dim]-vertexData[v1Tmp][dim];
				w0[dim] = point[dim]-vertexData[v1Tmp][dim];
			}
			double a=0.0,b=0.0,c=0.0,d=0.0,e=0.0;//a=1.0
			for( size_t dim=0 ; dim<dimension ; ++dim ) {
				a += n[dim]*n[dim];
				b += n[dim]*w3[dim];
				c += w3[dim]*w3[dim];
				d += n[dim]*w0[dim];
				e += w3[dim]*w0[dim];
			}
			double fac=a*c-b*b;//a*c-b*b
			if( fac>1e-10 ) {//else parallell and not applicable
				fac = 1.0/fac;
				//double s = fac*(b*e-c*d);
				double t = fac*(a*e-b*d);//fac*(a*e-b*d)
				if( t>0.0 && t<=1.0 ) {//within wall
					//double dx0 = w0[0] +fac*((b*e-c*d)*nW2[0]+()*w3[0]); 					
					w3Tmp.push_back(k);
					w3tTmp.push_back(t);
					if( flag<2 ) {
						s[flag] = t;
						wI[flag] = k;
					}				
					++flag;
					if (t==1)
						++vertexFlag;
				}
			}		
		}
	}
	else if (dimension==3) {
		for( size_t k=0 ; k<divCell->numWall() ; ++k ) {
			size_t v1w3Itmp = divCell->wall(k)->vertex1()->index();
			size_t v2w3Itmp = divCell->wall(k)->vertex2()->index();
			std::vector<double> w3(dimension),w0(dimension);
			double fac1=0.0,fac2=0.0;
			for( size_t d=0 ; d<dimension ; ++d ) {
				w3[d] = vertexData[v2w3Itmp][d]-vertexData[v1w3Itmp][d];
				fac1 += n[d]*(point[d]-vertexData[v1w3Itmp][d]);
				fac2 += n[d]*w3[d]; 
			}
			if( fac2 != 0.0 ) {//else parallell and not applicable
				double t = fac1/fac2;
				//std::cerr << k << "(" << divCell->numWall() << ")" << t << " " 
				//				<< fac1 << "/" << fac2 << std::endl;
				if( t>0.0 && t<=1.0 ) {//within wall
					w3Tmp.push_back(k);
					w3tTmp.push_back(t);
					if (flag<2) {
						s[flag] = t;
						wI[flag] = k;
					}
					++flag;
					if (t==1.0)
						++vertexFlag;
				}
			}
		}
	}

	if ( wI[1] == divCell->numWall() || wI[0]==wI[1]) {
		std::cerr << "BaseCompartmentChange::findTwoDivisionWalls:"
							<< "Two correct walls not found for division." << std::endl
							<< flag << " walls proposed (" << vertexFlag << " vertex)" << std::endl
							<< "w[0]=" << wI[0] << " w[1]=" << wI[1] << std::endl;
		exit(-1);
	}
	assert( wI[1] != divCell->numWall() && wI[0] != wI[1] );
 
	if( flag != 2 && !(flag==3 && vertexFlag) ) {
		std::cerr << "BaseCompartmentChange::findTwoDivisionWalls Warning:"
							<< " not two, but " << flag << " walls chosen as "
							<< "connection for cell " 
							<< cellI << std::endl; 
		printCellWallError(vertexData,divCell,w3Tmp,wI[0],wI[1],point,n);
		return -1;
	}	
	
	//Addition of new vertices at walls at position 's' 
  size_t v1I = divCell->wall(wI[0])->vertex1()->index();
  size_t v2I = divCell->wall(wI[0])->vertex2()->index();
  for( size_t d=0 ; d<dimension ; ++d )
    v1Pos[d] = vertexData[v1I][d]+ s[0]*(vertexData[v2I][d]-vertexData[v1I][d]);
  v1I = divCell->wall(wI[1])->vertex1()->index();
  v2I = divCell->wall(wI[1])->vertex2()->index();
  for( size_t d=0 ; d<dimension ; ++d )
    v2Pos[d] = vertexData[v1I][d]+s[1]*(vertexData[v2I][d]-vertexData[v1I][d]);
	
	return 0;
}

int BaseCompartmentChange::
findSecondDivisionWall(std::vector< std::vector<double> > &vertexData, 
											 Cell *divCell, size_t &wI, size_t &w3I, 
											 std::vector<double> &v1Pos, 
											 std::vector<double> &n, 
											 std::vector<double> &v2Pos)
{	
	size_t dimension=vertexData[0].size();
	w3I=divCell->numWall();
	//double minDist,w3s;
	std::vector<size_t> w3Tmp;
	std::vector<double> w3tTmp;
	int flag=0,vertexFlag=0;
	
	if (dimension==2) {
		for (size_t k=0; k<divCell->numWall(); ++k) {
			if (k!=wI) {
				size_t v1w3Itmp = divCell->wall(k)->vertex1()->index();
				size_t v2w3Itmp = divCell->wall(k)->vertex2()->index();
				std::vector<double> w3(dimension),w0(dimension);
				for (size_t d=0; d<dimension; ++d) {
					w3[d] = vertexData[v2w3Itmp][d]-vertexData[v1w3Itmp][d];
					w0[d] = v1Pos[d]-vertexData[v1w3Itmp][d];
				}
				double a=0.0,b=0.0,c=0.0,d=0.0,e=0.0;//a=1.0
				for (size_t dim=0; dim<dimension; ++dim) {
					a += n[dim]*n[dim];
					b += n[dim]*w3[dim];
					c += w3[dim]*w3[dim];
					d += n[dim]*w0[dim];
					e += w3[dim]*w0[dim];
				}
				double fac=a*c-b*b;//a*c-b*b
				if (fac>1e-10) {//else parallell and not applicable
					fac = 1.0/fac;
					//double s = fac*(b*e-c*d);
					double t = fac*(a*e-b*d);//fac*(a*e-b*d)
					if (t>0.0 && t<=1.0) {//within wall
						//double dx0 = w0[0] +fac*((b*e-c*d)*n[0]+()*w3[0]); 					
						++flag;
						if (t==1.0)
							++vertexFlag;
						w3I = k;
						w3Tmp.push_back(k);
						w3tTmp.push_back(t);
					}
				}
			}
		}
	}//if (dimension==2)
	else if (dimension==3) {
		for( size_t k=0 ; k<divCell->numWall() ; ++k ) {
			if( k!=wI ) {
				size_t v1w3Itmp = divCell->wall(k)->vertex1()->index();
				size_t v2w3Itmp = divCell->wall(k)->vertex2()->index();
				std::vector<double> w3(dimension),w0(dimension);
				double fac1=0.0,fac2=0.0;
				for( size_t d=0 ; d<dimension ; ++d ) {
					w3[d] = vertexData[v2w3Itmp][d]-vertexData[v1w3Itmp][d];
					fac1 += n[d]*(v1Pos[d]-vertexData[v1w3Itmp][d]);
					fac2 += n[d]*w3[d]; 
				}
				if( fac2 != 0.0 ) {//else parallell and not applicable
					double t = fac1/fac2;
					if( t>0.0 && t<=1.0 ) {//within wall
						//std::cerr << "wall " << k << " (t=" << t << " " << fac1 << "/"
						//				<< fac2 
						//				<< ") chosen as second wall"
						//				<< std::endl;					
						++flag;
						if (t==1.0)
							++vertexFlag;
						w3I = k;
						w3Tmp.push_back(k);
						w3tTmp.push_back(t);
					}
				}
			}
		}
	}//if (dimension==3)	
	//
	// Check that division consistent
	//
	assert( w3I != divCell->numWall() && w3I != wI );
	if( flag != 1 && !(flag==2 && vertexFlag) ) {
		std::cerr << "baseCompartmentChange::findSecondDivisionWall Warning"
							<< " more than one wall possible as connection "
							<< "for cell " << divCell->index() << " (" << dimension << "dim)" << std::endl 
							<< flag << " possible walls found and " << vertexFlag 
							<< " marked as vertices." << std::endl; 
		printCellWallError(vertexData, divCell, w3Tmp, wI, w3I,v1Pos,n);
		return -1;
	}	
	//
	// Set the second vertex position using the collected t
	//
	size_t v1w3I = divCell->wall(w3I)->vertex1()->index();
	size_t v2w3I = divCell->wall(w3I)->vertex2()->index();
	for( size_t d=0 ; d<dimension ; ++d )
		v2Pos[d] = vertexData[v1w3I][d] + 
			w3tTmp[w3tTmp.size()-1]*(vertexData[v2w3I][d]-vertexData[v1w3I][d]);		
	return 0;
}
