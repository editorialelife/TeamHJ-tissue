/**
 * Filename     : tissue.cc
 * Description  : A class describing a two-dimensional cell tissue
 * Author(s)    : Henrik Jonsson (henrik at thep.lu.se)
 * Created      : April 2006
 * Revision     : $Id:$
 */

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "tissue.h"
#include "wall.h"
#include "myFiles.h"

Tissue::Tissue() {  
  cell_.reserve(10000);
  wall_.reserve(10000);
  vertex_.reserve(10000);
  Cell tmpCell(static_cast<size_t>(-1),static_cast<std::string>("Background"));
  background_ = tmpCell;
}

Tissue::Tissue( const Tissue & tissueCopy ) {
  cell_.reserve(10000);
  wall_.reserve(10000);
  vertex_.reserve(10000);
  Cell tmpCell(static_cast<size_t>(-1),static_cast<std::string>("Background"));
  background_ = tmpCell;
}

Tissue::Tissue( const std::vector<Cell> &cellVal,
		const std::vector<Wall> &wallVal,
		const std::vector<Vertex> &vertexVal ) {
  
  cell_.reserve(10000);
  wall_.reserve(10000);
  vertex_.reserve(10000);

  Cell tmpCell(static_cast<size_t>(-1),static_cast<std::string>("Background"));
  background_ = tmpCell;
  cell_ = cellVal;
  wall_ = wallVal;
  vertex_ = vertexVal;
}

Tissue::Tissue( const char *initFile, int verbose ) {
  cell_.reserve(10000);
  wall_.reserve(10000);
  vertex_.reserve(10000);

  Cell tmpCell(static_cast<size_t>(-1),static_cast<std::string>("Background"));
  background_ = tmpCell;
  readInit(initFile,verbose);
}


Tissue::Tissue( std::string initFile, int verbose ) 
{
  cell_.reserve(10000);
  wall_.reserve(10000);
  vertex_.reserve(10000);
	
  Cell tmpCell(static_cast<size_t>(-1),static_cast<std::string>("Background"));
  background_ = tmpCell;
  readInit(initFile,verbose);
}

Tissue::Tissue( std::vector< std::vector<double> > &cellData,
								std::vector< std::vector<double> > &wallData,
								std::vector< std::vector<double> > &vertexData,
								std::vector< std::vector<size_t> > &cellVertex,
								std::vector< std::vector<size_t> > &wallVertex,
								int verbose)
{
  cell_.reserve(10000);
  wall_.reserve(10000);
  vertex_.reserve(10000);
	
  Cell tmpCell(static_cast<size_t>(-1),static_cast<std::string>("Background"));
  background_ = tmpCell;
	
	size_t numCell = cellData.size();
	size_t numWall = wallData.size();
	size_t numVertex = vertexData.size();
  setNumCell( numCell );
  setNumWall( numWall );
  setNumVertex( numVertex );
	assert( numCell );
  assert( numWall );
  assert( numVertex );
  
  //Set all indeces to the placement in the vectors
  for( size_t i=0 ; i<numCell ; ++i )
    cell(i).setIndex(i);
  for( size_t i=0 ; i<numWall ; ++i )
    wall(i).setIndex(i);
  for( size_t i=0 ; i<numVertex ; ++i )
    vertex(i).setIndex(i);
  //
  // set cell variables
  //
	size_t numCellVar = cellData[0].size();
  if( numCellVar ) {
    for( size_t i=0 ; i<numCell ; ++i ) {
			if (cellData[i].size() != numCellVar) {
				std::cerr << "Tissue::Tissue(cellData,wallData,vertexData,cellVertex,wallVertex) "
									<< "Wrong number of variables in cell " << i << std::endl;
				exit(-1);
			}
      for( size_t j=0 ; j<numCellVar ; ++j ) {
				cell(i).addVariable(cellData[i][j]);
      }
    }
	}
	//
  // Set wall data
  //
	if (wallData[0].size() < 1) {
		std::cerr << "Tissue::Tissue(cellData,wallData,vertexData,cellVertex,wallVertex) "
							<< " At least wall length must be given in wall variables." << std::endl; 
		exit(-1);
	}
	size_t numWallVar=wallData.size();
	for (size_t i = 0; i < numWall; ++i) {
		if (wallData[i].size() != numWallVar) {
			std::cerr << "Tissue::Tissue(cellData,wallData,vertexData,cellVertex,wallVertex) "
								<< "Wrong number of variables in wall " << i << std::endl;
			exit(-1);
		}
		wall(i).setLength(wallData[i][0]);
		for (size_t j=1; j<numWallVar; ++j) {
			wall(i).addVariable(wallData[i][j]);
		}
	}
	//
  //Set vertex positions
  //
  size_t dimension = vertexData[0].size();
	assert( dimension==2 || dimension==3 );
  for( size_t i=0 ; i<numVertex ; ++i ) {
		if (vertexData[i].size() != dimension) {
			std::cerr << "Tissue::Tissue(cellData,wallData,vertexData,cellVertex,wallVertex) "
								<< "Wrong dimension in vertex " << i << std::endl;
			exit(-1);
		}
    vertex(i).setPosition(vertexData[i]);
  }
	//
	// Set connectivity
  //
	// Cell-Vertex
	//
	if (numCell!=cellVertex.size()) {
		std::cerr << "Tissue::Tissue(cellData,wallData,vertexData,cellVertex,wallVertex) "
							<< "Cell number in cell variables not same as in cell vertex." << std::endl;
		exit(-1);
	}
	for (size_t i=0; i<numCell; ++i) {
		size_t numCellVertex=cellVertex.size();
		for (size_t k=0; k<numCellVertex; ++k) {
			size_t j=cellVertex[i][k];
			cell(i).addVertex( vertexP(j) );
			vertex(j).addCell( cellP(i) );
		}
	}
  //
	// Wall-Vertex
	//
	if (numWall!=wallVertex.size()) {
		std::cerr << "Tissue::Tissue(cellData,wallData,vertexData,cellVertex,wallVertex) "
							<< "Wall number in wall variables not same as in wall vertex." << std::endl;
		exit(-1);
	}
	for (size_t i=0; i<numWall; ++i) {
		assert (wallVertex.size()==2);
		size_t j1=wallVertex[i][0];
		size_t j2=wallVertex[i][1];
		vertex(j1).addWall( wallP(i) );
		vertex(j2).addWall( wallP(i) );
		wall(i).setVertex( vertexP(j1),vertexP(j2) );
	}
	//
	// Cell-Wall
	//
	// For each wall, find one or two cells that have the same vertex pair. 
	// If two cells connect wall with these, and if one connect wall with this and background.
	//
	for (size_t wI=0; wI<numWall; ++wI) {
		size_t cellCount=0;
		std::vector<size_t> cI(2,numCell);
		size_t vI1=wallVertex[wI][0];
		size_t vI2=wallVertex[wI][1];
		for (size_t i=0; i<numCell; ++i) {
			size_t cellVertexCount=0;
			for (size_t k=0; k<cellVertex[i].size(); ++k) {
				if (cellVertex[i][k]==vI1 || cellVertex[i][k]==vI2) {
					++cellVertexCount;
				}
			}
			if (cellVertexCount==2) {//cell and wall connected
				if (cellCount>1) {
					std::cerr << "Tissue::Tissue(cellData,wallData,vertexData,cellVertex,wallVertex) "
										<< " More than two cells found to wall " << wI << std::endl;
					exit(-1);
				}
				cell(i).addWall(wallP(wI));
				cI[cellCount] = i;
				++cellCount;
			}
		}
		if (cellCount==1) {
			wall(wI).setCell( cellP(cI[0]),background() );
		}
		else if (cellCount==2) {
			wall(wI).setCell( cellP(cI[0]),cellP(cI[1]) );
		}
		else {
			std::cerr << "Tissue::Tissue(cellData,wallData,vertexData,cellVertex,wallVertex) "
								<< " Wrong number of cells found to wall " << wI << std::endl;
			exit(-1);
		}		
	}

	//Sort all cellWalls and cellVertices to comply with area calculations
	//and plotting
	sortCellWallAndCellVertex();
	checkConnectivity(verbose);
}


Tissue::~Tissue() {
}

//!Sets all wall length variables from the two vertices positions
void Tissue::setWallLengthFromVertexPosition() {
  for( size_t i=0 ; i<numWall() ; ++i )
    wall(i).setLengthFromVertexPosition();
}

//!Adds a reaction to the list from an open file
int Tissue::addReaction( std::istream &IN ) {
  if( !IN ) return -1;
  reaction_.push_back( BaseReaction::createReaction(IN) );
  return 0;
}

//!Adds a compartmentChange to the list from an open file
int Tissue::addCompartmentChange( std::istream &IN ) {
  if( !IN ) return -1;
  compartmentChange_.push_back( BaseCompartmentChange::createCompartmentChange(IN) );
  return 0;
}

//!Reads a tissue from an open file
void Tissue::readInit(std::ifstream &IN,int verbose) {
  
  unsigned int numCellVal,numWallVal,numVertexVal;
  //std::string idVal;
  
  //IN >> idVal;
  IN >> numCellVal;
  IN >> numWallVal;
  IN >> numVertexVal;

  //setId( idVal );
  setNumCell( numCellVal );
  setNumWall( numWallVal );
  setNumVertex( numVertexVal );

  assert( numCellVal );
  assert( numWallVal );
  assert( numVertexVal );
  
  //Set all indeces to the placement in the vectors
  for( size_t i=0 ; i<numCellVal ; ++i )
    cell(i).setIndex(i);
  for( size_t i=0 ; i<numWallVal ; ++i )
    wall(i).setIndex(i);
  for( size_t i=0 ; i<numVertexVal ; ++i )
    vertex(i).setIndex(i);
  
  //Read connective topology
  //////////////////////////////////////////////////////////////////////
  if( verbose )
    std::cerr << "Tissue::readInit(IN) - reading connectivity topology" << std::endl;
  int wI,c1I,c2I,v1I,v2I;
  size_t w,c1,c2,v1,v2;
  for( size_t nW=0 ; nW<numWall() ; ++nW ) {
    //Read the connections
    IN >> wI;
    IN >> c1I;
    IN >> c2I;
    IN >> v1I;
    IN >> v2I;
    w = static_cast<size_t>(wI);
    c1 = static_cast<size_t>(c1I);
    c2 = static_cast<size_t>(c2I);
    v1 = static_cast<size_t>(v1I);
    v2 = static_cast<size_t>(v2I);
    //Assert all data is ok
    if( verbose>1 ) {
      std::cerr << wI << " " << c1I << " " << c2I << " " << v1I 
		<< " " << v2I << std::endl;    
      std::cerr << w << " " << c1 << " " << c2 << " " << v1 << " " << v2 
		<< " " << static_cast<size_t>(-1) << std::endl 
		<< std::endl;
    }    
    assert( w==nW );
    assert( c1==static_cast<size_t>(-1) || c1<numCell() );
    assert( c2==static_cast<size_t>(-1) || c2<numCell() );
    assert( v1<numVertex() );
    assert( v2<numVertex() );
    //Make the connections by first extracting the pointers
    Wall *wp = &(wall(w));
    Vertex *v1p = &(vertex(v1)),*v2p=&(vertex(v2));
    Cell *c1p,*c2p;
    if( c1 != static_cast<size_t>(-1) )
      c1p = &(cell(c1));
    else
      c1p = &(background_);
    if( c2 != static_cast<size_t>(-1) )
      c2p = &(cell(c2));
    else
      c2p = &(background_);
    //vertex-wall
    wall(w).setVertex( v1p,v2p );
    vertex(v1).addWall( wp );
    vertex(v2).addWall( wp );
    //cell-wall
    wall(w).setCell(c1p,c2p);
    if( c1 != static_cast<size_t>(-1) )
      cell(c1).addWall( wp );
    if( c2 != static_cast<size_t>(-1) )
      cell(c2).addWall( wp );
    //cell-vertex
    if( c1 != static_cast<size_t>(-1) ) {
      if( !cell(c1).hasVertex(v1p) ) {
				cell(c1).addVertex( v1p );
				vertex(v1).addCell( c1p );
      }
      if( !cell(c1).hasVertex(v2p) ) {
				cell(c1).addVertex( v2p );
				vertex(v2).addCell( c1p );
      }
    }
    if( c2 != static_cast<size_t>(-1) ) {
      if( !cell(c2).hasVertex(v1p) ) {
				cell(c2).addVertex( v1p );
				vertex(v1).addCell( c2p );
      }
      if( !cell(c2).hasVertex(v2p) ) {
				cell(c2).addVertex( v2p );
				vertex(v2).addCell( c2p );
      }
    }
  }
  //Read vertex positions
  //////////////////////////////////////////////////////////////////////
  if( verbose )
    std::cerr << "Tissue::readInit(IN) - reading vertex positions" 
							<< std::endl;
  size_t numVertexTmp,dimension;
  IN >> numVertexTmp;
  IN >> dimension;
  assert( numVertexTmp==numVertex() );
  assert( dimension==2 || dimension==3 );
  std::vector<double> pos(dimension);
  for( size_t i=0 ; i<numVertex() ; ++i ) {
    for( size_t j=0 ; j<dimension ; ++j )
      IN >> pos[j];
    vertex(i).setPosition(pos);
  }
	
  //Read wall data
  //////////////////////////////////////////////////////////////////////
  if( verbose )
    std::cerr << "Tissue::readInit(IN) - reading wall data" << std::endl;
	
  size_t numWallTmp,numLength,numVar;
  IN >> numWallTmp;
  IN >> numLength;
  IN >> numVar;
  assert( numWallTmp==numWall() );
  assert( numLength==1 );
  double length;
  double value;
  for (size_t i = 0; i < numWall(); ++i) {
	  IN >> length;
	  wall(i).setLength(length);
	  std::vector<double> variable;
	  for (size_t j = 0; j < numVar; ++j) {
		  IN >> value;
		  variable.push_back(value);
	  }
	  wall(i).setVariable(variable);
  }

  //Read cell variables
  //////////////////////////////////////////////////////////////////////
  if( verbose )
    std::cerr << "Tissue::readInit(IN) - reading cell variables" << std::endl;
  size_t numCellTmp,numCellVar;
  IN >> numCellTmp;
  IN >> numCellVar;
  assert( numCellTmp==numCell() );
  //assert( numCellVar==0 );
  double var=0.0;
  if( numCellVar )
    for( size_t i=0 ; i<numCell() ; ++i ) {
      for( size_t j=0 ; j<numCellVar ; ++j ) {
				IN >> var;
				cell(i).addVariable(var);
      }
    }
	//Sort all cellWalls and cellVertices to comply with area calculations
	//and plotting
	sortCellWallAndCellVertex();
	checkConnectivity(verbose);
}

void Tissue::readInit( const char *initFile, int verbose ) {

  std::ifstream IN(initFile);
  if( !IN ) {
    std::cerr << "Tissue::readInit(char*) - "
	      << "Cannot open file " << initFile << std::endl; exit(-1);}
  if( verbose )
    std::cerr << "Tissue::readInit(char*) - calling readInit(IN)" << std::endl;
  readInit(IN,verbose);
}

void Tissue::readInit( std::string initFile, int verbose ) {

  const char* iFile = initFile.c_str();
  std::ifstream IN(iFile);
  if( !IN ) {
    std::cerr << "Tissue::readInit(string) - "
	      << "Cannot open file " << initFile << std::endl; exit(-1);}
  if( verbose )
    std::cerr << "Tissue::readInit(string) - calling readInit(IN)" << std::endl;
  readInit(IN,verbose);
}

