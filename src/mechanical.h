/**
 * Filename     : mechanical.h
 * Description  : Classes describing mechanical updates
 * Author(s)    : Henrik Jonsson (henrik@thep.lu.se)
 * Created      : April 2006
 * Revision     : $Id:$
 */
#ifndef MECHANICAL_H
#define MECHANICAL_H

#include"tissue.h"
#include"baseReaction.h"
#include<cmath>

//!Updates vertices from an asymmetric wall spring potential
class VertexFromWallSpringAsymmetric : public BaseReaction {
  
 public:
  
  VertexFromWallSpringAsymmetric(std::vector<double> &paraValue, 
				 std::vector< std::vector<size_t> > 
				 &indValue );
  
  void derivs(Tissue &T,
	      std::vector< std::vector<double> > &cellData,
	      std::vector< std::vector<double> > &wallData,
	      std::vector< std::vector<double> > &vertexData,
	      std::vector< std::vector<double> > &cellDerivs,
	      std::vector< std::vector<double> > &wallDerivs,
	      std::vector< std::vector<double> > &vertexDerivs );
};

///
/// @brief Updates vertices from an spatially asymmetric wall spring potential
/// given by microtubule directions
///
class VertexFromWallSpringMT : public BaseReaction {
  
public:
  
  VertexFromWallSpringMT(std::vector<double> &paraValue, 
												 std::vector< std::vector<size_t> > 
												 &indValue );
  
  void derivs(Tissue &T,
							std::vector< std::vector<double> > &cellData,
							std::vector< std::vector<double> > &wallData,
							std::vector< std::vector<double> > &vertexData,
							std::vector< std::vector<double> > &cellDerivs,
							std::vector< std::vector<double> > &wallDerivs,
							std::vector< std::vector<double> > &vertexDerivs );
	void initiate(Tissue &T,
								std::vector< std::vector<double> > &cellData,
								std::vector< std::vector<double> > &wallData,
								std::vector< std::vector<double> > &vertexData );
	//void update(Tissue &T,double step);
};

///
/// @brief Updates vertices from an spatially asymmetric wall spring potential
/// given by microtubule directions and also updates the spring constants
/// slowly.
///
class VertexFromWallSpringMTHistory : public BaseReaction {
  
public:
  
  VertexFromWallSpringMTHistory(std::vector<double> &paraValue, 
																std::vector< std::vector<size_t> > 
																&indValue );
  
  void derivs(Tissue &T,
							std::vector< std::vector<double> > &cellData,
							std::vector< std::vector<double> > &wallData,
							std::vector< std::vector<double> > &vertexData,
							std::vector< std::vector<double> > &cellDerivs,
							std::vector< std::vector<double> > &wallDerivs,
							std::vector< std::vector<double> > &vertexDerivs );
	void initiate(Tissue &T,
								std::vector< std::vector<double> > &cellData,
								std::vector< std::vector<double> > &wallData,
								std::vector< std::vector<double> > &vertexData);
	//void update(Tissue &T,double step);
};

//!Updates vertices from an asymmetric epidermal wall spring potential
class VertexFromEpidermalWallSpringAsymmetric : public BaseReaction {
  
 public:
  
  VertexFromEpidermalWallSpringAsymmetric(std::vector<double> &paraValue, 
					  std::vector< std::vector<size_t> > 
					  &indValue );
  
  void derivs(Tissue &T,
	      std::vector< std::vector<double> > &cellData,
	      std::vector< std::vector<double> > &wallData,
	      std::vector< std::vector<double> > &vertexData,
	      std::vector< std::vector<double> > &cellDerivs,
	      std::vector< std::vector<double> > &wallDerivs,
	      std::vector< std::vector<double> > &vertexDerivs );
};

//!Updates vertices from an asymmetric epidermal wall spring potential
class VertexFromEpidermalCellWallSpringAsymmetric : public BaseReaction {
  
 public:
  
  VertexFromEpidermalCellWallSpringAsymmetric(std::vector<double> &paraValue, 
																							std::vector< std::vector<size_t> > 
																							&indValue );
  
  void derivs(Tissue &T,
							std::vector< std::vector<double> > &cellData,
							std::vector< std::vector<double> > &wallData,
							std::vector< std::vector<double> > &vertexData,
							std::vector< std::vector<double> > &cellDerivs,
							std::vector< std::vector<double> > &wallDerivs,
							std::vector< std::vector<double> > &vertexDerivs );
};

