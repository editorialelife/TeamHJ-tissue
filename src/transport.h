//
// Filename     : transport.h
// Description  : Classes describing transport reactions
// Author(s)    : Henrik Jonsson (henrik@thep.lu.se)
// Created      : September 2013
// Revision     : $Id:$
//
#ifndef TRANSPORT_H
#define TRANSPORT_H

#include<cmath>

#include"tissue.h"
#include"baseReaction.h"

///
/// @brief A membrane diffusion reaction
///
/// A reaction for passive diffusion of molecules localized in the membrane. The
/// transport is between neighboring membrane compartments within the same cell
/// described by:
///  
/// @f[ \frac{dP_{ij}}{dt} = - p_0 ( 2 P_{ij} - P_{ij} - P_{ij}) @f] 
///  
/// where p_0 is the diffusion rate, i is the cell, j a membrane section, anf j+/- neighboring membrane sections.
///  
/// In a model file the reaction is defined as
///
/// @verbatim
/// MembraneDiffusionSimple 1 1 1
/// p_0
/// P_{wallindex}
/// @endverbatim
///
/// where the reaction assumes that each wall keeps two variables per membrane molecule.
///
/// @note The Simple in the name reflects the fact that no geometric factors are included.
///
class MembraneDiffusionSimple : public BaseReaction {
  
 public:
  
  MembraneDiffusionSimple(std::vector<double> &paraValue, 
			  std::vector< std::vector<size_t> > 
			  &indValue );
  
  void derivs(Tissue &T,
	      DataMatrix &cellData,
	      DataMatrix &wallData,
	      DataMatrix &vertexData,
	      DataMatrix &cellDerivs,
	      DataMatrix &wallDerivs,
	      DataMatrix &vertexDerivs );
};


///
/// @brief A membrane diffusion reaction
///
/// A reaction for passive diffusion of molecules localized in the membrane. The
/// transport is between neighboring membrane compartments within the same cell
/// described by:
///  
/// @f[ \frac{dP_{ij}}{dt} = - p_0 ( 2 P_{ij} - P_{ij} - P_{ij}) @f] 
///  
/// where p_0 is the diffusion rate, i is the cell, j a membrane section, anf j+/- neighboring membrane sections.
///  
/// In a model file the reaction is defined as
///
/// @verbatim
/// MembraneDiffusionSimple 1 1 1
/// p_0
/// P_{wallindex}
/// @endverbatim
///
/// where the reaction assumes that each wall keeps two variables per membrane molecule.
///
/// @note The Simple in the name reflects the fact that no geometric factors are included.
///
class MembraneDiffusionSimple2 : public BaseReaction {
  
 public:
  
  MembraneDiffusionSimple2(std::vector<double> &paraValue, 
			  std::vector< std::vector<size_t> > 
			  &indValue );
  
  void derivs(Tissue &T,
	      DataMatrix &cellData,
	      DataMatrix &wallData,
	      DataMatrix &vertexData,
	      DataMatrix &cellDerivs,
	      DataMatrix &wallDerivs,
	      DataMatrix &vertexDerivs );
};



///
/// @brief A cell-to-cell diffusion reaction
///
/// A reaction for passive diffusion of molecules between neighboring cells.
/// Note that cell volume and other topological properties are not taken into account.
/// The diffusion is described by the equation
///  
/// @f[ \frac{dc_{i}}{dt} = - p_0 \sum_j ( c_{i} - c_{j}) @f] 
///  
/// where p_0 is the diffusion rate, $c_i$ is the cell concentration and $c_j$ is the concentration in a neighboring cell.
///  
/// In a model file the reaction is defined as
///
/// @verbatim
/// DiffusionSimple 1 1 1
/// p_0
/// c_index
/// @endverbatim
///
///
/// @note The Simple in the name reflects the fact that no geometric factors are included.
///
class DiffusionSimple : public BaseReaction {
  
 public:
  
  DiffusionSimple(std::vector<double> &paraValue, 
			  std::vector< std::vector<size_t> > 
			  &indValue );
  
  void derivs(Tissue &T,
	      DataMatrix &cellData,
	      DataMatrix &wallData,
	      DataMatrix &vertexData,
	      DataMatrix &cellDerivs,
	      DataMatrix &wallDerivs,
	      DataMatrix &vertexDerivs );
};