void Tissue::readMerryInit( const char *initFile, int verbose ) 
{
  std::ifstream IN(initFile);
  if( !IN ) {
    std::cerr << "Tissue::readMerryInit(char*) - "
							<< "Cannot open file " << initFile 
							<< std::endl; 
		exit(-1);
	}
  unsigned int numVertexVal,dimension;
  IN >> numVertexVal;
	IN >> dimension;
  setNumVertex( numVertexVal );
	for( size_t i=0 ; i<numVertexVal ; ++i )
    vertex(i).setIndex(i);
	
	std::vector<double> pos(dimension);
	std::vector<size_t> cellName,vertexName;
	// Read information about vertices
	for( size_t i=0 ; i<numVertexVal ; ++i ) {
		size_t tmp,numVertexCell;
		IN >> tmp;
		vertexName.push_back(tmp);
		for( size_t dim=0 ; dim<dimension ; ++dim )
			IN >> pos[dim];
		vertex(i).setPosition(pos);
		IN >> numVertexCell;
		for( size_t j=0 ; j<numVertexCell ; ++j ) {
			size_t tmpCellIndex,cellIndex=numCell(),newCellFlag=1;
			IN >> tmpCellIndex;
			for( size_t c=0 ; c<cellName.size() ; ++c ) {
				if( tmpCellIndex==cellName[c] ) {
					cellIndex=c;
					newCellFlag=0;
					break;
				}
			}
			if( newCellFlag ) {
				Cell tmpCell(cellIndex,"");
				cellName.push_back(tmpCellIndex);
				addCell(tmpCell);
			}
			vertex(i).addCell(&(cell(cellIndex)));
			cell(cellIndex).addVertex(&(vertex(i)));
		}
	}
	// Reading the walls as well
	size_t numWallVal;
	IN >> numWallVal;
	// Store vertexName,vertexIndex pairs in a hash table
	std::map<size_t,size_t> vertexNameToIndex;
	for (size_t i=0; i<numVertex(); ++i)
		vertexNameToIndex[vertexName[i]] = i;
	
	for (size_t i=0; i<numWallVal; ++i) {
		Wall tmpWall;
		tmpWall.setIndex(i);
		size_t v1Name,v2Name;
		IN >> v1Name;
		IN >> v2Name;
		if (vertexNameToIndex.find(v1Name) == vertexNameToIndex.end() ||
				vertexNameToIndex.find(v2Name) == vertexNameToIndex.end() ) {
			std::cerr << "Tissue::readMerryInit() Vertex read from file (in wall list) not found."
								<< std::endl;
			exit(-1);
		}
		Vertex *tmpVertex1(vertexP(vertexNameToIndex[v1Name]));
		Vertex *tmpVertex2(vertexP(vertexNameToIndex[v2Name]));
		tmpWall.setVertex1(tmpVertex1);
		tmpWall.setVertex2(tmpVertex2);
		// Find neighboring cells as well
		size_t numFoundCell=0;
		for (size_t c1I=0; c1I<tmpVertex1->numCell(); ++c1I)
			for (size_t c2I=0; c2I<tmpVertex2->numCell(); ++c2I)
				if (tmpVertex1->cell(c1I)==tmpVertex2->cell(c2I)) {
					if (numFoundCell==0) {
						tmpWall.setCell1(tmpVertex1->cell(c1I));
					}
					else if (numFoundCell==1) {
						tmpWall.setCell2(tmpVertex1->cell(c1I));
					}
					++numFoundCell;
				}
		if (numFoundCell==1) {
			tmpWall.setCell2(background());
			++numFoundCell;
		}
		if (numFoundCell!=2) {
			std::cerr << "Tissue::readMerryInit() Found " << numFoundCell
								<< " cells for wall " << i << std::endl;
			std::cerr << "Vertices: " << tmpVertex1->index() << " (" << v1Name << ") " 
								<< tmpVertex2->index() << " (" << v2Name << ")" << std::endl;
			for (size_t c1I=0; c1I<tmpVertex1->numCell(); ++c1I)
				for (size_t c2I=0; c2I<tmpVertex2->numCell(); ++c2I)
					std::cerr << tmpVertex1->cell(c1I)->index() << " " << tmpVertex2->cell(c2I)->index()
										<< std::endl;
			exit(-1);
		} 
		// Add the wall to tissue (including pointers from cells and vertices)
		addWall(tmpWall);
		Wall *tmpWallP=wallP(numWall()-1);
		tmpVertex1->addWall(tmpWallP);
		tmpVertex2->addWall(tmpWallP);
		if (tmpWallP->cell1()!=background())
			tmpWallP->cell1()->addWall(tmpWallP);
		if (tmpWallP->cell2()!=background())
			tmpWallP->cell2()->addWall(tmpWallP);
	}
	IN.close();
	
	
	if( verbose>1 ) {
		std::cerr << "Vertices:" << std::endl;
		for (size_t i=0; i<numVertex(); ++i) {
			std::cerr << vertex(i).index() << "\t";
			for (size_t dim=0; dim<vertex(i).numPosition(); ++dim)
				std::cerr << vertex(i).position(dim) << " ";
			std::cerr << "\t";
			for (size_t k=0; k<vertex(i).numCell(); ++k)
				std::cerr << vertex(i).cell(k)->index() << " ";
			std::cerr << std::endl;
		}

		std::cerr << "Cells:" << std::endl;
		for (size_t i=0; i<numCell(); ++i) {
			std::cerr << cell(i).index() << " (" << cellName[i] << ")\t";
			for (size_t k=0; k<cell(i).numVertex(); ++k)
				std::cerr << cell(i).vertex(k)->index() << " ";
			std::cerr << std::endl;
		}
		std::cerr << "Walls:" << std::endl;
		for (size_t i=0; i<numWall(); ++i) {
			std::cerr << wall(i).index() << "\t";
			std::cerr << wall(i).cell1()->index() << " " << wall(i).cell2()->index()<< " "
								<< wall(i).vertex1()->index() << " " << wall(i).vertex2()->index()
								<< std::endl;
		}

	}

	if (verbose)
		std::cerr << numCell() << " cells and " << numVertex() 
							<< " vertices and " << numWall() << " walls extracted by "
							<< "readMerryInit()" << std::endl;
	sortCellWallAndCellVertex();
	checkConnectivity(verbose);

	return;
	
	// Extract internal walls
	for (size_t i=0; i<numCell(); ++i) {
		for (size_t ii=i+1; ii<numCell(); ++ii) {
			std::vector<Vertex*> commonVertex;
			for (size_t j=0; j<cell(i).numVertex(); ++j)
				for (size_t jj=0; jj<cell(ii).numVertex(); ++jj)
					if (cell(i).vertex(j)==cell(ii).vertex(jj))
						commonVertex.push_back(cell(i).vertex(j));
			if (commonVertex.size()==2) {
				//cell i and ii joined by wall connected to vertices j and jj
				size_t numWallBefore=numWall();
				Wall wallTmp;
				wallTmp.setIndex(numWallBefore);
				wallTmp.setCell(&cell(i),&cell(ii));
				wallTmp.setVertex(commonVertex[0],commonVertex[1]);
				double length=0.0;
				for (size_t dim=0; dim<commonVertex[0]->numPosition(); ++dim)
					length += ( commonVertex[0]->position(dim)-commonVertex[1]->position(dim) ) *
						( commonVertex[0]->position(dim)-commonVertex[1]->position(dim) );
				length = std::sqrt(length);
				wallTmp.setLength(length);
				if( verbose>1 )
					std::cerr << "wall " << numWallBefore << " added with length "
										<< wallTmp.length() << " and connected to cells"
										<< cell(i).index() << "," << cell(ii).index() << " and vertices "
										<< commonVertex[0]->index() << "," << commonVertex[1]->index()
										<< std::endl; 
				addWall(wallTmp);
				// Add wall to cells and vertices
				cell(i).addWall( &(wall(numWallBefore)) );
				cell(ii).addWall( &(wall(numWallBefore)) );
				commonVertex[0]->addWall( &(wall(numWallBefore)) );
				commonVertex[1]->addWall( &(wall(numWallBefore)) );
			}
		}
	}
	// Extract walls towards boundary
	//assert( c1==static_cast<size_t>(-1) || c1<numCell() );
	for (size_t i=0; i<numCell(); ++i) {
		if (cell(i).numVertex() != cell(i).numWall() && cell(i).numVertex()>2 ) {
			//Sort vertices
			size_t Nv = cell(i).numVertex();
			std::vector<double> cellCenter = cell(i).positionFromVertex();
			std::vector<double> theta(Nv);
			std::vector<size_t> vI(Nv);
			for (size_t k=0; k<Nv; ++k) {				
				vI[k]=k;
				theta[k] = std::atan2( cell(i).vertex(k)->position(1)-cellCenter[1],
															 cell(i).vertex(k)->position(0)-cellCenter[0] );
			}
			for (size_t k=0; k<Nv; ++k)	
				for (size_t kk=k+1; kk<Nv; ++kk)				
					if (theta[kk]<theta[k]) {
						size_t tmpvI=vI[kk];
						double tmpTheta=theta[kk];
						vI[kk]=vI[k];
						theta[kk]=theta[k];
						vI[k]=tmpvI;
						theta[k]=tmpTheta;
					}
			for (size_t k=0; k<Nv; ++k) {
				size_t vI1 = vI[k];
				size_t vI2 = vI[(k+1)%Nv];
				//check if neigh vertices has wall inbetween
				size_t hasWall=0;
				for (size_t v=0; v<cell(i).vertex(vI1)->numWall(); ++v)
					for (size_t vv=0; vv<cell(i).vertex(vI2)->numWall(); ++vv)
						if (cell(i).vertex(vI1)->wall(v) == cell(i).vertex(vI2)->wall(vv) )
							hasWall++;
				if (hasWall>1) {
					std::cerr << "Tissue::readMerryInit() More than one vertex pair are connected "
										<< "for cell " << i << " (" << cellName[i] << ")" << std::endl;
					std::cerr << "Vertices: ";
					for (size_t kk=0; kk<cell(i).numVertex(); ++kk)
						std::cerr << cell(i).vertex(kk)->index() << " ";
					std::cerr << std::endl;
					std::cerr << "Walls: ";
					for (size_t kk=0; kk<cell(i).numWall(); ++kk)
						std::cerr << cell(i).wall(kk)->index() << " ";
					std::cerr << std::endl;					
					for (size_t v=0; v<cell(i).vertex(vI1)->numWall(); ++v)
						for (size_t vv=0; vv<cell(i).vertex(vI2)->numWall(); ++vv)
							if (cell(i).vertex(vI1)->wall(v) == cell(i).vertex(vI2)->wall(vv) )
								std::cerr << cell(i).vertex(vI1)->wall(v)->index()<< " " 
													<<  cell(i).vertex(vI2)->wall(vv)->index() << std::endl;
					exit(-1);
				}
				if (!hasWall) {
					//Add wall between vertices and add cell and bg
					size_t numWallBefore=numWall();
					Wall tmpWall;
					tmpWall.setIndex(numWallBefore);
					tmpWall.setCell(&cell(i),background());
					tmpWall.setVertex(cell(i).vertex(vI1),cell(i).vertex(vI2));
					double length=0.0;
					for (size_t dim=0; dim<cell(i).vertex(vI1)->numPosition(); ++dim)
						length += ( cell(i).vertex(vI1)->position(dim)-
												cell(i).vertex(vI2)->position(dim) ) *
							( cell(i).vertex(vI1)->position(dim)-
								cell(i).vertex(vI2)->position(dim) );
					length = std::sqrt(length);
					tmpWall.setLength(length);
					if (verbose>1)
						std::cerr << "wall " << numWallBefore << " added with length "
											<< tmpWall.length() << " and connected to cells "
											<< cell(i).index() << "," << background()->index() 
											<< " and vertices "
											<< cell(i).vertex(vI1)->index() << "," 
											<< cell(i).vertex(vI2)->index()
											<< std::endl; 
					addWall(tmpWall);
					// Add wall to cells and vertices
					cell(i).addWall( &(wall(numWallBefore)) );
					cell(i).vertex(vI1)->addWall( &(wall(numWallBefore)) );
					cell(i).vertex(vI2)->addWall( &(wall(numWallBefore)) );					
				}
			}
		}
		else {
			removeCell(i);
		}
	}
	if (verbose)
		std::cerr << numCell() << " cells and " << numVertex() 
							<< " vertices and " << numWall() << " walls extracted by "
							<< "readMerryInit()" << std::endl;
	checkConnectivity(verbose);
	sortCellWallAndCellVertex();
	checkConnectivity(verbose);
}

//!Reads a model from an open file
void Tissue::readModel(std::ifstream &IN,int verbose) {
  
  unsigned int numReactionVal,numCompartmentChangeVal,numDirection;

	if( verbose )
		std::cerr << "Reading model file:\n";
  IN >> numReactionVal;
  IN >> numCompartmentChangeVal;
	IN >> numDirection;
	assert(numDirection==0 || numDirection==1);

  //Read Reactions
  //////////////////////////////////////////////////////////////////////
  //Remove any present reactions before adding
  if( numReaction() ) 
    reaction_.resize(0);
  
	if( verbose )
		std::cerr << "reactions...\n"; 
  for( size_t i=0 ; i<numReactionVal ; i++ ) {
    if( addReaction(IN) )
      std::cerr << "Tissue::ReadModel(ifstream) "
								<< "Warning Adding reaction failed for "
								<< "tissue " << id() << " (index " << i << ")\n";
		else if( verbose )
			std::cerr << reaction(numReaction()-1)->id() << std::endl;
	}
  //Read compartmentChanges
  //////////////////////////////////////////////////////////////////////
  //Remove any present compartmentChanges before adding
  if( numCompartmentChange() ) 
    compartmentChange_.resize(0);
  
	if( verbose )
		std::cerr << "compartment changes...\n";
  for( size_t i=0 ; i<numCompartmentChangeVal ; i++ ) {
    if( addCompartmentChange(IN) ) 
      std::cerr << "Tissue::ReadModel(ifstream) "
								<< "Warning Adding compartmentChange failed for "
								<< "tissue " << id() << " (index " << i << ")\n";  
		else if( verbose )
			std::cerr << compartmentChange(numCompartmentChange()-1)->id() 
								<< std::endl;
	}
	// Read direction if applicable
	if( verbose )
		std::cerr << "direction...\n";
	if( numDirection ) {
		if( direction()->readDirection(IN) ) {
      std::cerr << "Tissue::ReadModel(ifstream) "
								<< "Adding direction failed." << std::endl;
			exit(-1);
		}
		else if( verbose )
			std::cerr << direction()->directionUpdate()->id() << std::endl
								<< direction()->directionDivision()->id() << std::endl;
	}			
	if( verbose )
		std::cerr << "Done\n\n";
}

//!Reads a model from a file
void Tissue::readModel(const char *fileName, int verbose) 
{
  std::string tmp(fileName);
  readModel(tmp,verbose);
}

//!Reads a model from file 
void Tissue::readModel(std::string fileName, int verbose) 
{  
  const char* fName = fileName.c_str();
  std::istream *IN = myFiles::openFile(fName);
  if( !IN ) {
    std::cerr << "Tissue::readModel(std::string) - "
							<< "Cannot open file " << fileName << "\n\n\7";
		exit(-1);
	}
  readModel((std::ifstream &) *IN,verbose);
}

size_t wallFromCellPair(std::vector< std::pair<size_t,size_t> > &wallCell,
			size_t c1,size_t c2) {

  for( size_t i=0 ; i<wallCell.size() ; ++i )
    if( (wallCell[i].first==c1 && wallCell[i].second==c2) ||
	(wallCell[i].first==c2 && wallCell[i].second==c1) )
      return i;
  return static_cast<size_t>(-1);
}