//!Updates vertices from a cell pressure potential
class VertexFromCellPressure : public BaseReaction {
  
 public:
  
  VertexFromCellPressure(std::vector<double> &paraValue, 
			 std::vector< std::vector<size_t> > &indValue );
  
  void derivs(Tissue &T,
	      std::vector< std::vector<double> > &cellData,
	      std::vector< std::vector<double> > &wallData,
	      std::vector< std::vector<double> > &vertexData,
	      std::vector< std::vector<double> > &cellDerivs,
	      std::vector< std::vector<double> > &wallDerivs,
	      std::vector< std::vector<double> > &vertexDerivs );
};

//!Updates vertices from a cell pressure potential
class VertexFromCellPressureVolumeNormalized : public BaseReaction {
  
 public:
  
  VertexFromCellPressureVolumeNormalized(std::vector<double> &paraValue, 
																				 std::vector< std::vector<size_t> > &indValue );
  
  void derivs(Tissue &T,
							std::vector< std::vector<double> > &cellData,
							std::vector< std::vector<double> > &wallData,
							std::vector< std::vector<double> > &vertexData,
							std::vector< std::vector<double> > &cellDerivs,
							std::vector< std::vector<double> > &wallDerivs,
							std::vector< std::vector<double> > &vertexDerivs );
};

//!Updates vertices from a cell pressure potential
class VertexFromCellPressureThresholdFromMaxPos : public BaseReaction {
  
 public:
  
  VertexFromCellPressureThresholdFromMaxPos(std::vector<double> &paraValue, 
																						std::vector< std::vector<size_t> > &indValue );
  
  void derivs(Tissue &T,
							std::vector< std::vector<double> > &cellData,
							std::vector< std::vector<double> > &wallData,
							std::vector< std::vector<double> > &vertexData,
							std::vector< std::vector<double> > &cellDerivs,
							std::vector< std::vector<double> > &wallDerivs,
							std::vector< std::vector<double> > &vertexDerivs );
};

//!Updates vertices from a cell 'pressure' potential for internal cells
class VertexFromCellInternalPressure : public BaseReaction {
  
 public:
  
  VertexFromCellInternalPressure(std::vector<double> &paraValue, 
				 std::vector< std::vector<size_t> > &indValue );
  
  void derivs(Tissue &T,
	      std::vector< std::vector<double> > &cellData,
	      std::vector< std::vector<double> > &wallData,
	      std::vector< std::vector<double> > &vertexData,
	      std::vector< std::vector<double> > &cellDerivs,
	      std::vector< std::vector<double> > &wallDerivs,
	      std::vector< std::vector<double> > &vertexDerivs );
};

//!Updates vertices from cells via a power diagram potential
class VertexFromCellPowerdiagram : public BaseReaction {
  
 public:
  
  VertexFromCellPowerdiagram(std::vector<double> &paraValue, 
			     std::vector< std::vector<size_t> > 
			     &indValue );
  
  void derivs(Tissue &T,
	      std::vector< std::vector<double> > &cellData,
	      std::vector< std::vector<double> > &wallData,
	      std::vector< std::vector<double> > &vertexData,
	      std::vector< std::vector<double> > &cellDerivs,
	      std::vector< std::vector<double> > &wallDerivs,
	      std::vector< std::vector<double> > &vertexDerivs );
};

//!Sets positional derivatives to zero for vertices in specified region  
class VertexNoUpdateFromPosition : public BaseReaction {
  
 public:
  
  VertexNoUpdateFromPosition(std::vector<double> &paraValue, 
			     std::vector< std::vector<size_t> > 
			     &indValue );
  
  void derivs(Tissue &T,
	      std::vector< std::vector<double> > &cellData,
	      std::vector< std::vector<double> > &wallData,
	      std::vector< std::vector<double> > &vertexData,
	      std::vector< std::vector<double> > &cellDerivs,
	      std::vector< std::vector<double> > &wallDerivs,
	      std::vector< std::vector<double> > &vertexDerivs );
};

//!Applies a force towards or from origo on vertices specified by indices
class VertexForceOrigoFromIndex : public BaseReaction {
  
 public:
  