///
/// @brief A cell-to-cell diffusion reaction dependent on conductivity variables in the walls
///
/// A reaction for passive diffusion of molecules between neighboring cells, where also conductivities between
/// the cells are taken into account and updated individually depending on the flux (no direction). The model
/// is inspired by the model presented in Hu and Cai (2013) Phys Rev Lett 111:138701.
///
/// Note that cell volume and other topological properties are not taken into account.
/// The  is described by the equation
///  
/// @f[ \frac{dc_{i}}{dt} = - p_0 \sum_j C_{ij} ( c_{i} - c_{j}) @f] 
/// @f[ \frac{dC_{ij}}{dt} = p_1 ( \frac{Q_{ij}^{p_2}}{C_{ij}^{p_3+1}} - p_4) C_{ij}) @f]
///
/// where
///
/// @f[ Q_{ij} = C_{ij} (c_{i} - c_{j}) @f]
///  
/// p_0 is the diffusion rate, $c_i$ is the cell concentration and $c_j$ is the concentration in a neighboring cell.
/// $C_{ij}$ is the conductivity in the wall (between the two cells), $p_1$ is the update rate of the conductivity,
/// $p_2$ is the 'flux' feedback control (=2 in Hu and Cai (2013)), $p_3$ is the control parameter from Hu and Cai (2013)
/// [named gamma and phase transition at gamma=1/2]. $p_4$ is a 'degradation' parameter. 
///
/// In a model file the reaction is defined as
///
/// @verbatim
/// DiffusionConductiveSimple 5 2 1 1
/// p_0 p_1 p_2 p_3 p_4
/// c_index (cell variable)
/// C_index (wall variable)
/// @endverbatim
///
/// @note The Simple in the name reflects the fact that no geometric factors are included.
///
class DiffusionConductiveSimple : public BaseReaction {
  
 public:
  
  DiffusionConductiveSimple(std::vector<double> &paraValue, 
			    std::vector< std::vector<size_t> > 
			    &indValue );
  ///
  /// @brief Derivative function for this reaction class
  ///
  /// @see BaseReaction::derivs(Compartment &compartment,size_t species,...)
  ///
  void derivs(Tissue &T,
	      DataMatrix &cellData,
	      DataMatrix &wallData,
	      DataMatrix &vertexData,
	      DataMatrix &cellDerivs,
	      DataMatrix &wallDerivs,
	      DataMatrix &vertexDerivs );
};

///
/// @brief A cell-to-cell diffusion reaction
///
/// A reaction for passive diffusion of molecules between neighboring cells.
/// Unlike DiffusionSimple, cell volume and other topological properties are taken into account.
/// The diffusion is described by the equation
///  
/// @f[ \frac{dc_{i}}{dt} = - p_0 A^{-1} \sum_j  L_{ij} ( c_{i} - c_{j}) @f] 
///  
/// where p_0 is the diffusion rate,$L_{ij}$ is the contact length between cells, 
/// $A^{-1}$ is the cell volume, $c_i$ is the cell concentration and $c_j$ is the 
/// concentration in a neighboring cell.
///  
/// In a model file the reaction is defined as
///
/// @verbatim
/// Diffusion2d 1 1 1
/// p_0
/// c_index
/// @endverbatim
///
///
/// @note The Simple in the name reflects the fact that no geometric factors are included.
///
class Diffusion2d : public BaseReaction {
  
public:
  
  Diffusion2d(std::vector<double> &paraValue, 
              std::vector< std::vector<size_t> > 
              &indValue );
  
  void derivs(Tissue &T,
	      DataMatrix &cellData,
	      DataMatrix &wallData,
	      DataMatrix &vertexData,
	      DataMatrix &cellDerivs,
	      DataMatrix &wallDerivs,
	      DataMatrix &vertexDerivs );
};