void Tissue::
createTissueFromSpheres(std::vector< std::vector<double> > &y,
												double rFac, int verbose) 
{  
  size_t N = y.size();
  if( !N ) return;
  std::vector< std::vector<size_t> > cellCellNeighbor(N),
    cellWallNeighbor(N);
  double d,r;
  unsigned int numWall=0,numVertex=0,num2Vertex=0,num3Vertex=0,
    num4Vertex=0;
  
  size_t rIndex = y[0].size()-1;
  if( rIndex<2 || rIndex>3 ) {
    std::cerr << "Tissue::createTissueFromSpheres()"
							<< "Only allowed for 2D and 3D.\n";
    exit(0);
  }
  //Get cell-cell neighbors which also defines the walls
  //////////////////////////////////////////////////////////////////////
  std::vector< std::pair<size_t,size_t> > wallCell;
  std::vector< std::vector<size_t> > cellWall(N);
  for(size_t i=0 ; i<N ; ++i ) {
    for(size_t j=i+1 ; j<N ; ++j ) {
      r = y[i][rIndex] + y[j][rIndex];
      d=0.;
      for( size_t dim=0 ; dim<rIndex ; dim++ )
				d += (y[i][dim]-y[j][dim])*(y[i][dim]-y[j][dim]);
      d = std::sqrt( d );
      
      if( d<=r*rFac ) {
        //Add cell-cell neighbors
        cellCellNeighbor[i].push_back(j);
        cellCellNeighbor[j].push_back(i);
				cellWall[i].push_back( numWall );
				cellWall[j].push_back( numWall );
				wallCell.push_back( std::pair<size_t,size_t>(i,j) );
        numWall++;
				if( verbose>1 )
					std::cerr << "Wall between cells " << i << "," << j << std::endl;
      }
    }
  }
  
  //Get verteces by checking cell neighbor relationships
  //////////////////////////////////////////////////////////////////////
  std::vector< std::pair<size_t,size_t> > 
    wallVertex(numWall,std::pair<size_t,size_t> (static_cast<size_t>(-1),
																								 static_cast<size_t>(-1)));
  std::vector< std::vector<size_t> > cellVertex(N),vertexCell,vertexWall;
  for(size_t i1=0 ; i1<N ; ++i1 ) {
    for(size_t k1=0 ; k1<cellCellNeighbor[i1].size() ; ++k1 ) {
      size_t i2=cellCellNeighbor[i1][k1]; 
      if( true ) { //if( i2>i1 ) { //For not double checking
				//get vector of common neighbors
				std::vector<size_t> common;
				for(size_t k2=k1+1 ; k2<cellCellNeighbor[i1].size() ; ++k2 ) {
					size_t i3=cellCellNeighbor[i1][k2]; 
					std::vector<size_t>::iterator it32 = 
						find(cellCellNeighbor[i2].begin(),
								 cellCellNeighbor[i2].end(),i3);
					//if( i3>i2 && it32 != cellCellNeighbor[i2].end() ) //common found
					if( true && it32 != cellCellNeighbor[i2].end() ) //common found
						common.push_back(i3);
				}
				//std::cerr << common.size() << " common indeces found for cells "
				//  << i1 << " " << i2 << " ( ";
				//for(size_t tmp=0 ; tmp<common.size() ; ++tmp )
				//std::cerr << common[tmp] << " ";
				//std::cerr << ")" << std::endl;
				//Check pairwise for neighbors (to identify 4-verteces)
				std::vector< std::pair<size_t,size_t> > pairs;
				std::vector<size_t> commonMarkedForPair(common.size());	
				for(size_t c1=0 ; c1<common.size() ; ++c1 )
					for(size_t c2=c1+1 ; c2<common.size() ; ++c2 ) {
						std::vector<size_t>::iterator cit = 
							find(cellCellNeighbor[common[c1]].begin(),
									 cellCellNeighbor[common[c1]].end(),common[c2]);
						if( cit != cellCellNeighbor[common[c1]].end() ) {//pair found
							pairs.push_back(std::pair<size_t,size_t>(common[c1],common[c2]));
							commonMarkedForPair[c1]++;
							commonMarkedForPair[c2]++;
						}
					}	
				//Add all 3-vertex
				for(size_t c=0 ; c<common.size() ; ++c )
					if( commonMarkedForPair[c]==0 ) {
						std::vector<size_t> tmpVec(3);
						tmpVec[0]=i1;tmpVec[1]=i2;tmpVec[2]=common[c];
						size_t addFlag=1;
						for( size_t v=0 ; v<vertexCell.size() ; ++v ) {
							std::vector<size_t>::iterator cit0 = 
								find(vertexCell[v].begin(),
										 vertexCell[v].end(),tmpVec[0]);
							std::vector<size_t>::iterator cit1 = 
								find(vertexCell[v].begin(),
										 vertexCell[v].end(),tmpVec[1]);
							std::vector<size_t>::iterator cit2 = 
								find(vertexCell[v].begin(),
										 vertexCell[v].end(),tmpVec[2]);
							if( cit0 != vertexCell[v].end() &&
									cit1 != vertexCell[v].end() &&
									cit2 != vertexCell[v].end() ) {//vertex already found
								addFlag=0;
								break;
							}
						}
						if( addFlag ) {
							vertexCell.push_back(tmpVec);
							vertexWall.push_back(std::vector<size_t>(0));
							for( size_t iTmp=0 ; iTmp<tmpVec.size() ; ++iTmp )
								cellVertex[tmpVec[iTmp]].push_back(numVertex);
							for( size_t iTmp=0 ; iTmp<tmpVec.size() ; ++iTmp )
								for( size_t jTmp=iTmp+1 ; jTmp<tmpVec.size() ; ++jTmp ) {
									size_t wallTmp = wallFromCellPair(wallCell,tmpVec[iTmp],tmpVec[jTmp]);
									if( wallTmp<numWall ) {
										if( wallVertex[wallTmp].first>numWall )
											wallVertex[wallTmp].first = numVertex;
										else if( wallVertex[wallTmp].second>numWall )
											wallVertex[wallTmp].second = numVertex;
										else
											std::cerr << "Warning, trying to add a third "
																<< "vertex to wall " << wallTmp 
																<< std::endl; 
										vertexWall[vertexWall.size()-1].push_back(wallTmp);
									}
								}
							if( verbose>1 )
								std::cerr << "3-vertex for " << i1 << " " << i2 << " "
													<< common[c] << " added" << std::endl;
							numVertex++;
							num3Vertex++;
						}
					}	
					else if( commonMarkedForPair[c]==1 ) {
						size_t i3Tmp=common[c],i4Tmp=common[c],tmpCount=0;
						for(size_t p=0 ; p<pairs.size() ; ++p )
							if( pairs[p].first==i3Tmp ) {
								i4Tmp=pairs[p].second;
								tmpCount++;
							}
							else if( pairs[p].second==i3Tmp ) {
								i4Tmp=pairs[p].first;
								tmpCount++;
							}
						//if( tmpCount==1 && i3Tmp<i4Tmp ) {
						if( tmpCount==1 && true ) {
							std::vector<size_t> tmpVec(4);
							tmpVec[0]=i1;tmpVec[1]=i2;tmpVec[2]=i3Tmp;tmpVec[3]=i4Tmp;
							size_t addFlag=1;
							for( size_t v=0 ; v<vertexCell.size() ; ++v ) {
								std::vector<size_t>::iterator cit0 = 
									find(vertexCell[v].begin(),
											 vertexCell[v].end(),tmpVec[0]);
								std::vector<size_t>::iterator cit1 = 
									find(vertexCell[v].begin(),
											 vertexCell[v].end(),tmpVec[1]);
								std::vector<size_t>::iterator cit2 = 
									find(vertexCell[v].begin(),
											 vertexCell[v].end(),tmpVec[2]);
								std::vector<size_t>::iterator cit3 = 
									find(vertexCell[v].begin(),
											 vertexCell[v].end(),tmpVec[3]);
								if( cit0 != vertexCell[v].end() &&
										cit1 != vertexCell[v].end() &&
										cit2 != vertexCell[v].end() &&
										cit3 != vertexCell[v].end() ) {//vertex already found
									addFlag=0;
									break;
								}
							}
							if( addFlag ) {
								vertexCell.push_back(tmpVec);
								vertexWall.push_back(std::vector<size_t>(0));
								for( size_t iTmp=0 ; iTmp<tmpVec.size() ; ++iTmp )
									cellVertex[tmpVec[iTmp]].push_back(numVertex);
								for( size_t iTmp=0 ; iTmp<tmpVec.size() ; ++iTmp )
									for( size_t jTmp=iTmp+1 ; jTmp<tmpVec.size() ; ++jTmp ) {
										size_t wallTmp = wallFromCellPair(wallCell,tmpVec[iTmp],tmpVec[jTmp]);
										if( wallTmp<numWall ) {
											if( wallVertex[wallTmp].first>numWall )
												wallVertex[wallTmp].first = numVertex;
											else if( wallVertex[wallTmp].second>numWall )
												wallVertex[wallTmp].second = numVertex;
											else
												std::cerr << "Warning, trying to add a third "
																	<< "vertex to wall " << wallTmp 
																	<< std::endl; 
											vertexWall[vertexWall.size()-1].push_back(wallTmp);
										}
										else
											std::cerr << "Warning found cell pair without wall"
																<< std::endl;
									}
								
								if( verbose>1 )
									std::cerr << "4-vertex for " << i1 << " " << i2 << " "
														<< i3Tmp << " " << i4Tmp << " added" 
														<< std::endl;
								numVertex++;
								num4Vertex++;
								//std::cerr << "vertexCell:" << std::endl;
								//for( size_t iTmp=0 ; iTmp<vertexCell.size() ; ++iTmp ) {
								//for( size_t jTmp=0 ; jTmp<vertexCell[iTmp].size() ; ++jTmp )
								//  std::cerr << vertexCell[iTmp][jTmp] << " ";
								//std::cerr << std::endl;
								//}
							}
						}
						else if( tmpCount != 1 ) {
							std::cerr << "Plausibel 4-vertex does not match pairs"
												<< std::endl;
						}
					}
					else {
						std::cerr << "Warning: possible 5-vertex found..." 
											<< std::endl;
					}
      } 
    }
  }
  //std::cerr << "WallCell:" << std::endl;
  //for(size_t i=0 ; i<wallCell.size() ; ++i )
  //std::cerr << i << " - " << wallCell[i].first << " " 
  //      << wallCell[i].second << std::endl;
  //std::cerr << "WallVertex:" << std::endl;
  //for(size_t i=0 ; i<wallVertex.size() ; ++i )
  //std::cerr << i << " - " << wallVertex[i].first << " " 
  //      << wallVertex[i].second << std::endl;
  
  
  //Add vertecis to empty walls
  for( size_t wallI=0 ; wallI<numWall ; ++wallI ) {
    if( wallVertex[wallI].first>numVertex ) {
      wallVertex[wallI].first = numVertex;
      cellVertex[wallCell[wallI].first].push_back(numVertex);
      cellVertex[wallCell[wallI].second].push_back(numVertex);
      std::vector<size_t> tmpVec(1);tmpVec[0]=wallI;
      vertexWall.push_back(tmpVec);
      tmpVec.resize(2);
      tmpVec[0]=wallCell[wallI].first;
      tmpVec[1]=wallCell[wallI].second;
      vertexCell.push_back(tmpVec);
      if( verbose>1 )
				std::cerr << "Adding vertex " << numVertex << " to wall (first) "
									<< wallI << std::endl;
      numVertex++;
      num2Vertex++;
    }
    if( wallVertex[wallI].second>numVertex ) {
      wallVertex[wallI].second = numVertex;
      cellVertex[wallCell[wallI].first].push_back(numVertex);
      cellVertex[wallCell[wallI].second].push_back(numVertex);
      std::vector<size_t> tmpVec(1);tmpVec[0]=wallI;
      vertexWall.push_back(tmpVec);
      tmpVec.resize(2);
      tmpVec[0]=wallCell[wallI].first;
      tmpVec[1]=wallCell[wallI].second;
      vertexCell.push_back(tmpVec);
      if( verbose>1 )
				std::cerr << "Adding vertex " << numVertex << " to wall (second) "
									<< wallI << std::endl;
      numVertex++;
      num2Vertex++;
    }
  }
  //Add walls between two twoVertices in a cell (only if there are two)
  //////////////////////////////////////////////////////////////////////
  for( size_t cellI=0 ; cellI<cellVertex.size() ; ++cellI ) {
    std::vector<size_t> twoVertexList;
    size_t numTwoVertex=0;
    for( size_t vertexI=0 ; vertexI<cellVertex[cellI].size() ; ++vertexI ) {
      if( vertexCell[ cellVertex[cellI][vertexI] ].size() == 2 ) {
				numTwoVertex++;
				twoVertexList.push_back( cellVertex[cellI][vertexI] );
      }
    }
    if( numTwoVertex == 2 && twoVertexList[0] != twoVertexList[1] ) {
      //Add new wall between these vertices and assume wall boundary towards
      //outside (ie one of the wallCell index is -1...
      size_t wallI = wallCell.size();
      numWall++;
      cellWall[cellI].push_back( wallI );
      vertexWall[twoVertexList[0]].push_back( wallI );
      vertexWall[twoVertexList[1]].push_back( wallI );
      wallCell.resize(wallI+1);
      wallVertex.resize(wallI+1);
      wallCell[wallI].first = cellI;
      wallCell[wallI].second = static_cast<size_t>(-1);
      wallVertex[wallI].first = twoVertexList[0];
      wallVertex[wallI].second = twoVertexList[1];
      if( verbose )
				std::cerr << "Wall " << wallI << " added in cell " << cellI 
									<< " (" << wallCell[wallI].second << ") between vertices " 
									<< twoVertexList[0] << " and "
									<< twoVertexList[1] << std::endl;
    }
  }
  
  std::cerr << N << " cells " << numWall << " walls and " << numVertex
						<< " vertices defined (" << num2Vertex << " 2v, " 
						<< num3Vertex << " 3v, " << num4Vertex << " 4v)"
						<< std::endl << std::endl;
  
  //Fill the tissue with cells, walls and verteces
  //////////////////////////////////////////////////////////////////////
  if( verbose )
    std::cerr << "Tissue::createTissueFromSpheres() "
							<< "Creating the tissue." 
							<< std::endl;    
  setNumCell( N );
  setNumWall( numWall );
  setNumVertex( numVertex );
  for( size_t i=0 ; i<numCell() ; i++ ) {
    cell(i).setIndex(i);
    cell(i).addVariable( y[i][rIndex] );
    for( size_t j=0 ; j<cellWall[i].size() ; j++ )
      cell(i).addWall( &wall( cellWall[i][j] ) ); 
    for( size_t j=0 ; j<cellVertex[i].size() ; j++ )
      cell(i).addVertex( &vertex( cellVertex[i][j] ) ); 
  }
  for( size_t i=0 ; i<(this->numWall()) ; i++ ) {
    wall(i).setIndex(i);
    Cell *cell1,*cell2;
    //Check if wallCell is background
    if( wallCell[i].first<numCell() )
      cell1 = &cell(wallCell[i].first);
    else
      cell1 = &background_;
    if( wallCell[i].second<numCell() )
      cell2 = &cell(wallCell[i].second);
    else
      cell2 = &background_;
		
    wall(i).setCell(cell1,cell2);
    wall(i).setVertex(&vertex(wallVertex[i].first),
											&vertex(wallVertex[i].second));
  }
  for( size_t i=0 ; i<(this->numVertex()) ; i++ ) {
    vertex(i).setIndex(i);
    for( size_t j=0 ; j<vertexCell[i].size() ; j++ )
      vertex(i).addCell( &cell( vertexCell[i][j] ) ); 
    for( size_t j=0 ; j<vertexWall[i].size() ; j++ )
      vertex(i).addWall( &wall( vertexWall[i][j] ) ); 
    size_t dimension = y[0].size()-1;
    std::vector<double> pos(dimension);
    for( size_t j=0 ; j<vertexCell[i].size() ; j++ )
      for( size_t d=0 ; d<dimension ; d++ )
				pos[d] += y[vertexCell[i][j]][d];
    for( size_t d=0 ; d<dimension ; d++ )
      pos[d] /= static_cast<double>( vertexCell[i].size() );
    vertex(i).setPosition(pos);
  }
  //Set the wall lengths from the vertices positions
  //////////////////////////////////////////////////////////////////////
  if( verbose )
    std::cerr << "Tissue::createTissueFromSpheres() "
							<< "Setting wall lengths from vertex positions." 
							<< std::endl;
  setWallLengthFromVertexPosition();
  
  //Check that the tissue is ok
	checkConnectivity(verbose);

  //std::cerr << "CellWall:" << std::endl;
  //for(size_t i=0 ; i<cellWall.size() ; ++i ) {
  //std::cerr << i << " - ";
  //for(size_t j=0 ; j<cellWall[i].size() ; ++j )
  //  std::cerr << cellWall[i][j] << " ";
  //std::cerr << std::endl;
  //}
  //std::cerr << "CellVertex:" << std::endl;
  //for(size_t i=0 ; i<cellVertex.size() ; ++i ) {
  //std::cerr << i << " - ";
  //for(size_t j=0 ; j<cellVertex[i].size() ; ++j )
  //  std::cerr << cellVertex[i][j] << " ";
  //std::cerr << std::endl;
  //}
  //std::cerr << "WallCell:" << std::endl;
  //for(size_t i=0 ; i<wallCell.size() ; ++i )
  //std::cerr << i << " - " << wallCell[i].first << " " 
  //      << wallCell[i].second << std::endl;
  //std::cerr << "WallVertex:" << std::endl;
  //for(size_t i=0 ; i<wallVertex.size() ; ++i )
  //std::cerr << i << " - " << wallVertex[i].first << " " 
  //      << wallVertex[i].second << std::endl;
  //std::cerr << "VertexCell:" << std::endl;
  //for(size_t i=0 ; i<vertexCell.size() ; ++i ) {
  //std::cerr << i << " - ";
  //for(size_t j=0 ; j<vertexCell[i].size() ; ++j )
  //  std::cerr << vertexCell[i][j] << " ";
  //std::cerr << std::endl;
  //}
  //std::cerr << "VertexWall:" << std::endl;
  //for(size_t i=0 ; i<vertexWall.size() ; ++i ) {
  //std::cerr << i << " - ";
  //for(size_t j=0 ; j<vertexWall[i].size() ; ++j )
  //  std::cerr << vertexWall[i][j] << " ";
  //std::cerr << std::endl;
  //}  
}