  VertexForceOrigoFromIndex(std::vector<double> &paraValue, 
			  std::vector< std::vector<size_t> > 
			  &indValue );
  
  void derivs(Tissue &T,
	      std::vector< std::vector<double> > &cellData,
	      std::vector< std::vector<double> > &wallData,
	      std::vector< std::vector<double> > &vertexData,
	      std::vector< std::vector<double> > &cellDerivs,
	      std::vector< std::vector<double> > &wallDerivs,
	      std::vector< std::vector<double> > &vertexDerivs );
};

//!Applies a force towards or from origo on vertices of cells
class CellForceOrigoFromIndex : public BaseReaction {
  
 public:
  
  CellForceOrigoFromIndex(std::vector<double> &paraValue, 
			  std::vector< std::vector<size_t> > 
			  &indValue );
  
  void derivs(Tissue &T,
	      std::vector< std::vector<double> > &cellData,
	      std::vector< std::vector<double> > &wallData,
	      std::vector< std::vector<double> > &vertexData,
	      std::vector< std::vector<double> > &cellDerivs,
	      std::vector< std::vector<double> > &wallDerivs,
	      std::vector< std::vector<double> > &vertexDerivs );
};

//!Applies a force towards or from a Cylinder surface
class CylinderForce : public BaseReaction {
  
 public:
  
  CylinderForce(std::vector<double> &paraValue, 
								std::vector< std::vector<size_t> > 
								&indValue );
  
  void derivs(Tissue &T,
							std::vector< std::vector<double> > &cellData,
							std::vector< std::vector<double> > &wallData,
							std::vector< std::vector<double> > &vertexData,
							std::vector< std::vector<double> > &cellDerivs,
							std::vector< std::vector<double> > &wallDerivs,
							std::vector< std::vector<double> > &vertexDerivs );
};

//!Applies a force towards or from a SphereCylinder surface
class SphereCylinderForce : public BaseReaction {
  
 public:
  
  SphereCylinderForce(std::vector<double> &paraValue, 
		      std::vector< std::vector<size_t> > 
		      &indValue );
  
  void derivs(Tissue &T,
	      std::vector< std::vector<double> > &cellData,
	      std::vector< std::vector<double> > &wallData,
	      std::vector< std::vector<double> > &vertexData,
	      std::vector< std::vector<double> > &cellDerivs,
	      std::vector< std::vector<double> > &wallDerivs,
	      std::vector< std::vector<double> > &vertexDerivs );
};

//!Applies a force towards a spherecylinder surface with defined radius
class SphereCylinderForceFromRadius : public BaseReaction {
  
 public:
  
  SphereCylinderForceFromRadius(std::vector<double> &paraValue, 
				std::vector< std::vector<size_t> > 
				&indValue );
  
  void derivs(Tissue &T,
	      std::vector< std::vector<double> > &cellData,
	      std::vector< std::vector<double> > &wallData,
	      std::vector< std::vector<double> > &vertexData,
	      std::vector< std::vector<double> > &cellDerivs,
	      std::vector< std::vector<double> > &wallDerivs,
	      std::vector< std::vector<double> > &vertexDerivs );
};

//!Applies a force perpendicular to a defined wall of infinite size
class InfiniteWallForce : public BaseReaction {
  
 public:
  
  InfiniteWallForce(std::vector<double> &paraValue, 
		    std::vector< std::vector<size_t> > 
		    &indValue );
  
  void derivs(Tissue &T,
	      std::vector< std::vector<double> > &cellData,
	      std::vector< std::vector<double> > &wallData,
	      std::vector< std::vector<double> > &vertexData,
	      std::vector< std::vector<double> > &cellDerivs,
	      std::vector< std::vector<double> > &wallDerivs,
	      std::vector< std::vector<double> > &vertexDerivs );
};

//!Applies a force on epidermal vertices
class EpidermalVertexForce : public BaseReaction {
  
 public:
  
  EpidermalVertexForce(std::vector<double> &paraValue, 
		    std::vector< std::vector<size_t> > 
		    &indValue );
  
  void derivs(Tissue &T,
	      std::vector< std::vector<double> > &cellData,
	      std::vector< std::vector<double> > &wallData,
	      std::vector< std::vector<double> > &vertexData,
	      std::vector< std::vector<double> > &cellDerivs,
	      std::vector< std::vector<double> > &wallDerivs,
	      std::vector< std::vector<double> > &vertexDerivs );
};