///
/// @brief A cell to cell transport reaction
///
/// A reaction for transport molecules from cell to cell dependent on a membrane localised efflux carrier. The
/// transport is between neighboring cells
/// is described by:
///  
///  @f[ \frac{dA_i}{dt} =  p_0 \sum_{neigh} (P_{ni} A_n - P_{in} A_i) @f] 
///
///  
/// where p_0 is the tranpsort rate, i is the cell and n neighboring membrane sections.
///  
/// In a model file the reaction is defined as
///
/// @verbatim
/// ActiveTransportCellEfflux 1 2 1 1
/// p_0
/// A_{cellIndex}
/// P_{wallindex}
/// @endverbatim
///
/// where the reaction assumes that each wall keeps two variables per membrane molecule.
///
///
///
class ActiveTransportCellEfflux  : public BaseReaction {
  
 public:
  
  ActiveTransportCellEfflux(std::vector<double> &paraValue, 
			  std::vector< std::vector<size_t> > 
			  &indValue );
  
  ///
  /// @brief Derivative function for this reaction class
  ///
  /// @see BaseReaction::derivs(Compartment &compartment,size_t species,...)
  ///
  void derivs(Tissue &T,
	      DataMatrix &cellData,
	      DataMatrix &wallData,
	      DataMatrix &vertexData,
	      DataMatrix &cellDerivs,
	      DataMatrix &wallDerivs,
	      DataMatrix &vertexDerivs );
};





///
/// @brief A cell to cell transport reaction
///
/// A reaction for transport molecules from cell to cell dependent on a membrane localised efflux carrier. The
/// transport is between neighboring cells
/// is described by:
///  
///  @f[ \frac{dA_i}{dt} =  p_0 \sum_{neigh} (P_{ni} /frac{A_n}{A_n+K} - P_{in} /frac{A_i}{A_i+K}) @f] 
///
///  
/// where p_0 is the tranpsort rate, i is the cell and n neighboring membrane sections.
///  
/// In a model file the reaction is defined as
///
/// @verbatim
/// ActiveTransportCellEffluxMM 2 2 1 1
/// p_0 p_1
/// A_{cellIndex}
/// P_{wallindex}
/// @endverbatim
///
/// where the reaction assumes that each wall keeps two variables per membrane molecule.
///
///
///
 

class ActiveTransportCellEffluxMM  : public BaseReaction {
  
 public:
  
  ActiveTransportCellEffluxMM(std::vector<double> &paraValue, 
			  std::vector< std::vector<size_t> > 
			  &indValue );
  
  void derivs(Tissue &T,
	      DataMatrix &cellData,
	      DataMatrix &wallData,
	      DataMatrix &vertexData,
	      DataMatrix &cellDerivs,
	      DataMatrix &wallDerivs,
	      DataMatrix &vertexDerivs );
};



///
/// @brief A cell-wall  transport reaction
///
/// A reaction for transport molecules from cell to wall to cell dependent on a membrane localised efflux carrier and an syymetrically localised influx carrier . The
/// transport is between neighboring cells
/// is described by:
///  
/// @f[ \frac{dA_i}{dt} =  \sum_{j} ( (p_0+p_1 [AUX]_i) A_{ij} ) 
/// - \sum_{j} (p_2+ p_3 P_{ij}) A_i @f]
///  
/// @f[ \frac{dA_{ij}}{dt} = (from above) + p_4 (A_{ji}-A_{ij}) @f]
///
///  
/// where p_0 is the passive influx rate, p_1 is influx dependant on AUX in cell,
///  p_2 is passive efflux, p_3 is PIN dependent efflux, 
///  
/// In a model file the reaction is defined as
///
/// @verbatim
/// ActiveTransportWall 5 2 2 2
/// p_0-p_4
/// A_{cellIndex} Aux1_{cellIndex}
/// A_{wallindex} P_{wallindex}
/// @endverbatim
///
/// where the reaction assumes that each wall keeps two variables per membrane molecule.
///
///
///
 

class ActiveTransportWall  : public BaseReaction {
  
 public:
  
  ActiveTransportWall(std::vector<double> &paraValue, 
			  std::vector< std::vector<size_t> > 
			  &indValue );
  
  void derivs(Tissue &T,
	      DataMatrix &cellData,
	      DataMatrix &wallData,
	      DataMatrix &vertexData,
	      DataMatrix &cellDerivs,
	      DataMatrix &wallDerivs,
	      DataMatrix &vertexDerivs );
};




#endif