void Tissue::createTissueFromVoronoi(std::vector< std::vector<double> > &vertexPos,
																		 std::vector< std::vector<size_t> > &cellVertexTmp,
																		 int verbose) 
{
	std::vector< std::vector<size_t> > cellVertex(cellVertexTmp.size()),
		cellWall(cellVertexTmp.size()),vertexCell(vertexPos.size());
	size_t boundaryIndex = static_cast<size_t>(-1);
	std::set<size_t> boundaryNeighVertex;
	//Convert cellVertexTmp to cellVertex by adding additional vertices
  //at boundary and lower each index by one
	//////////////////////////////////////////////////////////////////////
	for( size_t i=0 ; i<cellVertexTmp.size() ; ++i ) {
		for( size_t k=0 ; k<cellVertexTmp[i].size() ; ++k ) {
			size_t vI1=static_cast<size_t>(cellVertexTmp[i][k]-1);
			size_t vI2=static_cast<size_t>(cellVertexTmp[i][(k+1)%cellVertexTmp[i].size()]-1);
			cellVertex[i].push_back( vI1 );
			if( vI2==boundaryIndex && vI1 != boundaryIndex ) {
				boundaryNeighVertex.insert(vI1);
			}				
			if( vI1==boundaryIndex ) {
				cellVertex[i].push_back( vI1 );
				if( vI2 != boundaryIndex )
					boundaryNeighVertex.insert(vI2);
			}
			else
				vertexCell[vI1].push_back(i);
		}
	}
	std::cerr << "cellVertexTmp converted\n";
	//Add new vertices at the boundary
	//////////////////////////////////////////////////////////////////////
	size_t numOldVertex=vertexPos.size();
	size_t numNewVertex=boundaryNeighVertex.size();
	vertexPos.resize(vertexPos.size()+numNewVertex,vertexPos[0]);
	vertexCell.resize(vertexPos.size());
	size_t kCount=0;
	for( std::set<size_t>::iterator k=boundaryNeighVertex.begin() ; 
			 k!=boundaryNeighVertex.end() ; ++k ) {
		for( size_t i=0 ; i<cellVertex.size() ; ++i ) {
			for( size_t j=0 ; j<cellVertex[i].size() ; ++j ) {
				if( *k==cellVertex[i][j] ) {
					size_t jPlus=(j+1)%cellVertex[i].size();
					size_t jMinus = j!=0 ? j-1 : cellVertex[i].size()-1; 
					if( cellVertex[i][jPlus]==boundaryIndex ) {
						cellVertex[i][jPlus]=numOldVertex+kCount;
						vertexCell[numOldVertex+kCount].push_back(i);
					}
					else if( cellVertex[i][jMinus]==boundaryIndex ) {
						cellVertex[i][jMinus]=numOldVertex+kCount;
						vertexCell[numOldVertex+kCount].push_back(i);
					}
				}
			}
		}
		++kCount;
	}
	std::cerr << "New vertices added\n";
	//Create walls
	//////////////////////////////////////////////////////////////////////
	std::vector< std::vector<size_t> > vertexWall( vertexCell.size() );
	std::vector< std::pair<size_t,size_t> > wallCell,wallVertex;
	for( size_t i=0 ; i<cellVertex.size() ; ++i ) {
		for( size_t j=0 ; j<cellVertex[i].size() ; ++j ) {
			size_t jPlus=(j+1)%cellVertex[i].size();
			size_t vI1=cellVertex[i][j];
			size_t vI2=cellVertex[i][jPlus];
			//First check if it is a boundary wall (new wall)
			if( vI1>=numOldVertex && vI2>=numOldVertex ) {
				cellWall[i].push_back(wallCell.size());
				std::pair<size_t,size_t> tmpPair(i,boundaryIndex);
				wallCell.push_back(tmpPair);
				vertexWall[vI1].push_back(wallVertex.size());
				vertexWall[vI2].push_back(wallVertex.size());
				tmpPair.first=vI1;
				tmpPair.second=vI2;
				wallVertex.push_back(tmpPair);
			}
			else {
				//Check if already in wall
				size_t inWallFlag=0;
				for( size_t k=0 ; k<wallVertex.size() ; ++k ) {
					if( ( wallVertex[k].first==vI1 && wallVertex[k].second==vI2 ) ||
							( wallVertex[k].first==vI2 && wallVertex[k].second==vI1 ) ) {
						++inWallFlag;
						if( wallCell[k].first != wallCell[k].second ) {
							std::cerr << "Tissue::createInitFromVoronoi() Wall not"
												<< " marked for additional cell." << std::endl;
							exit(-1);
						}
						wallCell[k].second=i;
						cellWall[i].push_back(k);
					}
				}
				if( !inWallFlag ) {
					//New wall, set one cell and two vertices
					cellWall[i].push_back(wallCell.size());
					std::pair<size_t,size_t> tmpPair(i,i);
					wallCell.push_back(tmpPair);
					vertexWall[vI1].push_back(wallVertex.size());
					vertexWall[vI2].push_back(wallVertex.size());
					tmpPair.first=vI1;
					tmpPair.second=vI2;
					wallVertex.push_back(tmpPair);
				}
				else if( inWallFlag>1 ) {
					std::cerr << "Tissue::createInitFromVoronoi() Vertices in"
										<< " multiple (>2) walls." << std::endl;
					exit(-1);
				}
			}
		}
	}//for i (adding walls)
	std::cerr << "New walls added\n";
	assert( cellWall.size() == cellVertex.size() );
	assert( wallCell.size() == wallVertex.size() );
	assert( vertexCell.size() == vertexWall.size() );
	assert( vertexCell.size() == vertexPos.size() );

// 	std::cerr << "cellVertex and cellWall:" << std::endl;
// 	for( size_t i=0 ; i<cellVertex.size() ; ++i ) {
// 		std::cerr << i << " (" << cellVertex[i].size() << ") ";
// 		for( size_t k=0 ; k<cellVertex[i].size() ; ++k )
// 			std::cerr << cellVertex[i][k] << " ";
// 		std::cerr << " (" << cellWall[i].size() << ") ";		
// 		for( size_t k=0 ; k<cellWall[i].size() ; ++k )
// 			std::cerr << cellWall[i][k] << " ";
// 		std::cerr << std::endl;
// 	}
// 	std::cerr << "vertexCell and vertexWall:" << std::endl;
// 	for( size_t i=0 ; i<vertexCell.size() ; ++i ) {
// 		std::cerr << i << " (" << vertexCell[i].size() << ") "; 
// 		for( size_t k=0 ; k<vertexCell[i].size() ; ++k )
// 			std::cerr << vertexCell[i][k] << " ";
// 		std::cerr << " (" << vertexWall[i].size() << ") ";
// 		for( size_t k=0 ; k<vertexWall[i].size() ; ++k )
// 			std::cerr << vertexWall[i][k] << " ";
// 		std::cerr << std::endl;
// 	}
// 	std::cerr << "wallCell and wallVertex:" << std::endl;
// 	for( size_t i=0 ; i<wallCell.size() ; ++i ) {
// 		std::cerr << i << "  "; 
// 		std::cerr << wallCell[i].first << " " << wallCell[i].second 
// 							<< "  ";
// 		std::cerr << wallVertex[i].first << " " << wallVertex[i].second;
// 		std::cerr << std::endl;
// 	}
	
	//Extract possible positions for the new vertices
	for( size_t i=numOldVertex ; i<vertexPos.size() ; ++i ) {
		std::cerr << i << std::endl;
		size_t foundWallFlag=0;
		size_t falseCell1=0,falseCell2=0;
		size_t wallI=0,cellI=0;
		//Identify wall
		for( size_t k=0 ; k<vertexWall[i].size() ; ++k )
			if( wallCell[ vertexWall[i][k] ].first != boundaryIndex &&
					wallCell[ vertexWall[i][k] ].second != boundaryIndex ) {
				++foundWallFlag;
				wallI = vertexWall[i][k];
				falseCell1 = wallCell[ vertexWall[i][k] ].first;
				falseCell2 = wallCell[ vertexWall[i][k] ].second;
			}
		if( foundWallFlag != 1 ) {
			std::cerr << "Tissue::createInitFromVoronoi() Multiple ("
								<< foundWallFlag << ") walls found for vertex" 
								<< std::endl;
			exit(-1);
		}
		//Extract vertex at other end
		assert( wallVertex[wallI].first==i || wallVertex[wallI].second==i );
		size_t vertexI = wallVertex[wallI].first==i ? 
			wallVertex[wallI].second : wallVertex[wallI].first; 
		
		//Find cell not connected to previous wall
		size_t foundCellFlag=0;
		for( size_t k=0 ; k<vertexCell[vertexI].size() ; ++k )
			if( vertexCell[vertexI][k] != falseCell1 && 
					vertexCell[vertexI][k] != falseCell2 ) {
				++foundCellFlag;				
				cellI = vertexCell[vertexI][k];
			}
		if( foundCellFlag != 1 ) {
			std::cerr << "Tissue::createInitFromVoronoi() Multiple ("
								<< foundCellFlag << ") cells found for vertex " 
								<< vertexI << std::endl;
			std::cerr << falseCell1 << " " << falseCell2 << std::endl;
			exit(-1);
		}
		std::vector<double> cellPos( vertexPos[0].size() );
		if( !cellVertex[cellI].size() ) {
			std::cerr << "No vertices defined for chosen cell" << std::endl;
			exit(-1);
		}
		for( size_t k=0 ; k<cellVertex[cellI].size() ; ++k )
			for( size_t d=0 ; d<cellPos.size() ; ++d )
				cellPos[d] += vertexPos[cellVertex[cellI][k]][d];
		for( size_t d=0 ; d<cellPos.size() ; ++d )
			cellPos[d] /= cellVertex[cellI].size();

		//Extract direction and normalize
		std::vector<double> direction(cellPos.size());
		double norm=0.0;
		for( size_t d=0 ; d<cellPos.size() ; ++d ) {
			direction[d] = vertexPos[vertexI][d]-cellPos[d];
			norm += direction[d]*direction[d];
		}
		if( norm<=0.0 ) {
			std::cerr << "Tissue::createInitFromVoronoi() Direction "
								<< "without length (" << norm << ")" << std::endl; 
			exit(-1);
		}
		norm = std::sqrt(norm);
		if( norm<=0.0 ) {
			std::cerr << "Tissue::createInitFromVoronoi() Direction "
								<< "without length (" << norm << ")" << std::endl; 
			exit(-1);
		}
		for( size_t d=0 ; d<cellPos.size() ; ++d )
			direction[d] /= norm;
		//Set new vertex position
		double length=1.0;
		for( size_t d=0 ; d<cellPos.size() ; ++d )
			vertexPos[i][d] = vertexPos[vertexI][d] + length*direction[d];
	}
	
	//Create the tissue
	assert( cellWall.size() == cellVertex.size() );
	assert( wallCell.size() == wallVertex.size() );
	assert( vertexCell.size() == vertexWall.size() );
	assert( vertexCell.size() == vertexPos.size() );
	setNumCell( cellWall.size() );
	setNumWall( wallCell.size() );
	setNumVertex( vertexCell.size() );
	for( size_t i=0 ; i<numCell() ; ++i ) {
		cell(i).setIndex(i);
		for( size_t k=0 ; k<cellWall[i].size() ; ++k )
			cell(i).addWall( &wall(cellWall[i][k]) );
		for( size_t k=0 ; k<cellVertex[i].size() ; ++k )
			cell(i).addVertex( &vertex(cellVertex[i][k]) );
	}
	for( size_t i=0 ; i<numVertex() ; ++i ) {
		vertex(i).setIndex(i);
		for( size_t k=0 ; k<vertexCell[i].size() ; ++k )
			vertex(i).addCell( &cell(vertexCell[i][k]) );
		for( size_t k=0 ; k<vertexWall[i].size() ; ++k )
			vertex(i).addWall( &wall(vertexWall[i][k]) );
	}
	for( size_t i=0 ; i<numWall() ; ++i ) {
		wall(i).setIndex(i);
		if( wallCell[i].first == static_cast<size_t>(-1) )
			wall(i).setCell(background(),&cell(wallCell[i].second));
		else if( wallCell[i].second == static_cast<size_t>(-1) )
			wall(i).setCell(&cell(wallCell[i].first),background());
		else
			wall(i).setCell(&cell(wallCell[i].first),&cell(wallCell[i].second));
		wall(i).setVertex( &vertex(wallVertex[i].first),&vertex(wallVertex[i].second) );
	}
	
// 	std::cerr << "cellVertex and cellWall:" << std::endl;
// 	for( size_t i=0 ; i<cellVertex.size() ; ++i ) {
// 		std::cerr << i << "," << cell(i).index() << " (" << cellVertex[i].size() << ","
// 							<< cell(i).numVertex() << ") ";
// 		for( size_t k=0 ; k<cellVertex[i].size() ; ++k )
// 			std::cerr << cellVertex[i][k] << "," << cell(i).vertex(k)->index()
// 								<< " ";
// 		std::cerr << " (" << cellWall[i].size() << ","
// 							<< cell(i).numWall() << ") ";
// 		for( size_t k=0 ; k<cellWall[i].size() ; ++k )
// 			std::cerr << cellWall[i][k] << "," << cell(i).wall(k)->index()
// 								<< " ";
// 		std::cerr << std::endl;
// 	}
// 	std::cerr << "vertexCell and vertexWall:" << std::endl;
// 	for( size_t i=0 ; i<vertexCell.size() ; ++i ) {
// 		std::cerr << i << "," << vertex(i).index() << " (" 
// 							<< vertexCell[i].size() << "," 
// 							<< vertex(i).numCell() << ") ";
// 		for( size_t k=0 ; k<vertexCell[i].size() ; ++k )
// 			std::cerr << vertexCell[i][k] << "," << vertex(i).cell(k)->index()
// 								<< " ";
// 		std::cerr << " (" << vertexWall[i].size() << "," 
// 							<< vertex(i).numWall() << ") ";
// 		for( size_t k=0 ; k<vertexWall[i].size() ; ++k )
// 			std::cerr << vertexWall[i][k] << "," << vertex(i).wall(k)->index()
// 								<< " ";
// 		std::cerr << std::endl;
// 	}
// 	std::cerr << "wallCell and wallVertex:" << std::endl;
// 	for( size_t i=0 ; i<wallCell.size() ; ++i ) {
// 		std::cerr << i << "," << wall(i).index() << "  "; 
// 		std::cerr << wallCell[i].first << "," << wall(i).cell1()->index()
// 							<< " " << wallCell[i].second << "," 
// 							<< wall(i).cell2()->index() << "  ";
// 		std::cerr << wallVertex[i].first << "," << wall(i).vertex1()->index()
// 							<< " " << wallVertex[i].second << "," 
// 							<< wall(i).vertex2()->index() << "  ";
		
// 		std::cerr << std::endl;
// 	}

	//Set positions for the vertices
	assert( numVertex()==vertexPos.size() );
	for( size_t i=0 ; i<numVertex() ; ++i )
		vertex(i).setPosition(vertexPos[i]);

	//Get the wall lengths from teh vertex positions
	setWallLengthFromVertexPosition();
	
	std::cerr << "Checking tissue" << std::endl;
	checkConnectivity(verbose);
	std::cerr << "Tisue created" << std::endl;
	std::vector< std::vector<double> > cellData( numCell() ),
		cellDeriv( numCell() ),wallData( numWall() ),wallDeriv( numWall() ),
		vertexData( numVertex() ),vertexDeriv( numVertex() );
	removeEpidermalCells(cellData,wallData,vertexData,cellDeriv,wallDeriv,
											 vertexDeriv);
	std::cerr << "Checking tissue after first removal" << std::endl;
	checkConnectivity(verbose);
	std::cerr << "Tisue created" << std::endl;
	removeEpidermalCells(cellData,wallData,vertexData,cellDeriv,wallDeriv,
											 vertexDeriv);
	std::cerr << "Checking tissue after second removal" << std::endl;
	checkConnectivity(verbose);
	std::cerr << "Tisue created" << std::endl;
	removeEpidermalCells(cellData,wallData,vertexData,cellDeriv,wallDeriv,
											 vertexDeriv);
	std::cerr << "Checking tissue after third removal" << std::endl;
	checkConnectivity(verbose);
	std::cerr << "Tisue created" << std::endl;

}

void Tissue::derivs( std::vector< std::vector<double> > &cellData,
		     std::vector< std::vector<double> > &wallData,
		     std::vector< std::vector<double> > &vertexData,
		     std::vector< std::vector<double> > &cellDeriv,
		     std::vector< std::vector<double> > &wallDeriv,
		     std::vector< std::vector<double> > &vertexDeriv ) {
  
  //Set all derivatives to zero
  for( size_t i=0 ; i<cellDeriv.size() ; ++i )
    std::fill(cellDeriv[i].begin(),cellDeriv[i].end(),0.0);
  for( size_t i=0 ; i<wallDeriv.size() ; ++i )
    std::fill(wallDeriv[i].begin(),wallDeriv[i].end(),0.0);
  for( size_t i=0 ; i<vertexDeriv.size() ; ++i )
    std::fill(vertexDeriv[i].begin(),vertexDeriv[i].end(),0.0);
  
  //Calculate derivative contributions from all reactions
  for( size_t r=0 ; r<numReaction() ; ++r )
    reaction(r)->derivs(*this,cellData,wallData,vertexData,
			cellDeriv,wallDeriv,vertexDeriv);
}

void::Tissue::initiateReactions(std::vector< std::vector<double> > &cellData,
																std::vector< std::vector<double> > &wallData,
																std::vector< std::vector<double> > &vertexData) 
{
	for (size_t i=0; i<numReaction(); ++i)
		reaction(i)->initiate(*this,cellData,wallData,vertexData);
}

void::Tissue::updateReactions(std::vector< std::vector<double> > &cellData,
															std::vector< std::vector<double> > &wallData,
															std::vector< std::vector<double> > &vertexData,
															double step) 
{
	for (size_t i=0; i<numReaction(); ++i)
		reaction(i)->update(*this,cellData,wallData,vertexData,step);	
}

void::Tissue::
initiateDirection(std::vector< std::vector<double> > &cellData,
									std::vector< std::vector<double> > &wallData,
									std::vector< std::vector<double> > &vertexData,
									std::vector< std::vector<double> > &cellDerivs,
									std::vector< std::vector<double> > &wallDerivs,
									std::vector< std::vector<double> > &vertexDerivs ) 
{
	direction()->initiate(*this,cellData,wallData,vertexData,cellDerivs,
												wallDerivs,vertexDerivs);
}

void::Tissue::
updateDirection(double step,
								std::vector< std::vector<double> > &cellData,
								std::vector< std::vector<double> > &wallData,
								std::vector< std::vector<double> > &vertexData,
								std::vector< std::vector<double> > &cellDerivs,
								std::vector< std::vector<double> > &wallDerivs,
								std::vector< std::vector<double> > &vertexDerivs) 
{
	direction()->update(*this,step,cellData,wallData,vertexData,cellDerivs,
											wallDerivs,vertexDerivs);	
}

void::Tissue::
updateDirectionDivision(size_t cellI,
												std::vector< std::vector<double> > &cellData,
												std::vector< std::vector<double> > &wallData,
												std::vector< std::vector<double> > &vertexData,
												std::vector< std::vector<double> > &cellDerivs,
												std::vector< std::vector<double> > &wallDerivs,
												std::vector< std::vector<double> > &vertexDerivs) 
{
	direction()->divide(*this,cellI,cellData,wallData,vertexData,
											cellDerivs,wallDerivs,vertexDerivs);	
}

void Tissue::
checkCompartmentChange( std::vector< std::vector<double> > &cellData,
												std::vector< std::vector<double> > &wallData,
												std::vector< std::vector<double> > &vertexData,
												std::vector< std::vector<double> > &cellDeriv,
												std::vector< std::vector<double> > &wallDeriv,
												std::vector< std::vector<double> > &vertexDeriv ) {
  
  for( size_t k=0 ; k<numCompartmentChange() ; ++k ) {
    for( size_t i=0 ; i<numCell() ; ++i ) {
      if( compartmentChange(k)->flag(this,i,cellData,wallData,vertexData,cellDeriv,wallDeriv,vertexDeriv) ) {
				compartmentChange(k)->update(this,i,cellData,wallData,vertexData,cellDeriv,wallDeriv,vertexDeriv);
				//If cell division, sort walls and vertices for cell plus 
        //divided cell plus their neighbors
				//Get list of potential cells to be sorted
				//Also add division rule for directions
				if( compartmentChange(k)->numChange()==1 ) {
					std::set<size_t> sortCell;
					sortCell.insert(i);
					size_t ii=numCell()-1;
					sortCell.insert(ii);
					for( size_t k=0 ; k<cell(i).numWall() ; ++k ) {
						if( cell(i).wall(k)->cell1()->index() == i )
							sortCell.insert(cell(i).wall(k)->cell2()->index());
						else
							sortCell.insert(cell(i).wall(k)->cell1()->index());
					}
					for( size_t k=0 ; k<cell(ii).numWall() ; ++k ) {
						if( cell(ii).wall(k)->cell1()->index() == ii )
							sortCell.insert(cell(ii).wall(k)->cell2()->index());
						else
							sortCell.insert(cell(ii).wall(k)->cell1()->index());
					}									
					//Remove if background within the list
					sortCell.erase( static_cast<size_t>(-1) );
					//Sort the cells
					//for( std::set<size_t>::iterator k=sortCell.begin() ; 
					//	 k!=sortCell.end() ; ++k )
					//std::cerr << *k << " ";
					//std::cerr << "to be sorted" << std::endl;
					for( std::set<size_t>::iterator k=sortCell.begin() ; 
							 k!=sortCell.end() ; ++k )
						cell(*k).sortWallAndVertex(*this);
				}	
				else if( compartmentChange(k)->numChange()==-1 )
					--i;
				else if( compartmentChange(k)->numChange()<-1 )
					i=numCell()+1;
			}
		}
	}
}

void Tissue::removeCell(size_t cellIndex,
												std::vector< std::vector<double> > &cellData,
												std::vector< std::vector<double> > &wallData,
												std::vector< std::vector<double> > &vertexData,
												std::vector< std::vector<double> > &cellDeriv,
												std::vector< std::vector<double> > &wallDeriv,
												std::vector< std::vector<double> > &vertexDeriv ) 
{
	assert(cellIndex<numCell());
	std::vector<size_t> wallRemove;
	//Mark walls for removal via index or change wallCell to background
	//To be removed if connected to removed cell(by default) and background
	for( size_t k=0 ; k<cell(cellIndex).numWall() ; ++k )
		if( ( cell(cellIndex).wall(k)->cell1()->index() == cellIndex &&
					cell(cellIndex).wall(k)->cell2() == background() ) ||
				( cell(cellIndex).wall(k)->cell2()->index() == cellIndex &&
					cell(cellIndex).wall(k)->cell1() == background() ) ) {
			wallRemove.push_back(cell(cellIndex).wall(k)->index());
			//cell(cellIndex).wall(k)->setCell1( background() );
			//cell(cellIndex).wall(k)->setCell2( background() );
		}
		else if( cell(cellIndex).wall(k)->cell1()->index() == cellIndex ) {
			cell(cellIndex).wall(k)->setCell1( background() );
			//std::cerr << "Cell " << cellIndex << " switched to bg for wall "
			//				<< cell(cellIndex).wall(k)->index() << std::endl;
		}
		else if( cell(cellIndex).wall(k)->cell2()->index() == cellIndex ) {
			cell(cellIndex).wall(k)->setCell2( background() );
			//std::cerr << "Cell " << cellIndex << " switched to bg for wall "
			//				<< cell(cellIndex).wall(k)->index() << std::endl;
		}
		else {
			std::cerr << "Tissue::removeCell() wall not connected to cell"
								<< std::endl;
			exit(-1);
		}
	//Remove cell and potential wall connections from vertices
	//Caveat: Also remove background...
	for( size_t k=0 ; k<cell(cellIndex).numVertex() ; ++k ) {
		//cell(cellIndex).vertex(k)->removeCell( &cell(cellIndex) );
		//if( cell(cellIndex).vertex(k)->removeCell( &cell(cellIndex) ) )
		//std::cerr << "Cell " << cellIndex << " removed from vertex "
		//				<< cell(cellIndex).vertex(k)->index() << std::endl;
		//cell(cellIndex).vertex(k)->removeCell( background() );
		//if( cell(cellIndex).vertex(k)->removeCell( background() ) )
		//std::cerr << "Background(cell) removed from vertex "
		//				<< cell(cellIndex).vertex(k)->index() << std::endl;
		
		//for( size_t w=0 ; w<wallRemove.size() ; ++w )
		//cell(cellIndex).vertex(k)->removeWall( &wall(wallRemove[w]) );
		
//if( cell(cellIndex).vertex(k)->removeWall( &wall(wallRemove[w]) ) )
		//	std::cerr << "Wall " << wallRemove[w] << " removed from vertex "
		//						<< cell(cellIndex).vertex(k)->index() << std::endl;		
	}
	
	static size_t numCR=0,numWR=0,numVR=0; 
	//Remove vertices without connection to cells or walls
	for( size_t k=0 ; k<cell(cellIndex).numVertex() ; ++k ) {
		//Remove cell from vertex
		cell(cellIndex).vertex(k)->removeCell( &cell(cellIndex) );
		//Remove walls from vertex
		for( size_t w=0 ; w<wallRemove.size() ; ++w )
			cell(cellIndex).vertex(k)->removeWall( &wall(wallRemove[w]) );
		
		if( cell(cellIndex).vertex(k)->numCell() == 0 &&
				cell(cellIndex).vertex(k)->numWall() == 0 ) {
			//remove vertex
			size_t vI=cell(cellIndex).vertex(k)->index();
			if( vI>=vertexData.size() ) {
				std::cerr << "Tissue::removeCell() wrong in index " << std::endl
									<< numCell() << " " << numWall() << " " << numVertex()
									<< std::endl
									<< cellIndex << " " << cell(cellIndex).index() << " "
									<< cell(cellIndex).numVertex() << " "
									<< cell(cellIndex).numWall() << std::endl;
				for( size_t kk=0 ; kk<cell(cellIndex).numVertex() ; ++kk )
					std::cerr << cell(cellIndex).vertex(kk)->index() << " ";
				std::cerr << std::endl;				
			} 
			assert( vI<vertexData.size() );
			vertexData[vI] = vertexData[vertexData.size()-1];
			vertexDeriv[vI] = vertexDeriv[vertexDeriv.size()-1];
			vertexData.pop_back();
			vertexDeriv.pop_back();
			removeVertex(vI);
			std::cerr << "Vertex " << vI << " removed" << std::endl;
			numVR++;
		}
		else if( cell(cellIndex).vertex(k)->numCell() == 0 ||
						 cell(cellIndex).vertex(k)->numWall() == 0 ) {
			std::cerr << "Tissue::removeCell() strange vertex." << std::endl;
			std::cerr << "It has " << cell(cellIndex).vertex(k)->numCell() 
								<< " cells and " << cell(cellIndex).vertex(k)->numWall()
								<< " walls." << std::endl;
			std::cerr << "Cells: ";
			for( size_t kk=0 ; kk<cell(cellIndex).vertex(k)->numCell() ; ++kk )
				std::cerr << cell(cellIndex).vertex(k)->cell(kk)->index() << " ";
			std::cerr << "\nWalls: ";
			for( size_t kk=0 ; kk<cell(cellIndex).vertex(k)->numWall() ; ++kk )
				std::cerr << cell(cellIndex).vertex(k)->wall(kk)->index() << " ";			
			exit(-1);
		}
	}
	//Remove walls connected to cellIndex and background
	for( size_t k=0 ; k<cell(cellIndex).numWall() ; ++k ) {
	  if( ( cell(cellIndex).wall(k)->cell1() == background() &&
		cell(cellIndex).wall(k)->cell2() == &cell(cellIndex) ) ||
	      ( cell(cellIndex).wall(k)->cell2() == background() &&
		cell(cellIndex).wall(k)->cell1() == &cell(cellIndex) ) ) {
			size_t wI=cell(cellIndex).wall(k)->index();
			//std::cerr << wI << " " << numWall() << " " << wallData.size() << std::endl;
			assert( wI<wallData.size() );
			wallData[wI] = wallData[wallData.size()-1];
			wallDeriv[wI] = wallDeriv[wallDeriv.size()-1];
			wallData.pop_back();
			wallDeriv.pop_back();
			removeWall(wI);
			//std::cerr << "Wall " << wI << " removed." << std::endl;
			numWR++;
			//wall(wI).setIndex(wI);
			//std::cerr << wI << " " << numWall() << " " << wallData.size() << std::endl;
		}
	}
// 	//Old malfunctional version
// 	for( size_t k=0 ; k<wallRemove.size() ; ++k ) {
// 		size_t wI=wallRemove[k];
// 		std::cerr << wI << " " << numWall() << " " << wallData.size() << std::endl;
// 		assert( wI<wallData.size() );
// 		wallData[wI] = wallData[wallData.size()-1];
// 		wallDeriv[wI] = wallDeriv[wallDeriv.size()-1];
// 		wallData.pop_back();
// 		wallDeriv.pop_back();
// 		removeWall(wI);
// 		std::cerr << "Wall " << wI << " removed." << std::endl;
// 		numWR++;
// 		wall(wI).setIndex(wI);
// 	}
	//Remove cell
	//std::cerr << cellIndex << " " << numCell() << " " << cellData.size()
	//				<< std::endl;
	assert( cellIndex<cellData.size() );
	cellData[cellIndex] = cellData[cellData.size()-1];
	cellDeriv[cellIndex] = cellDeriv[cellDeriv.size()-1];
	cellData.pop_back();
	cellDeriv.pop_back();
	removeCell(cellIndex);
	//std::cerr << "Cell " << cellIndex << " removed." << std::endl;
	//cell(cellIndex).setIndex(cellIndex);
	numCR++;
	//std::cerr << cellIndex << " " << numCell() << " " << cellData.size()
	//				<< std::endl;

	assert( cellData.size() == numCell() );
	assert( wallData.size() == numWall() );
	assert( vertexData.size() == numVertex() );	
	//checkConnectivity(1);
	std::cerr << numCR << " cells, " << numWR << " walls, and "
						<< numVR << " vertices removed in total" << std::endl;
}