class VertexFromPressureExperimental : public BaseReaction
{  
 public:
	VertexFromPressureExperimental(std::vector<double> &paraValue, 
							 std::vector< std::vector<size_t> > &indValue);
	
	void derivs(Tissue &T,
			  std::vector< std::vector<double> > &cellData,
			  std::vector< std::vector<double> > &wallData,
			  std::vector< std::vector<double> > &vertexData,
			  std::vector< std::vector<double> > &cellDerivs,
			  std::vector< std::vector<double> > &wallDerivs,
			  std::vector< std::vector<double> > &vertexDerivs);
	double polygonArea(std::vector< std::pair<double, double> > vertices);
};

class VertexFromWallSpringExperimental : public BaseReaction
{
 public:
	VertexFromWallSpringExperimental(std::vector<double> &paraValue,
							   std::vector< std::vector<size_t> > &indValue);

	void derivs(Tissue &T,
			  std::vector< std::vector<double> > &cellData,
			  std::vector< std::vector<double> > &wallData,
			  std::vector< std::vector<double> > &vertexData,
			  std::vector< std::vector<double> > &cellDerivs,
			  std::vector< std::vector<double> > &wallDerivs,
			  std::vector< std::vector<double> > &vertexDerivs);
};

class CellVolumeExperimental : public BaseReaction
{
 public:
	CellVolumeExperimental(std::vector<double> &paraValue,
					   std::vector< std::vector<size_t> > &indValue);
	
	void derivs(Tissue &T,
			  std::vector< std::vector<double> > &cellData,
			  std::vector< std::vector<double> > &wallData,
			  std::vector< std::vector<double> > &vertexData,
			  std::vector< std::vector<double> > &cellDerivs,
			  std::vector< std::vector<double> > &wallDerivs,
			  std::vector< std::vector<double> > &vertexDerivs);
};


class EpidermalRadialForce : public BaseReaction
{
 public:
	EpidermalRadialForce(std::vector<double> &paraValue,
					 std::vector< std::vector<size_t> > &indValue);
	
	void derivs(Tissue &T,
			  std::vector< std::vector<double> > &cellData,
			  std::vector< std::vector<double> > &wallData,
			  std::vector< std::vector<double> > &vertexData,
			  std::vector< std::vector<double> > &cellDerivs,
			  std::vector< std::vector<double> > &wallDerivs,
			  std::vector< std::vector<double> > &vertexDerivs);
};

class PerpendicularWallPressure : public BaseReaction
{
 public:
	PerpendicularWallPressure(std::vector<double> &paraValue,
						 std::vector< std::vector<size_t> > &indValue);
	
	void derivs(Tissue &T,
			  std::vector< std::vector<double> > &cellData,
			  std::vector< std::vector<double> > &wallData,
			  std::vector< std::vector<double> > &vertexData,
			  std::vector< std::vector<double> > &cellDerivs,
			  std::vector< std::vector<double> > &wallDerivs,
			  std::vector< std::vector<double> > &vertexDerivs);
};

class ContinousMTDirection : public BaseReaction
{
 public:
	ContinousMTDirection(std::vector<double> &paraValue,
			     std::vector< std::vector<size_t> > &indValue);
	
	void derivs(Tissue &T,
		    std::vector< std::vector<double> > &cellData,
		    std::vector< std::vector<double> > &wallData,
		    std::vector< std::vector<double> > &vertexData,
		    std::vector< std::vector<double> > &cellDerivs,
		    std::vector< std::vector<double> > &wallDerivs,
		    std::vector< std::vector<double> > &vertexDerivs);
};

// Do not use this reaction. Restricted area (unless you are a developer).
class DebugReaction : public BaseReaction
{
 public:
	DebugReaction(std::vector<double> &paraValue,
		      std::vector< std::vector<size_t> > &indValue);

	void derivs(Tissue &T,
		    std::vector< std::vector<double> > &cellData,
		    std::vector< std::vector<double> > &wallData,
		    std::vector< std::vector<double> > &vertexData,
		    std::vector< std::vector<double> > &cellDerivs,
		    std::vector< std::vector<double> > &wallDerivs,
		    std::vector< std::vector<double> > &vertexDerivs);
};

#endif