void Tissue::
removeCells(std::vector<size_t> &cellIndex,
						std::vector< std::vector<double> > &cellData,
						std::vector< std::vector<double> > &wallData,
						std::vector< std::vector<double> > &vertexData,
						std::vector< std::vector<double> > &cellDeriv,
						std::vector< std::vector<double> > &wallDeriv,
						std::vector< std::vector<double> > &vertexDeriv ) 
{
	// Sort the indices to make sure highest indices are removed first 
	// (since removed index is occupied with the last one)
	sort(cellIndex.begin(),cellIndex.end());
	
	size_t numRemove = cellIndex.size();
	for (size_t ii=0; ii<numRemove; ++ii) {
		size_t i = numRemove-(ii+1);
		removeCell(cellIndex[i],cellData,wallData,vertexData,cellDeriv,wallDeriv,
							 vertexDeriv);
	}
}

void Tissue::
removeEpidermalCells(std::vector< std::vector<double> > &cellData,
										 std::vector< std::vector<double> > &wallData,
										 std::vector< std::vector<double> > &vertexData,
										 std::vector< std::vector<double> > &cellDeriv,
										 std::vector< std::vector<double> > &wallDeriv,
										 std::vector< std::vector<double> > &vertexDeriv,
										 double radialThreshold ) 
{
	size_t dimension=vertexData[0].size();
	std::vector<size_t> cellR;
	//Mark cells for removal (sorted with highest index first
	for( size_t i=0 ; i<numCell() ; ++i ) {
		size_t cellI=numCell()-1-i;
		if( cell(cellI).isNeighbor( background() ) ) {
			if( radialThreshold>0.0 ) {//check that cell is outside
				std::vector<double> cellPos;
				cellPos = cell(cellI).positionFromVertex(vertexData);
				double r=0.0;
				for( size_t d=0 ; d<dimension ; ++d )
					r += cellPos[d]*cellPos[d];
				if( r>0.0 )
					r = std::sqrt(r);
				if( r>0.0 && r>radialThreshold )					
					cellR.push_back( cellI );
			}
			else
				cellR.push_back( cellI );
		}
	}
	std::cerr << "Removing " << cellR.size() << " epidermal cells:" 
						<< std::endl;
	for( size_t i=0 ; i<cellR.size() ; ++i )
		std::cerr << cellR[i] << " ";
	std::cerr << std::endl;
	
	//Remove cells
	for( size_t i=0 ; i<cellR.size() ; ++i ) {
		removeCell(cellR[i],cellData,wallData,vertexData,cellDeriv,wallDeriv,
							 vertexDeriv);
		//checkConnectivity(1);
	}
}

void Tissue::removeEpidermalCellsMk2(std::vector< std::vector<double> > &cellData,
	std::vector< std::vector<double> > &wallData,
	std::vector< std::vector<double> > &vertexData,
	std::vector< std::vector<double> > &cellDeriv,
	std::vector< std::vector<double> > &wallDeriv,
	std::vector< std::vector<double> > &vertexDeriv,
	double radialThreshold) 
{
	size_t dimensions = vertexData[0].size();
	std::vector<size_t> cellR;

	//Mark cells for removal (sorted with highest index first)
	for (size_t i = 0; i < numCell(); ++i) {
		size_t cellI = numCell() - 1 - i;

		if (cell(cellI).isNeighbor(background())) {
 			if (radialThreshold > 0.0) { //check that cell is outside
				Cell &c = cell(cellI);

				bool marked = true;

				for (size_t j = 0; j < c.numVertex(); ++j) {
					Vertex *vertex = c.vertex(j);

					double r = 0.0;
					
					for (size_t d = 0; d < dimensions; ++d) {
						r += vertexData[vertex->index()][d] * vertexData[vertex->index()][d];
					}
					
					if (r < radialThreshold * radialThreshold) {
						marked = false;
						break;
					}
				}

				if (marked == true) {
					cellR.push_back(cellI);
				}
			}
			else {
				cellR.push_back(cellI);
			}
		}
	}

	std::cerr << "Removing " << cellR.size() << " epidermal cells:" << std::endl;
	for (size_t i = 0; i < cellR.size(); ++i) {
		std::cerr << cellR[i] << " ";
	}

	std::cerr << std::endl;
	
	//Remove cells
	for (size_t i = 0; i < cellR.size(); ++i) {
		removeCell(cellR[i], cellData, wallData, vertexData, cellDeriv, wallDeriv, vertexDeriv);
		//checkConnectivity(1);
	}
}

void Tissue::
removeEpidermalCellsAtDistance(std::vector< std::vector<double> > &cellData,
															 std::vector< std::vector<double> > &wallData,
															 std::vector< std::vector<double> > &vertexData,
															 std::vector< std::vector<double> > &cellDeriv,
															 std::vector< std::vector<double> > &wallDeriv,
															 std::vector< std::vector<double> > &vertexDeriv,
															 double distanceThreshold,double max,
															 size_t direction ) 
{
	size_t dimension;
	dimension = vertexData[0].size();
	assert( direction<dimension );
	std::vector<size_t> cellR;
	//Mark cells for removal (sorted with highest index first
	for( size_t i=0 ; i<numCell() ; ++i ) {
		size_t cellI=numCell()-1-i;
		if( cell(cellI).isNeighbor( background() ) ) {
			std::vector<double> cellPos;
			cellPos = cell(cellI).positionFromVertex(vertexData);
			double dist = std::fabs( cellPos[direction]-max );
			if( dist>distanceThreshold )					
				cellR.push_back( cellI );
		}
	}
	std::cerr << "Removing " << cellR.size() << " epidermal cells:" 
						<< std::endl;
	for( size_t i=0 ; i<cellR.size() ; ++i )
		std::cerr << cellR[i] << " ";
	std::cerr << std::endl;
	
	//Remove cells
	for( size_t i=0 ; i<cellR.size() ; ++i ) {
		removeCell(cellR[i],cellData,wallData,vertexData,cellDeriv,wallDeriv,
							 vertexDeriv);
		//checkConnectivity(1);
	}
}

void Tissue::divideCell( Cell *divCell, size_t wI, size_t w3I, 
												 std::vector<double> &v1Pos,
												 std::vector<double> &v2Pos,
												 std::vector< std::vector<double> > &cellData,
												 std::vector< std::vector<double> > &wallData,
												 std::vector< std::vector<double> > &vertexData,
												 std::vector< std::vector<double> > &cellDeriv,
												 std::vector< std::vector<double> > &wallDeriv,
												 std::vector< std::vector<double> > &vertexDeriv,
												 std::vector<size_t> &volumeChangeList,
												 double threshold) 

{	
	size_t Nc=numCell(),Nw=numWall(),Nv=numVertex();
	size_t i = divCell->index();
  size_t dimension = vertexData[0].size();

	//Move new vertices if closer than threshold to old vertex
	if( threshold>=0.0 ) {
		Wall *w1 = divCell->wall(wI), *w2 = divCell->wall(w3I);
		double w1L=0.0,w2L=0.0,t1=0.0,t2=0.0;
		for( size_t dim=0; dim<dimension; ++dim ) {
			w1L += (vertexData[w1->vertex1()->index()][dim]-
							vertexData[w1->vertex2()->index()][dim]) *
				(vertexData[w1->vertex1()->index()][dim] -
				 vertexData[w1->vertex2()->index()][dim]);
			w2L += (vertexData[w2->vertex1()->index()][dim]-
							vertexData[w2->vertex2()->index()][dim]) *
				(vertexData[w2->vertex1()->index()][dim] -
				 vertexData[w2->vertex2()->index()][dim]);
			t1 += (v1Pos[dim]-vertexData[w1->vertex2()->index()][dim])*
				(v1Pos[dim]-vertexData[w1->vertex2()->index()][dim]);
			t2 += (v2Pos[dim]-vertexData[w2->vertex2()->index()][dim])*
				(v2Pos[dim]-vertexData[w2->vertex2()->index()][dim]);
		}
		w1L = std::sqrt(w1L);
		w2L = std::sqrt(w2L);
		t1 = std::sqrt(t1)/w1L;
		t2 = std::sqrt(t2)/w2L;
		assert( t1>=0.0 && t1<=1.0 );
		assert( t2>=0.0 && t2<=1.0 );
		if( t1<threshold ) {
			std::cerr << "Tissue::divideCell() Moving vertex 1 from "
								<< t1 << " to " << threshold << std::endl;
			t1=threshold;
			for( size_t dim=0; dim<dimension; ++dim ) {
				v1Pos[dim] = vertexData[w1->vertex2()->index()][dim] +
					t1*(vertexData[w1->vertex1()->index()][dim]-
							vertexData[w1->vertex2()->index()][dim]);
			}
		}
		else if( t1>(1.0-threshold) ) {
			std::cerr << "Tissue::divideCell() Moving vertex 1 from "
								<< t1 << " to " << 1.0-threshold << std::endl;
			t1 = threshold;
			for( size_t dim=0; dim<dimension; ++dim ) {
				v1Pos[dim] = vertexData[w1->vertex1()->index()][dim] +
					t1*(vertexData[w1->vertex2()->index()][dim]-
							vertexData[w1->vertex1()->index()][dim]);
			}
		}
		if( t2<threshold ) {
			std::cerr << "Tissue::divideCell() Moving vertex 2 from "
								<< t2 << " to " << threshold << std::endl;
			t2=threshold;
			for( size_t dim=0; dim<dimension; ++dim ) {
				v2Pos[dim] = vertexData[w2->vertex2()->index()][dim] +
					t2*(vertexData[w2->vertex1()->index()][dim]-
							vertexData[w2->vertex2()->index()][dim]);
			}
		}
		else if( t2>(1.0-threshold) ) {
			std::cerr << "Tissue::divideCell() Moving vertex 2 from "
								<< t2 << " to " << 1.0-threshold << std::endl;
			t2 = threshold;
			for( size_t dim=0; dim<dimension; ++dim ) {
				v2Pos[dim] = vertexData[w2->vertex1()->index()][dim] +
					t2*(vertexData[w2->vertex2()->index()][dim]-
							vertexData[w2->vertex1()->index()][dim]);
			}
		}		
	}

	//Create the new data structure and set indices in the tissue vectors
	//////////////////////////////////////////////////////////////////////
	//Add the new cell
	addCell( cell(i) );
	cell(Nc).setIndex(Nc);
  cellData.resize(Nc+1,cellData[i]);
  cellDeriv.resize(Nc+1,cellDeriv[0]);
  
	//Add the two new vertices
  Vertex tmpVertex;
  tmpVertex.setPosition(v1Pos);
  tmpVertex.setIndex(Nv);
  addVertex(tmpVertex);
  vertexData.resize(Nv+1,v1Pos);  
  tmpVertex.setPosition(v2Pos);
  tmpVertex.setIndex(Nv+1);
  addVertex(tmpVertex);
  vertexData.resize(Nv+2,v2Pos);  
  vertexDeriv.resize(Nv+2,vertexDeriv[0]);  
  
	//Add the three new walls
  Wall tmpWall;
	//New wall dividing old cell into two
  tmpWall.setIndex(Nw);
  addWall(tmpWall);
  wallData.resize(Nw+1,wallData[0]);
	double tmpLength = 0.0;
	for( size_t d=0 ; d<dimension ; ++d )
		tmpLength += (v1Pos[d]-v2Pos[d])*(v1Pos[d]-v2Pos[d]);
	wallData[Nw][0] = std::sqrt( tmpLength );

	//Wall continuing the first selected wall
  addWall( *(cell(i).wall(wI)) );
  wall(Nw+1).setIndex(Nw+1);
	//Set new lengths as fractions of the old determined from the new 
  //vertex position
	double oldL = wallData[cell(i).wall(wI)->index()][0];
	size_t v1w = cell(i).wall(wI)->vertex1()->index();
	size_t v2w = cell(i).wall(wI)->vertex2()->index();
	tmpLength=0.0; 
	double tmpLengthFrac=0.0; 
	for( size_t d=0 ; d<dimension ; ++d ) {
		tmpLengthFrac += (v1Pos[d]-vertexData[v1w][d])*
			(v1Pos[d]-vertexData[v1w][d]);
		tmpLength += (vertexData[v2w][d]-vertexData[v1w][d])*
			(vertexData[v2w][d]-vertexData[v1w][d]);
	}
	tmpLength = std::sqrt( tmpLength );
	tmpLengthFrac = std::sqrt( tmpLengthFrac );
	double lengthFrac = tmpLengthFrac/tmpLength;
	wallData.resize(Nw+2,wallData[cell(i).wall(wI)->index()]);
  wallData[cell(i).wall(wI)->index()][0] = lengthFrac*oldL;
  wallData[Nw+1][0] = oldL-wallData[cell(i).wall(wI)->index()][0];
	
	//Wall continuing the second selected wall
  addWall(*(cell(i).wall(w3I)));
  wall(Nw+2).setIndex(Nw+2);
	//Set new lengths as fractions of the old determined from the new
  //vertex position
	oldL = wallData[cell(i).wall(w3I)->index()][0];
	v1w = cell(i).wall(w3I)->vertex1()->index();
	v2w = cell(i).wall(w3I)->vertex2()->index();
	tmpLength=0.0; 
	tmpLengthFrac=0.0; 
	for( size_t d=0 ; d<dimension ; ++d ) {
		tmpLengthFrac += (v2Pos[d]-vertexData[v1w][d])*
			(v2Pos[d]-vertexData[v1w][d]);
		tmpLength += (vertexData[v2w][d]-vertexData[v1w][d])*
			(vertexData[v2w][d]-vertexData[v1w][d]);
	}
	tmpLength = std::sqrt( tmpLength );
	tmpLengthFrac = std::sqrt( tmpLengthFrac );
	lengthFrac = tmpLengthFrac/tmpLength;
  wallData.resize(Nw+3,wallData[cell(i).wall(w3I)->index()]);
	wallData[cell(i).wall(w3I)->index()][0] = lengthFrac*oldL;
	wallData[Nw+2][0] = oldL-wallData[cell(i).wall(w3I)->index()][0];

	//Resize derivative matrix as well
	wallDeriv.resize(Nw+3,wallDeriv[0]);
  
	// Create connection matrix by first selecting vertices and walls for
	// the new and old cells
	//////////////////////////////////////////////////////////////////////
  std::vector<size_t> oldVIndex,newVIndex,oldWIndex,newWIndex,
		usedWIndex( cell(i).numWall() );
	
	//Extract walls and vertices for 'old' cell
	size_t tmpWIndex = cell(i).wall(wI)->index(); 
	size_t tmpVIndex = cell(i).wall(wI)->vertex1()->index();
	size_t nextW = wI;
	//size_t count=0;
	//std::cerr << "wI=" << wI << "(" << cell(i).wall(wI)->index() 
	//				<< ") w3I=" << w3I << "(" << cell(i).wall(w3I)->index() 
	//				<< ")" << std::endl;
	do {
		//std::cerr << count << " " << nextW << "  " << tmpWIndex << " " 
		//				<< tmpVIndex << std::endl;
		//add to new vectors
		oldWIndex.push_back( tmpWIndex );
		oldVIndex.push_back( tmpVIndex );
		//mark as used
		usedWIndex[nextW] = 1;
		//usedVIndex[xxx] = 1;
		//find next wall
		nextW = cell(i).numWall();
		size_t flag=0;
		for( size_t w=0 ; w<cell(i).numWall() ; ++w ) {
			if( !usedWIndex[w] && (cell(i).wall(w)->vertex1()->index()==tmpVIndex ||
														 cell(i).wall(w)->vertex2()->index()==tmpVIndex ) ) {
				nextW = w;
				flag++;
			}
		}
		if( flag != 1 ) {
			std::cerr << "Tissue::divideCell() " << flag  
								<< " walls marked for next wall..." << std::endl;
			std::cerr << tmpVIndex << std::endl;
			for( size_t w=0 ; w<cell(i).numWall() ; ++w ) {
				std::cerr << w << " " << usedWIndex[w] << " " 
									<< cell(i).wall(w)->vertex1()->index() << " "
									<< cell(i).wall(w)->vertex2()->index() << std::endl;
			}
		}
		assert( flag==1 );
		tmpWIndex = cell(i).wall(nextW)->index();
		if( cell(i).wall(nextW)->vertex1()->index()==tmpVIndex )
			tmpVIndex = cell(i).wall(nextW)->vertex2()->index();
		else if( cell(i).wall(nextW)->vertex2()->index()==tmpVIndex )
			tmpVIndex = cell(i).wall(nextW)->vertex1()->index();
		else {
			std::cerr << "Tissue::DivideCell() " 
								<< "Wrong vertex indices for chosen wall " 
								<< cell(i).wall(nextW)->vertex1()->index() << " "
								<< cell(i).wall(nextW)->vertex2()->index() << std::endl;
			exit(-1);
		}
		//std::cerr << count++ << " " << nextW << "  " << tmpWIndex << " " 
		//				<< tmpVIndex << std::endl;
	} while( nextW != w3I ); 
	if( cell(i).wall(nextW)->vertex1()->index()==oldVIndex[oldVIndex.size()-1] )
		oldWIndex.push_back(cell(i).wall(w3I)->index());
	else if( cell(i).wall(nextW)->vertex2()->index()==oldVIndex[oldVIndex.size()-1] )
		oldWIndex.push_back(Nw+2);
	else {
		std::cerr << "Wrong last index for old cell (not in w3I)" << std::endl;
		exit(-1);
	}
	//Extract walls and vertices for 'new' cell
	tmpWIndex = Nw+1;//new copy of wI; 
	tmpVIndex = wall(Nw+1).vertex2()->index();
	//std::cerr << "Old cell done..." << std::endl;
	usedWIndex[wI]=0;
	usedWIndex[w3I]=0;
	nextW=wI;
	do {
		//std::cerr << count << " " << nextW << "  " << tmpWIndex << " " 
		//				<< tmpVIndex << std::endl;
		//add to new vectors
		newWIndex.push_back( tmpWIndex );
		newVIndex.push_back( tmpVIndex );
		//mark as used
		usedWIndex[nextW] = 1;
		//usedVIndex[xxx] = 1;
		//find next wall
		nextW = cell(i).numWall();
		size_t flag=0;
		for( size_t w=0 ; w<cell(i).numWall() ; ++w ) {
			if( !usedWIndex[w] && ( cell(i).wall(w)->vertex1()->index()==tmpVIndex ||
															cell(i).wall(w)->vertex2()->index()==tmpVIndex ) ) {
				nextW = w;
				flag++;
			}
		}
		if( flag != 1 )
			std::cerr << flag << " walls marked for next wall..." << std::endl;
		assert( flag==1 );
		tmpWIndex = cell(i).wall(nextW)->index();
		if( cell(i).wall(nextW)->vertex1()->index()==tmpVIndex )
			tmpVIndex = cell(i).wall(nextW)->vertex2()->index();
		else if( cell(i).wall(nextW)->vertex2()->index()==tmpVIndex )
			tmpVIndex = cell(i).wall(nextW)->vertex1()->index();
		else {
			std::cerr << "Tissue::DivideCell() " 
								<< "Wrong vertex indices for chosen wall " 
								<< cell(i).wall(nextW)->vertex1() << " "
								<< cell(i).wall(nextW)->vertex2() << std::endl;
			exit(-1);
		}
		//std::cerr << count++ << " " << nextW << "  " << tmpWIndex << " " 
		//				<< tmpVIndex << std::endl;
	} while( nextW != w3I ); 
	if( cell(i).wall(nextW)->vertex1()->index()==newVIndex[newVIndex.size()-1] )
		newWIndex.push_back(cell(i).wall(w3I)->index());
	else if( cell(i).wall(nextW)->vertex2()->index()==newVIndex[newVIndex.size()-1] )
		newWIndex.push_back(Nw+2);
	else {
		std::cerr << "Wrong last index for new cell (not in w3I)" << std::endl;
		exit(-1);
	}
	
	//std::cerr << "New cell done..." << std::endl;
	
  //Add new vertices
  oldVIndex.push_back(Nv);
  oldVIndex.push_back(Nv+1);
  newVIndex.push_back(Nv);
  newVIndex.push_back(Nv+1);
  
  //Add new walls
  oldWIndex.push_back(Nw);
  newWIndex.push_back(Nw);
  //newWIndex.push_back(Nw+1);
  //newWIndex.push_back(Nw+2);
  
	//std::cerr << "Cell " << i << " has " << cell(i).numWall() << " walls and " 
	//				<< cell(i).numVertex() << " vertices" << std::endl;
// 	for( size_t w=0 ; w<cell(i).numWall() ; ++w )
// 		std::cerr << cell(i).wall(w)->index() << " ";
// 	std::cerr << std::endl;
// 	for( size_t v=0 ; v<cell(i).numVertex() ; ++v )
// 		std::cerr << cell(i).vertex(v)->index() << " ";
// 	std::cerr << std::endl;
	
// 	for( size_t k=0 ; k<oldWIndex.size() ; ++k )
// 		std::cerr << oldWIndex[k] << " ";
// 	std::cerr << "  ";
// 	for( size_t k=0 ; k<newWIndex.size() ; ++k )
// 		std::cerr << newWIndex[k] << " ";
// 	std::cerr << std::endl;
	
// 	for( size_t k=0 ; k<oldVIndex.size() ; ++k )
// 		std::cerr << oldVIndex[k] << " ";
// 	std::cerr << "  ";
// 	for( size_t k=0 ; k<newVIndex.size() ; ++k )
// 		std::cerr << newVIndex[k] << " ";
// 	std::cerr << std::endl;
	//exit(0);
	
  //Set vertices and cells for the walls
  wall(Nw).setVertex( &(vertex(Nv)),&(vertex(Nv+1)) );
  wall(Nw).setCell( &(cell(i)),&(cell(Nc)) );
  assert( wall(Nw).cell1()->index()==i );
  assert( wall(Nw).cell2()->index()==Nc );
	
  wall(cell(i).wall(wI)->index()).setVertex2( &(vertex(Nv)) );
  int vInCellFlag=0;
  for( size_t k=0 ; k<oldVIndex.size() ; ++k )
    if( oldVIndex[k]==cell(i).wall(wI)->vertex1()->index() ) 
      vInCellFlag++;
  assert( vInCellFlag==1 );
  
  wall(Nw+1).setVertex1( &(vertex(Nv)) );
  vInCellFlag=0;
  for( size_t k=0 ; k<newVIndex.size() ; ++k )
    if( newVIndex[k]==wall(Nw+1).vertex2()->index() ) 
      vInCellFlag++;
  assert( vInCellFlag==1 );
	
  if( wall(Nw+1).cell1()->index() == i ) {
    wall(Nw+1).setCell1( &(cell(Nc)) );
    if( wall(Nw+1).cell2()->index() < Nc ) {
      wall(Nw+1).cell2()->addWall( &(wall(Nw+1)) );
      wall(Nw+1).cell2()->addVertex( &(vertex(Nv)) );
    }
  }
  else if( wall(Nw+1).cell2()->index() == i ) {
    wall(Nw+1).setCell2( &(cell(Nc)) );
    if( wall(Nw+1).cell1()->index() < Nc ) {
      wall(Nw+1).cell1()->addWall( &(wall(Nw+1)) );
      wall(Nw+1).cell1()->addVertex( &(vertex(Nv)) );
    }
  }
  else {
    std::cerr << "Tissue::divideCell() "
							<< "First wall not connected to dividing cell" << std::endl;
    std::cerr << i << " " << cell(i).index() << "\t" 
							<< wall( cell(i).wall(wI)->index() ).cell1()->index() << " "
							<< wall(Nw+1).cell1()->index() << "\t"
							<< wall(cell(i).wall(wI)->index()).cell2()->index() << " "
							<< wall(Nw+1).cell2()->index() << "\n";		
    exit(-1);
  }
  
	vInCellFlag=0;
	size_t wInOldCellFlag=0;
  for( size_t k=0 ; k<oldWIndex.size() ; ++k )
		if( cell(i).wall(w3I)->index()==oldWIndex[k] )
			wInOldCellFlag=1;
	if( wInOldCellFlag ) {
		for( size_t k=0 ; k<oldVIndex.size() ; ++k )
			if( oldVIndex[k]==wall(cell(i).wall(w3I)->index()).vertex1()->index() ) 
				vInCellFlag++;
		if( vInCellFlag == 1 ) {
			wall(cell(i).wall(w3I)->index()).setVertex2( &(vertex(Nv+1)) );
			wall(Nw+2).setVertex1( &(vertex(Nv+1)) );
		}
		else {
			wall(cell(i).wall(w3I)->index()).setVertex1( &(vertex(Nv+1)) );
			wall(Nw+2).setVertex2( &(vertex(Nv+1)) );
		}
	}
	else {		
		for( size_t k=0 ; k<oldVIndex.size() ; ++k )
			if( oldVIndex[k]==wall(Nw+2).vertex1()->index() ) 
				vInCellFlag++;
		if( vInCellFlag == 1 ) {
			wall(Nw+2).setVertex2( &(vertex(Nv+1)) );
			wall(cell(i).wall(w3I)->index()).setVertex1( &(vertex(Nv+1)) );
		}
		else {
			wall(Nw+2).setVertex1( &(vertex(Nv+1)) );
			wall(cell(i).wall(w3I)->index()).setVertex2( &(vertex(Nv+1)) );
		}
	}

	//Change cell connection for new wall (w3I or Nw+2)
	size_t newWallIndex = cell(i).wall(w3I)->index();
	size_t w3IInNewCellFlag = 0;
	for( size_t w=0 ; w<newWIndex.size() ; ++w )
		if( newWIndex[w]==newWallIndex )
			w3IInNewCellFlag++;
	if( !w3IInNewCellFlag++ )
		newWallIndex=Nw+2;
	
	if( wall(newWallIndex).cell1()->index() == i ) {
    wall(newWallIndex).setCell1( &(cell(Nc)) );
    if( wall(newWallIndex).cell2()->index() < Nc ) {
      wall(newWallIndex).cell2()->addWall( &(wall(Nw+2)) );
      wall(newWallIndex).cell2()->addVertex( &(vertex(Nv+1)) );
    }
  }    
  else if( wall(newWallIndex).cell2()->index() == i ) {
    wall(newWallIndex).setCell2( &(cell(Nc)) );
    if( wall(newWallIndex).cell1()->index() < Nc ) {
      wall(newWallIndex).cell1()->addWall( &(wall(Nw+2)) );
      wall(newWallIndex).cell1()->addVertex( &(vertex(Nv+1)) );
    }    
  }
  else {
    std::cerr << "Tissue::divideCell() "
							<< "Second wall not connected to dividing cell" << std::endl;
    for( size_t k=0 ; k<cell(i).numWall() ; ++k ) {
      size_t v1w3Itmp = cell(i).wall(k)->vertex1()->index();
      size_t v2w3Itmp = cell(i).wall(k)->vertex2()->index();
      std::cerr << vertexData[v1w3Itmp][0] << " " 
								<< vertexData[v1w3Itmp][1] << " 4\n"
								<< vertexData[v2w3Itmp][0] << " "
								<< vertexData[v2w3Itmp][1] << " 4\n\n\n";
    }
		exit(-1);
  }
  
  //Set cells and walls for the vertices
	//////////////////////////////////////////////////////////////////////
  for( size_t v=0 ; v<newVIndex.size() ; ++v ) {
    std::vector<Cell*> tmpCell;
    std::vector<Wall*> tmpWall;
    if( newVIndex[v]==Nv ) {
			//First new vertex
      tmpCell.push_back( &(cell(i)) );
      tmpCell.push_back( &(cell(Nc)) );
			//plus one more
      if( cell(i).wall(wI)->cell1()->index()==i &&
					cell(i).wall(wI)->cell2() != background() )
				tmpCell.push_back( cell(i).wall(wI)->cell2() );
      else if( cell(i).wall(wI)->cell2()->index()==i &&
							 cell(i).wall(wI)->cell1() != background() )
				tmpCell.push_back( cell(i).wall(wI)->cell1() );
      else if( cell(i).wall(wI)->cell1()->index() != i &&
							 cell(i).wall(wI)->cell2()->index() != i ){
				std::cerr << "Tissue::divideCell() "
									<< "Wall wI not connected to dividing cell" 
									<< std::endl;
				exit(-1);
      }
      tmpWall.push_back( cell(i).wall(wI) );
      tmpWall.push_back( &(wall(Nw)) );
      tmpWall.push_back( &(wall(Nw+1)) );
    }
    else if( newVIndex[v]==Nv+1 ) {
			//Second new vertex
      tmpCell.push_back( &(cell(i)) );
      tmpCell.push_back( &(cell(Nc)) );
      //plus one more
      if( cell(i).wall(w3I)->cell1()->index()==i ||
					cell(i).wall(w3I)->cell1()->index()==Nc ) {
				if( cell(i).wall(w3I)->cell2() != background() )
					tmpCell.push_back( cell(i).wall(w3I)->cell2() );
			}
			else if( cell(i).wall(w3I)->cell2()->index()==i ||
							 cell(i).wall(w3I)->cell2()->index()==Nc ) {
				if( cell(i).wall(w3I)->cell1() != background() )
					tmpCell.push_back( cell(i).wall(w3I)->cell1() );
			}
      else {
				std::cerr << "Tissue::divideCell() "
									<< "Wall w3I not connected to dividing cell" 
									<< std::endl;
				exit(-1);
      }
      tmpWall.push_back( cell(i).wall(w3I) );
      tmpWall.push_back( &(wall(Nw)) );
      tmpWall.push_back( &(wall(Nw+2)) );
    }
    else {
			//For other vertices
      for( size_t c=0 ; c<vertex(newVIndex[v]).numCell() ; ++c )
				if( vertex(newVIndex[v]).cell(c)->index() == i )
					vertex(newVIndex[v]).setCell(c,&(cell(Nc)));
      for( size_t w=0 ; w<vertex(newVIndex[v]).numWall() ; ++w ) {
				if( vertex(newVIndex[v]).wall(w)->index() == 
						cell(i).wall(wI)->index() )
					vertex(newVIndex[v]).setWall(w,&(wall(Nw+1)));
      	else if( vertex(newVIndex[v]).wall(w)->index() == 
								 cell(i).wall(w3I)->index() ) {
					size_t w3IInNewFlag=0;
					for( size_t ww=0 ; ww<newWIndex.size() ; ++ww )
						if( newWIndex[ww] == cell(i).wall(w3I)->index() )
							w3IInNewFlag++;
					if( !w3IInNewFlag )
						vertex(newVIndex[v]).setWall(w,&(wall(Nw+2)));
				}
      }
    }
    if( newVIndex[v]>=Nv ) {
      vertex(newVIndex[v]).setCell(tmpCell);
      vertex(newVIndex[v]).setWall(tmpWall);
    }
  }

	//Finally check if vertices in old cell connected to w3I should be changed
	//to Nw+2
  for( size_t v=0 ; v<oldVIndex.size() ; ++v ) {
		if( oldVIndex[v] < Nv ) {
      for( size_t w=0 ; w<vertex(oldVIndex[v]).numWall() ; ++w ) {
      	if( vertex(oldVIndex[v]).wall(w)->index() == 
						cell(i).wall(w3I)->index() ) {
					size_t w3IInOldFlag=0;
					for( size_t ww=0 ; ww<oldWIndex.size() ; ++ww )
						if( oldWIndex[ww] == cell(i).wall(w3I)->index() )
							w3IInOldFlag++;
					if( !w3IInOldFlag )
						vertex(oldVIndex[v]).setWall(w,&(wall(Nw+2)));
				}
      }
		}
	}
  
  //Set walls and vertices for the cells
  //This should be done after the vertex and wall insertion not to
  //mess up wI and w3I
  std::vector<Wall*> tmpW(oldWIndex.size());
  std::vector<Vertex*> tmpV(oldVIndex.size());
  for( size_t w=0 ; w<oldWIndex.size() ; ++w )
    tmpW[w] = &(wall(oldWIndex[w]));
  for( size_t v=0 ; v<oldVIndex.size() ; ++v )
    tmpV[v] = &(vertex(oldVIndex[v]));
  cell(i).setWall(tmpW);
  cell(i).setVertex(tmpV);
  
  tmpW.resize( newWIndex.size() );
  tmpV.resize( newVIndex.size() );
  for( size_t w=0 ; w<newWIndex.size() ; ++w ) {
    tmpW[w] = &(wall(newWIndex[w]));
    if( tmpW[w]->index() != Nw && tmpW[w]->cell1()->index()==i )
      tmpW[w]->setCell1( &(cell(Nc)) );
    else if( tmpW[w]->index() != Nw && tmpW[w]->cell2()->index()==i )
      tmpW[w]->setCell2( &(cell(Nc)) );
  }
  for( size_t v=0 ; v<newVIndex.size() ; ++v )
    tmpV[v] = &(vertex(newVIndex[v]));
  cell(Nc).setWall(tmpW);
  cell(Nc).setVertex(tmpV);
	assert( wall(Nw).cell1()->index()==i );
  assert( wall(Nw).cell2()->index()==Nc );
	
	//Update directional wall vector if applicable
	updateDirectionDivision(i,cellData,wallData,vertexData,
													cellDeriv,wallDeriv,vertexDeriv);

	// Update the volume dependent variables for each cell variable index 
	// given in volumeChangeList
	if (volumeChangeList.size()) {
		cell(i).sortWallAndVertex(*this);
		cell(Nc).sortWallAndVertex(*this);
		double Vi = cell(i).calculateVolume(vertexData);
		double Vn = cell(Nc).calculateVolume(vertexData);
		double fi = Vi/(Vi+Vn);
		double fn = Vn/(Vi+Vn);
		
		for (size_t k=0; k<volumeChangeList.size(); ++k) {
			cellData[i][volumeChangeList[k]] *= fi;
			cellData[Nc][volumeChangeList[k]] *= fn;
		}
	}		
	//checkConnectivity(1);
}

void Tissue::removeTwoVertex( size_t index ) 
{
	if (vertex(index).numWall() != 2) {
		std::cerr << "Tissue::removeTwoVertex() Vertex not a two-vertex, not removed!" 
							<< std::endl;
		return;
	}
	Vertex* v=vertexP(index);
	Wall* w1=vertex(index).wall(0);
	Wall* w2=vertex(index).wall(1);
	assert(v->numCell()==1 || v->numCell()==2);
	Cell* c1=vertex(index).cell(0);
	Cell* c2;
	if (v->numCell()==2)
		c2=vertex(index).cell(1);
	else
		c2=background();
	// Get 'neighboring' vertices from walls
	Vertex* v1=w1->vertex1();
	if (v1==v)
		v1=w1->vertex2();
	Vertex* v2=w2->vertex1();
	if (v2==v)
		v2=w2->vertex2();
	
	// Remove v,w2 from c1 and c2 (if not background)
	if (c1!=background()) {
		size_t numW=c1->numWall();
		size_t numV=c1->numVertex();
		assert(numW>3 && numW==numV);
		std::vector<Vertex*> newV(numV-1);
		std::vector<Wall*> newW(numV-1);
		size_t vI=0;
		size_t wI=0;
		for (size_t i=0; i<numV; ++i) {
			if (c1->vertex(i) != v)
				newV[vI++]=c1->vertex(i);
			if (c1->wall(i) != w2)
				newW[wI++]=c1->wall(i);
		}
		if (vI!=numV-1 || wI!=numW-1) {
			std::cerr << "Tissue::removeTwoVertex Vertex or wall to be removed not found in cell."
								<< std::endl;
			std::cerr << "Cell: " << c1->index() << " NumVertex(wall): " << c1->numVertex() << " ("
								<< c1->numWall() << ") Vertex: " << v->index() << " Wall: "
								<< w2->index() << " vI: " << vI << " wI: " << wI << std::endl;
			exit(-1);
		}
		c1->setVertex(newV);
		c1->setWall(newW);
	}		
	if (c2!=background()) {
		size_t numW=c2->numWall();
		size_t numV=c2->numVertex();
		assert(numW>3 && numW==numV);
		std::vector<Vertex*> newV(numV-1);
		std::vector<Wall*> newW(numV-1);
		size_t vI=0;
		size_t wI=0;
		for (size_t i=0; i<numV; ++i) {
			if (c2->vertex(i) != v)
				newV[vI++]=c2->vertex(i);
			if (c2->wall(i) != w2)
				newW[wI++]=c2->wall(i);
		}
		if (vI!=numV-1 || wI!=numW-1) {
			std::cerr << "Tissue::removeTwoVertex Vertex or wall to be removed not found in cell."
								<< std::endl;
			std::cerr << "Cell: " << c2->index() << " NumVertex(wall): " << c2->numVertex() << " ("
								<< c2->numWall() << ") Vertex: " << v->index() << " Wall: "
								<< w2->index() << " vI: " << vI << " wI: " << wI << std::endl;
			exit(-1);
		}
		c2->setVertex(newV);
		c2->setWall(newW);
	}		
	// Update w1
	if (w1->vertex1()==v && w1->vertex2()==v1)
		w1->setVertex1(v2);
	else if (w1->vertex2()==v && w1->vertex1()==v1)
		w1->setVertex2(v2);
	else {
		std::cerr << "Tissue::removeTwoVertex() Wrong in updating wall." << std::endl; 
		exit(-1);
	}
	// update w1 length?
	// Update v2 to connect to w1 instead of w2
	size_t numW=v2->numWall();
	size_t updateFlag=0;
	for (size_t i=0; i<numW; ++i)
		if (v2->wall(i)==w2) {
			v2->setWall(i,w1);
			++updateFlag;
		}
	if (updateFlag!=1) {
		std::cerr << "Tissue::removeTwoVertex() Update of v2 wrong." << std::endl;
		exit(-1);
	}
	// Remove v and w2
	removeVertex(v->index());
	removeWall(w2->index());
}

void Tissue::sortCellWallAndCellVertex(Cell *cell) 
{	
	std::cerr << "Tissue::sortCellWallAndCellVertex()" << std::endl;
	std::vector<size_t> sortedFlag(numCell());
	size_t numSorted=0;
	if( !cell )		
		cell = this->cellP(0);
	sortCellRecursive(cell, sortedFlag, numSorted);
	//std::cerr << "Tissue::sortCellWallAndCellVertex() " << numSorted << " of " << numCell()
	//				<< " sorted recursively." << std::endl;
}

void Tissue::sortCellRecursive( Cell* cell, std::vector<size_t> &sortedFlag, size_t &numSorted)
{
	if (sortedFlag[cell->index()])
		return;
	cell->sortWallAndVertex(*this);
	sortedFlag[cell->index()]++;
	numSorted++;
	for (size_t k=0; k<cell->numWall(); ++k) {
		Cell *cellNext = cell->cellNeighbor(k);
		if (cellNext!=background())
			sortCellRecursive(cellNext,sortedFlag,numSorted);
	}
	return;
}

void Tissue::checkConnectivity(size_t verbose) 
{	
  int exitFlag=0;
	size_t numC = numCell();
  // Check if all indices are used
  //
  for (size_t i=0; i<numC; ++i) {
    if( verbose ) {
      if( cell(i).index() != i ) {
				std::cerr << "Tissue::checkConnectivity() "
									<< "Cell " << i << " has index " << cell(i).index()
									<< std::endl;
				exitFlag++;
      }
    }
    else
      assert( cell(i).index() == i );
  }
  for (size_t i=0; i<numWall(); ++i) {
    if( verbose ) {
      if( wall(i).index() != i ) {
				std::cerr << "Tissue::checkConnectivity() "
									<< "Wall " << i << " has index " << wall(i).index()
									<< std::endl;
				exitFlag++;
      }
    }
    else
      assert( wall(i).index() == i );
  }
  for (size_t i=0; i<numVertex(); ++i) {
    if( verbose ) {
      if( vertex(i).index() != i ) {
				std::cerr << "Tissue::checkConnectivity() "
									<< "Vertex " << i << " has index " << vertex(i).index()
									<< std::endl;
				exitFlag++;
      }
    }
    else
      assert( vertex(i).index() == i );
  }
  // Make sure all cellVertex(Wall) are real vertices(walls) via index
  //
  for( size_t k=0 ; k<numC ; ++k ) {
    for( size_t l=0 ; l<cell(k).numWall() ; ++l ) { 
      if( verbose ) {
				if( cell(k).wall(l)->index()>=numWall() ) {
					std::cerr << "Tissue::checkConnectivity() " << "Cell " << k 
										<< " is connected to wall "
										<< cell(k).wall(l)->index() << "("
										<< numWall() << " walls in total)" << std::endl;
					exitFlag++;
				}
      }
      else {
				assert( cell(k).wall(l)->index()<numWall() );
      }
    }
    for( size_t l=0 ; l<cell(k).numVertex() ; ++l ) { 
      if( verbose ) {
				if( cell(k).vertex(l)->index()>=numVertex() ) {
					std::cerr << "Tissue::checkConnectivity() " << "Cell " << k 
										<< " is connected to vertex "
										<< cell(k).vertex(l)->index() << "("
										<< numVertex() << " vertices in total)" << std::endl;
					exitFlag++;
				}
				for( size_t ll=l+1 ; ll<cell(k).numVertex() ; ++ll ) { 
					if( cell(k).vertex(l)==cell(k).vertex(ll) ) {
						std::cerr << "Tissue::checkConnectivity() " << "Cell " << k 
											<< " is connected to vertex "
											<< cell(k).vertex(l)->index() << "(twice)"
											<< std::endl;
						exitFlag++;
					}
				}
      }
      else {
				assert( cell(k).vertex(l)->index()<numVertex() );
      }
    }
  }
  //Make sure all wallVertex(Cell) are real vertices(cells) via index
  //////////////////////////////////////////////////////////////////////
  for( size_t k=0 ; k<numWall() ; ++k ) {
    if( verbose ) {
      if( ( wall(k).cell1()->index()>=numCell() &&
						wall(k).cell1() != background() ) ||
					( wall(k).cell2()->index()>=numCell() &&
						wall(k).cell2() != background() ) ) {
				std::cerr << "Tissue::checkConnectivity() " << "Wall " << k 
									<< " is connected to cell "
									<< wall(k).cell1()->index() << " and "
									<< wall(k).cell2()->index() << " ("
									<< numCell() << " cells in total)" << std::endl;
				exitFlag++;
      }
      if( wall(k).cell1() == wall(k).cell2() ) {
				std::cerr << "Tissue::checkConnectivity() " << "Wall " << k 
									<< " is connected to cell "
									<< wall(k).cell1()->index() << " and "
									<< wall(k).cell2()->index() << " (same cell)" 
									<< std::endl;
				exitFlag++;
      }
    }
    else {
      assert( ( wall(k).cell1()->index()<numCell() ||
								wall(k).cell1() == background() ) &&
							( wall(k).cell2()->index()<numCell() ||
								wall(k).cell2() == background() ) );
      assert( wall(k).cell1() != wall(k).cell2() );
    }
    if( verbose ) {
      if( wall(k).vertex1()->index()>=numVertex() ||
					wall(k).vertex2()->index()>=numVertex() ) {
				std::cerr << "Tissue::checkConnectivity() " << "Wall " << k 
									<< " is connected to vertex "
									<< wall(k).vertex1()->index() << " and "
									<< wall(k).vertex2()->index() << " ("
									<< numVertex() << " vertices in total)" << std::endl;
				exitFlag++;
      }
      if( wall(k).vertex1() == wall(k).vertex2() ) {
				std::cerr << "Tissue::checkConnectivity() " << "Wall " << k 
									<< " is connected to vertex "
									<< wall(k).vertex1()->index() << " and "
									<< wall(k).vertex2()->index() << " (same vertex)" 
									<< std::endl;
				exitFlag++;
      }
    }
    else {
      assert( wall(k).vertex1()->index()<numVertex() &&
							wall(k).vertex2()->index()<numVertex() );			
      assert( wall(k).vertex1() != wall(k).vertex2() );
    }
  }
  
  //Make sure all vertexCell(Wall) are real cells(walls) via index
  //////////////////////////////////////////////////////////////////////
  for( size_t k=0 ; k<numVertex() ; ++k ) {
    for( size_t l=0 ; l<vertex(k).numCell() ; ++l ) { 
      if( verbose ) {
				if( vertex(k).cell(l)->index()>=numCell() ) {
					std::cerr << "Tissue::checkConnectivity() " << "Vertex " << k 
										<< " is connected to cell "
										<< vertex(k).cell(l)->index() << "("
										<< numCell() << " cells in total)" << std::endl;
					exitFlag++;
				}
				if( vertex(k).cell(l) == background() ) {
					std::cerr << "Tissue::checkConnectivity() " 
										<< "Vertex " << k << " is connected to background"
										<< std::endl;
					exitFlag++;
				}
				for( size_t ll=l+1 ; ll<vertex(k).numCell() ; ++ll ) { 
					if( vertex(k).cell(l)==vertex(k).cell(ll) ) {
						std::cerr << "Tissue::checkConnectivity() " << "Vertex " << k 
											<< " is connected to cell "
											<< vertex(k).cell(l)->index() << " twice."
											<< std::endl;
						exitFlag++;
					}
				}		
      }
      else {
				assert( vertex(k).cell(l)->index()<numCell() );
				assert( vertex(k).cell(l) != background() );
      }
    }
    for( size_t l=0 ; l<vertex(k).numWall() ; ++l ) { 
      if( verbose ) {
				if( vertex(k).wall(l)->index()>=numWall() ) {
					std::cerr << "Tissue::checkConnectivity() " << "Vertex " << k 
										<< " is connected to wall "
										<< vertex(k).wall(l)->index() << "("
										<< numWall() << " walls in total)" << std::endl;
					exitFlag++;
				}
				for( size_t ll=l+1 ; ll<vertex(k).numWall() ; ++ll ) { 
					if( vertex(k).wall(l)==vertex(k).wall(ll) ) {
						std::cerr << "Tissue::checkConnectivity() " << "Vertex " << k 
											<< " is connected to wall "
											<< vertex(k).wall(l)->index() << " twice."
											<< std::endl;
						exitFlag++;
					}
				}		
      }
      else {
				assert( vertex(k).wall(l)->index()<numWall() );
      }
    }
  }
  
  // Make sure all compartments include same set of variables
	//
  size_t numVarTmp=cell(0).numVariable();
  for (size_t i=1; i<numC; ++i) {
    if( verbose ) {
      if( numVarTmp != cell(i).numVariable() ) {
				std::cerr << "Tissue::checkConnectivity() " << "Cell " << i 
									<< " has " << cell(i).numVariable() << " variables" 
									<< " while cell 0 has " << numVarTmp << std::endl;
				exitFlag++;
      }
    }
    else
      assert( numVarTmp==cell(i).numVariable() );
  }
  // Do checks on connectivity from cells
  //////////////////////////////////////////////////////////////////////
  for (size_t i=0; i<numC; ++i) {
    
    if( verbose ) {
      if ( cell(i).numWall() != cell(i).numVertex() ) {
				std::cerr << "Tissue::checkConnectivity() "
									<< "Cell " << i << " has " << cell(i).numWall()
									<< " walls and " << cell(i).numVertex() 
									<< " vertices!" << std::endl; 
				exitFlag++;
      }
    }
    else
      assert( cell(i).numWall() == cell(i).numVertex() );
    
    //Make sure that vertecis in all cell-walls are cell-vertices
    for (size_t w=0 ; w<cell(i).numWall(); ++w) {
      if ( verbose ) {
				if ( !cell(i).hasVertex( cell(i).wall(w)->vertex1() ) ) {
					std::cerr << "Tissue::checkConnectivity() "
										<< "Cell " << i << " has wall " << cell(i).wall(w)->index()
										<< " with vertex " << cell(i).wall(w)->vertex1()->index()
										<< " but is not connected to the vertex!"
										<< std::endl;
					exitFlag++;
				}
				if ( !cell(i).hasVertex( cell(i).wall(w)->vertex2() ) ) {
					std::cerr << "Tissue::checkConnectivity() "
										<< "Cell " << i << " has wall " << cell(i).wall(w)->index() 
										<< " with vertex " << cell(i).wall(w)->vertex2()->index()
										<< " but is not connected to the vertex!"
										<< std::endl;
					exitFlag++;
				}				
      }
      else {
				assert( cell(i).hasVertex( cell(i).wall(w)->vertex1() ) );
				assert( cell(i).hasVertex( cell(i).wall(w)->vertex2() ) );
      }
    }
    //Make sure that two walls in all cell-vertices are cell-walls
    for (size_t v=0 ; v<cell(i).numVertex(); ++v) {
      int numWall=0;
      for (size_t w=0 ; w<cell(i).vertex(v)->numWall(); ++w) {
				numWall += cell(i).hasWall( cell(i).vertex(v)->wall(w) );
      }
      if ( verbose ) {
				if( numWall != 2 ) {
					std::cerr << "Tissue::checkConnectivity() "
										<< "Cell " << i << " has vertex " << cell(i).vertex(v)->index() 
										<< " with " << cell(i).vertex(v)->numWall() 
										<< " walls but " << numWall 
										<< " walls are connected to the cell!"
										<< std::endl;
					exitFlag++;
				}								
      }
      else {
				assert( numWall==2 );
      }
    }
  }//for i

	// Check that walls and vertices are properly sorted in cells
  for (size_t i=0; i<numC; ++i) {
		Cell* cP = cellP(i);
		size_t numW=cP->numWall();
		for (size_t k=0; k<numW; ++k) {
			size_t kPlus = (k+1)%numW;
			if (cP->wall(k)->cell1()==cP) {
				if (cP->wall(k)->cellSort1()==-1) {
					if (cP->vertex(kPlus) != cP->wall(k)->vertex1() ||
							cP->vertex(k) != cP->wall(k)->vertex2() ) {
						std::cerr << "Tissue::checkConnectivity() "
											<< "1: vertices and walls not sorted correctly in cell " 
											<< i << " wall " << cP->wall(k)->index() << std::endl;
						++exitFlag;
					}
				}
				else { //cellSort=1 or cellSort=0
					if (cP->vertex(k) != cP->wall(k)->vertex1() ||
							cP->vertex(kPlus) != cP->wall(k)->vertex2() ) {
						std::cerr << "Tissue::checkConnectivity() "
											<< "2: vertices and walls not sorted correctly in cell "
											<< i << " wall " << cP->wall(k)->index() << std::endl;
						++exitFlag;
					}
				}
			}
			else if (cP->wall(k)->cell2()==cP) {
				if (cP->wall(k)->cellSort2()==-1) {
					if (cP->vertex(kPlus) != cP->wall(k)->vertex1() ||
							cP->vertex(k) != cP->wall(k)->vertex2() ) {
						std::cerr << "Tissue::checkConnectivity() "
											<< "3: vertices and walls not sorted correctly in cell "
											<< i << " wall " << cP->wall(k)->index() << std::endl;
						++exitFlag;
					}
				}
				else { //cellSort=1 or cellSort=0
					if (cP->vertex(k) != cP->wall(k)->vertex1() ||
							cP->vertex(kPlus) != cP->wall(k)->vertex2() ) {
						std::cerr << "Tissue::checkConnectivity() "
											<< "4: vertices and walls not sorted correctly in cell "
											<< i << " wall " << cP->wall(k)->index() << std::endl;
						++exitFlag;
					}
				}
			}
			else {
				std::cerr << "Tissue::checkConnectivity() "
									<< "cellWall not connected to cell." << std::endl; 
				++exitFlag;
			}
		}
	}
	
  if ( exitFlag ) {
		std::cerr << "Tissue::checkConnectivity() "
							<< exitFlag << " errors found in tissue." << std::endl;
    exit(-1);
	}
}

unsigned int Tissue::
findPeaksGradientAscent( std::vector< std::vector<double> > &cellData, 
												 size_t col, std::vector<size_t> &cellMax,
												 std::vector<size_t> &flag )
{
  assert(cellData.size() == numCell() );
  assert( cellData[0].size()>col );
  
  if( cellMax.size() )
    cellMax.resize(0);
  if( flag.size() != numCell() ) flag.resize(numCell());
  for( size_t i=0 ; i<numCell() ; ++i )
    flag[i]=0;
  
  std::vector<size_t> cellTmp;//Values before threshold check
  std::vector<unsigned int> numTmp;//times cellTmp been visited
  std::vector<size_t> walkTmp;//positions for a walk (start point)
  
  size_t count=1;
  //Find the maxima from each cell
  for( size_t iStart=0 ; iStart<numCell() ; ++iStart ) {
    size_t i=iStart;
    double value,newValue;
    walkTmp.resize(1);
    walkTmp[0]=i;
    //find the max by walking uphill (greedy)
    if( !flag[i] ) {
      do {
				newValue=value=cellData[i][col];
				size_t newI=i;
				//Check all neighboring cells
				for(size_t k=0 ; k<cell(i).numWall() ; k++ ) {
					size_t j = cell(i).wall(k)->cell1()->index();
					if( j==i )
						j = cell(i).wall(k)->cell2()->index();
					if( j != background()->index() ) {
						if( cellData[j][col]>newValue ) {
							newValue=cellData[j][col];
							newI=j;
						}
					}
				}
				i=newI;
				walkTmp.push_back( i );
      } while( newValue>value && !flag[i] );
    }
    //Collect the path data and add one visit for the maximum
    if( !flag[i] ) { //new maximum
      cellTmp.push_back( i );
      numTmp.push_back(1);
      unsigned int n=count++;//cellTmp.size();
      for( size_t a=0 ; a<walkTmp.size() ; a++ )
				flag[ walkTmp[a] ] = n;
    }
    else { //old maximum or background
      size_t n = flag[i];
      for( size_t a=0 ; a<walkTmp.size() ; a++ )
				flag[ walkTmp[a] ] = n;
      if( flag[i]>0 )//old maxima
				numTmp[n-1]++;
    }
  }
  //No threshold checking...
  unsigned int threshold = 1;
  double valThreshold = 0.0;
  //Get the maxima visited more than threshold times and with an intensity
  //value higher than threshold
  std::vector<int> clusterNum;
  for( size_t n=0 ; n<cellTmp.size() ; n++ )
    if( numTmp[n]>=threshold && cellData[ cellTmp[n] ][col]>valThreshold ) {
      cellMax.push_back( cellTmp[n] ); 
      clusterNum.push_back( n+1 );
    }
  
  //Save the basins of attraction
  //   boa.resize( cellMax.size() );
  //   for( int i=0 ; i<numCompartment() ; i++ )
  //     for( int n=0 ; n<cellMax.size() ; n++ )
  //       if( flag[i] == clusterNum[n] )
  // 	boa[n].push_back(i);
  
  return static_cast<unsigned int>(cellMax.size());
}

void Tissue::printInit(std::ostream &os) {
  
	// Increase resolution to max for doubles
	unsigned int oldPrecision = os.precision(); 
	os.precision(15);
	//std::cerr << "Tissue::prinitInit(): old precision: " << oldPrecision << " new " 
	//				<< os.precision() << std::endl;
	
  os << numCell() << " " << numWall() << " " << numVertex() << std::endl;

  //Print the connectivity from walls
  for( size_t i=0 ; i<numWall() ; ++i ) {
    os << i << " ";
		if( wall(i).cell1()->index()<numCell() )
			os << wall(i).cell1()->index() << " " ;
		else
			os << "-1 ";
		if( wall(i).cell2()->index()<numCell() )
			os << wall(i).cell2()->index() << " ";
		else
			os << "-1 ";
		os << wall(i).vertex1()->index() 
			 << " " << wall(i).vertex2()->index() << std::endl;
	}
	os << std::endl;
  
  //Print the vertex positions
  os << numVertex() << " " << vertex(0).numPosition() << std::endl;
  for( size_t i=0 ; i<numVertex() ; ++i ) {
    for( size_t j=0 ; j<vertex(i).numPosition() ; ++j )
      os << vertex(i).position(j) << " ";
    os << std::endl;
  }
  os << std::endl;
  
  //Print wall data
  os << numWall() << " 1 " << wall(0).numVariable() << std::endl;
  for( size_t i=0 ; i<numWall() ; ++i ) {
    os << wall(i).length() << " ";
    for( size_t j=0 ; j<wall(i).numVariable() ; ++j )
      os << wall(i).variable(j) << " ";
    os << std::endl;
  }
  os << std::endl;

  //Print cell data
  os << numCell() << " " << cell(0).numVariable() << std::endl;
  if( cell(0).numVariable() ) {
    for( size_t i=0 ; i<numCell() ; ++i ) {
      for( size_t j=0 ; j<cell(i).numVariable() ; ++j )
				os << cell(i).variable(j) << " ";
      os << std::endl;
    }
    os << std::endl;
  }  
	os.precision(oldPrecision);		
}

void Tissue::printInit(std::vector< std::vector<double> > &cellData,
											 std::vector< std::vector<double> > &wallData,
											 std::vector< std::vector<double> > &vertexData,
											 std::ostream &os) {
  
	assert( numCell()==cellData.size() && 
					numWall()==wallData.size() &&
					numVertex()==vertexData.size() );
	
	// Increase resolution to max for doubles
	unsigned int oldPrecision = os.precision(); 
	os.precision(15);
	//std::cerr << "Tissue::prinitInit(): old precision: " << oldPrecision << " new " 
	//				<< os.precision() << std::endl;
	
  os << numCell() << " " << numWall() << " " << numVertex() << std::endl;
	
  // Print the connectivity from walls
  for( size_t i=0 ; i<numWall() ; ++i ) {
    os << i << " ";
		if( wall(i).cell1()->index()<numCell() )
			os << wall(i).cell1()->index() << " " ;
		else
			os << "-1 ";
		if( wall(i).cell2()->index()<numCell() )
			os << wall(i).cell2()->index() << " ";
		else
			os << "-1 ";
		os << wall(i).vertex1()->index() 
			 << " " << wall(i).vertex2()->index() << std::endl;
	}
  os << std::endl;
  
  //Print the vertex positions
  os << numVertex() << " " << vertex(0).numPosition() << std::endl;
  for( size_t i=0 ; i<numVertex() ; ++i ) {
		assert( vertex(i).numPosition()==vertexData[i].size() );
    for( size_t j=0 ; j<vertex(i).numPosition() ; ++j )
      os << vertexData[i][j] << " ";
    os << std::endl;
  }
  os << std::endl;
  
  //Print wall data
  os << numWall() << " 1 " << wall(0).numVariable() << std::endl;
  for( size_t i=0 ; i<numWall() ; ++i ) {
		assert( wallData[i].size() );
    for( size_t j=0 ; j<wallData[i].size() ; ++j )
      os << wallData[i][j] << " ";
    os << std::endl;
  }
  os << std::endl;

  //Print cell data
  os << numCell() << " " << cell(0).numVariable() << std::endl;
  if( cell(0).numVariable() ) {
    for( size_t i=0 ; i<numCell() ; ++i ) {
			assert( cellData[i].size() );
			for( size_t j=0 ; j<cellData[i].size() ; ++j )
				os << cellData[i][j] << " ";
      os << std::endl;
    }
    os << std::endl;
  }  
	os.precision(oldPrecision);	
}

void Tissue::printVertex(std::ostream &os) {
  
  for( size_t i=0 ; i<numVertex() ; ++i )
    os << vertex(i).position(0) << " "
       << vertex(i).position(1) << std::endl;
}

void Tissue::printWall(std::ostream &os) {
  
  for( size_t i=0 ; i<numWall() ; ++i ) {
    for( size_t d=0 ; d<wall(i).vertex1()->numPosition() ; ++d )
      os << wall(i).vertex1()->position(d) << " "; 
    os << std::endl;
    for( size_t d=0 ; d<wall(i).vertex1()->numPosition() ; ++d )
      os << wall(i).vertex2()->position(d) << " "; 
    os << std::endl << std::endl << std::endl;
  }
}

void Tissue::printVertexAndCell(std::ostream &os) {
  
  size_t Nv = numVertex(); 
  if( !Nv ) {
    os << "0 0" << std::endl << "0 0" << std::endl;
    return;
  }
  //Print the vertex positions
  size_t dimension = vertex(0).numPosition();
  os << numVertex() << " " << dimension << std::endl;
  for( size_t i=0 ; i<Nv ; ++i ) {
    for( size_t d=0 ; d<dimension ; ++d )
      os << vertex(i).position(d) << " ";
    os << std::endl;
  }
  os << std::endl;
  //Print the cells, first connected vertecis and then variables
  size_t Nc = numCell();
  static std::vector<size_t> randomIndex(Nc);
  static size_t flag=1;
  if( flag ) {
    for( size_t i=0 ; i<Nc ; ++i ) randomIndex[i]=i;
    random_shuffle(randomIndex.begin(),randomIndex.end());
    flag=0;
  }
  os << Nc << " 3" << std::endl;
  for( size_t i=0 ; i<Nc ; ++i ) {
    size_t Ncv = cell(i).numVertex(); 
    os << Ncv << " ";
    for( size_t k=0 ; k<Ncv ; ++k )
      os << cell(i).vertex(k)->index() << " ";
    os << i << " " << randomIndex[i] << " " << cell(i).volume() 
			 << std::endl;
  }
}

void Tissue::
printVertexAndCell(std::vector< std::vector<double> > &cellData,
									 std::vector< std::vector<double> > &vertexData,
									 std::ostream &os) {
  
  size_t Nv = vertexData.size(); 
  if( !Nv ) {
    os << "0 0" << std::endl << "0 0" << std::endl;
    return;
  }
  //Print the vertex positions
  size_t dimension = vertexData[0].size();
  os << numVertex() << " " << dimension << std::endl;
  for( size_t i=0 ; i<Nv ; ++i ) {
    for( size_t d=0 ; d<dimension ; ++d )
      os << vertexData[i][d] << " ";
    os << std::endl;
  }
  os << std::endl;
  //Print the cells, first connected vertecis and then variables
  size_t Nc = cellData.size();
	//For normal printing
	//////////////////////////////////////////////////////////////////////
	int numPrintVar=cell(0).numVariable()+3;
	os << Nc << " " << numPrintVar << std::endl;
  for( size_t i=0 ; i<Nc ; ++i ) {
		size_t Ncv = cell(i).numVertex(); 
		os << Ncv << " ";
		for( size_t k=0 ; k<Ncv ; ++k )
			os << cell(i).vertex(k)->index() << " ";
	
		for( size_t k=0 ; k<cellData[i].size() ; ++k )
			os << cellData[i][k] << " ";
		os << i << " " << cell(i).calculateVolume(vertexData) << " " 
			 << cell(i).numWall() << std::endl;
  }
  /////////////////////////////////////////////////////////////////////
	//End, normal printing
	
	//For membrane-PIN printing (version1)
	//////////////////////////////////////////////////////////////////////
// 	os << Nc << std::endl;
// 	size_t aI=0,pI=1;
//   for( size_t i=0 ; i<Nc ; ++i ) {
//     size_t Ncv = cell(i).numVertex(); 
//     os << Ncv << " ";
//     for( size_t k=0 ; k<Ncv ; ++k )
//       os << cell(i).vertex(k)->index() << " ";

// 		double p3=0.01;
// 		double sum=p3;
// 		for( size_t n=0 ; n<Ncv ; ++n ) {
// 			if( !cell(i).isNeighbor(background()) ) { 
// 				if( cell(i).wall(n)->cell1()->index()==i )
// 					sum += cellData[ cell(i).wall(n)->cell2()->index() ][ aI ];
// 				else
// 					sum += cellData[ cell(i).wall(n)->cell1()->index() ][ aI ];
// 			}
// 		}
		
// 		os << p3*cellData[i][pI]/sum << " ";		
		
// 		for( size_t k=0 ; k<Ncv ; ++k ) {
// 			if( !cell(i).isNeighbor(background()) ) { 
// 				size_t neighIndex; 
// 				if( cell(i).wall(k)->cell1()->index()==i )
// 					neighIndex = cell(i).wall(k)->cell2()->index();				
// 				else
// 					neighIndex = cell(i).wall(k)->cell1()->index();				
// 				os << cellData[i][pI] * cellData[neighIndex][aI] / sum << " ";
// 			}
// 			else
// 				os << 0.0 << " ";
// 		}
// 		os << std::endl;
//   }	

//   //For membrane PIN1 printing (version3)
//   //////////////////////////////////////////////////////////////////////
//  	os << Nc << std::endl;
// 	size_t auxinI=1,pinI=2,auxI=3,pidI=4,xI=5;
//  	std::vector<double> parameter(8);
// 	parameter[0]=0.1;
// 	parameter[1]=1.0;
// 	parameter[2]=0.9;
// 	parameter[3]=0.1;
// 	parameter[4]=1.0;
// 	parameter[5]=1.0;
// 	parameter[6]=0.0001;
// 	parameter[7]=4;
	
// 	for( size_t i=0 ; i<Nc ; ++i ) {
// 		size_t Ncv = cell(i).numVertex(); 
// 		os << Ncv << " ";
// 		for( size_t k=0 ; k<Ncv ; ++k )
// 			os << cell(i).vertex(k)->index() << " ";
		
// 		//Transport
// 		//////////////////////////////////////////////////////////////////////
// 		size_t numWalls=cell(i).numWall();
// 		//PID factor
// 		double tmpPow = std::pow(cellData[i][pidI],parameter[7]);
// 		double Ci = tmpPow/(tmpPow+std::pow(parameter[6],parameter[7]));
		
// 		//Polarization coefficient normalization constant
// 		double sum=0.0;
// 		std::vector<double> Pij(numWalls);
// 		for( size_t n=0 ; n<numWalls ; ++n ) {
// 			if( cell(i).wall(n)->cell1() != background() &&
// 					cell(i).wall(n)->cell2() != background() ) { 
// 				size_t neighI;
// 				if( cell(i).wall(n)->cell1()->index()==i )
// 					neighI = cell(i).wall(n)->cell2()->index();
// 				else
// 					neighI = cell(i).wall(n)->cell1()->index();
// 				//double powX = std::pow(cellData[ neighI ][ xI ],parameter[5));
// 				//double Cij = powX/(std::pow(parameter[4),parameter[5))+powX);
// 				double Cij = cellData[ neighI ][ xI ];
// 				sum += Pij[n] = (1.0-parameter[2]) + 
// 					parameter[2]*(Ci*Cij+(1.0-Ci)*(1.0-Cij));
// 				//sum += Pij[n] = (1.0-parameter[2]) + 
// 				//parameter[2]*cellData[ neighI ][xI];
// 			}
// 			else 
// 				sum += Pij[n] = (1.0-parameter[2]);
// 		}
// 		//sum /= numWalls;//For adjusting for different num neigh
// 		sum += parameter[3];
		
// 		if( sum >= 0.0 )
// 			std::cout << parameter[3]*cellData[i][pinI] / sum << " ";
// 		else
// 			std::cout << "0.0 ";
		
// 		for( size_t n=0 ; n<numWalls ; ++n ) {
// 			double pol=0.0;
// 			if( sum != 0.0 )
// 				pol = cellData[i][pinI] * Pij[n] / sum;
// 			std::cout << pol << " ";
// 		}
// 		std::cout << std::endl;
// 	}
//////////////////////////////////////////////////////////////////////
//End Pij printing version3
	
// 	//Transport
// 	//////////////////////////////////////////////////////////////////////
// 	size_t numWalls=T.cell(i).numWall();
// 	//PID factor
// 	double tmpPow = std::pow(cellData[i][pidI],parameter(7));
// 	double Ci = tmpPow/(tmpPow+std::pow(parameter(29),parameter(30)));
//     //Polarization coefficient normalization constant
//     double sum=0.0;
//     std::vector<double> Pij(numWalls);
//     for( size_t n=0 ; n<numWalls ; ++n ) {
//       if( T.cell(i).wall(n)->cell1() != T.background() &&
// 					T.cell(i).wall(n)->cell2() != T.background() ) { 
// 				size_t neighI;
// 				if( T.cell(i).wall(n)->cell1()->index()==i )
// 					neighI = T.cell(i).wall(n)->cell2()->index();
// 				else
// 					neighI = T.cell(i).wall(n)->cell1()->index();
// 				double powX = std::pow(cellData[ neighI ][ xI ],parameter(28));
// 				double Cij = powX/(std::pow(parameter(27),parameter(28))+powX);
// 				sum += Pij[n] = (1.0-parameter(25)) + 
// 					parameter(25)*(Ci*Cij+(1.0-Ci)*(1.0-Ci));
//       }
//       else 
// 				sum += Pij[n] = (1.0-parameter(25));
//     }
//     //sum /= numWalls;//For adjusting for different num neigh
//     sum += parameter(26);
    
//     for( size_t n=0 ; n<numWalls ; ++n ) {
//       //if( !T.cell(i).isNeighbor(T.background()) ) { 
//       if( T.cell(i).wall(n)->cell1() != T.background() &&
// 					T.cell(i).wall(n)->cell2() != T.background() ) { 
// 				size_t neighI; 
// 				if( T.cell(i).wall(n)->cell1()->index()==i )
// 					neighI = T.cell(i).wall(n)->cell2()->index();				
// 				else
// 					neighI = T.cell(i).wall(n)->cell1()->index();				
// 				double pol=0.0;
// 				if( sum != 0.0 )
// 					pol = cellData[i][pinI] * Pij[n] / sum;
// 				double transportRate = parameter(23)*cellData[neighI][auxI]*
// 					pol*cellData[i][auxinI] /
// 					( (parameter(24)+cellData[i][auxinI])*
// 						(cellData[i][auxI]+cellData[neighI][auxI]) );
// 				cellDerivs[i][auxinI] -= transportRate + 
// 					parameter(31)*cellData[i][auxinI];
// 				cellDerivs[neighI][auxinI] += transportRate +
// 					parameter(31)*cellData[i][auxinI];
//       }
//     }

//////////////////////////////////////////////////////////////////////
//End Pij printing version2


//   for( size_t i=0 ; i<Nc ; ++i ) {
//     size_t Ncv = cell(i).numVertex(); 
//     os << Ncv << " ";
//     for( size_t k=0 ; k<Ncv ; ++k )
//       os << cell(i).vertex(k)->index() << " ";

// 		double p3=0.01;
// 		double sum=p3;
// 		for( size_t n=0 ; n<Ncv ; ++n ) {
// 			if( !cell(i).isNeighbor(background()) ) { 
// 				if( cell(i).wall(n)->cell1()->index()==i )
// 					sum += cellData[ cell(i).wall(n)->cell2()->index() ][ aI ];
// 				else
// 					sum += cellData[ cell(i).wall(n)->cell1()->index() ][ aI ];
// 			}
// 		}
		
// 		os << p3*cellData[i][pI]/sum << " ";		
		
// 		for( size_t k=0 ; k<Ncv ; ++k ) {
// 			if( !cell(i).isNeighbor(background()) ) { 
// 				size_t neighIndex; 
// 				if( cell(i).wall(k)->cell1()->index()==i )
// 					neighIndex = cell(i).wall(k)->cell2()->index();				
// 				else
// 					neighIndex = cell(i).wall(k)->cell1()->index();				
// 				os << cellData[i][pI] * cellData[neighIndex][aI] / sum << " ";
// 			}
// 			else
// 				os << 0.0 << " ";
// 		}
// 		os << std::endl;
//   }	

}

void Tissue::
printVertexAndWall(std::vector< std::vector<double> > &wallData,
									 std::vector< std::vector<double> > &vertexData,
									 std::ostream &os) {
  
  size_t Nv = vertexData.size(); 
  if( !Nv ) {
    os << "0 0" << std::endl << "0 0" << std::endl;
    return;
  }
  //Print the vertex positions
  size_t dimension = vertexData[0].size();
  os << numVertex() << " " << dimension << std::endl;
  for( size_t i=0 ; i<Nv ; ++i ) {
    for( size_t d=0 ; d<dimension ; ++d )
      os << vertexData[i][d] << " ";
    os << std::endl;
  }
  os << std::endl;
  // Print the walls, first connected vertecis and then variables
  size_t Nw = wallData.size();
	//
	// Print wall variables
	//
	int numPrintVar=wallData[0].size()+2;
	os << Nw << " " << numPrintVar << std::endl;
  for( size_t i=0 ; i<Nw ; ++i ) {
		os << "2 ";
		os << wall(i).vertex1()->index() << " " 
			 << wall(i).vertex2()->index() << " ";
		for( size_t k=0 ; k<wallData[i].size() ; ++k )
			os << wallData[i][k] << " ";
		os << i << " " << wall(i).lengthFromVertexPosition(vertexData)
			 << std::endl;
  }
}	
