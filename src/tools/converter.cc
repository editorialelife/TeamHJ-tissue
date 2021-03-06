//
// Filename     : converter.cc
// Description  : Reads and prints states in different formats
// Author(s)    : Henrik Jonsson (henrik@thep.lu.se)
// Created      : May 2012
// Revision     : $Id:$
//
#include <fstream>
#include <boost/concept_check.hpp>

#include "../baseSolver.h"
#include "../cell.h"
#include "../myConfig.h"
#include "../mySignal.h"
#include "../myTimes.h"
#include "../tissue.h"
#include "../vertex.h"
#include "../wall.h"
#include "../pvd_file.h"
#include "../ply_reader.h"
#include "../VTUostream.h"

int main ( int argc,char *argv[] )
{
    //Command line handling
    myConfig::registerOption ( "input_format", 1 );
    myConfig::registerOption ( "output_format", 1 );
    myConfig::registerOption ( "help", 0 );
    myConfig::registerOption ( "verbose", 1 );

    myConfig::initConfig ( argc, argv );

    if ( myConfig::getBooleanValue ( "help" ) )
    {
        std::cerr << std::endl
                  << "Usage: " << argv[0] << " initFile > outputFile" << std::endl << std::endl;
        std::cerr << "Additional flags are:" << std::endl << std::endl;
        std::cerr << "-input_format format - Sets format in input file." << std::endl
                  << "Available input formats are:" << std::endl
                  << "tissue (default), " << std::endl
                  << "organism (organism file assuming spheres and only position+radii), " << std::endl
                  << "voronoi (voronoi format from qhull output), " << std::endl
                  << "MGXTriMesh (MGX exported mesh in mesh format, before making cells), " << std::endl
                  << "MGXTriVtu (MGX exported mesh in vtu format, before making cells), " << std::endl
                  << "MGXCellVtu (MGX exported mesh in vtu format, after making cells [TO COME]), " << std::endl
                  << "merryProj (Montpellier (openAlea) format [now obselete?]), " << std::endl
                  << "ply (standard geometry format, adopted? as standard for exchange between plant modellers)." << std::endl
                  << std::endl;
        std::cerr << "-output_format format - Sets format for output of"
                  << " state in specified file format. " << std::endl
                  << "Available output formats are:" << std::endl
                  << "tissue (default), " << std::endl
                  << "triTissue (tissue with central triangulation), " << std::endl
                  << "fem (Pawel's FEM simulation format), " << std::endl
                  << "organism (organism init file including neighborhood), " << std::endl
                  << "vtu1 (vtk format with single wall compartment variables), " << std::endl
                  << "vtu2 (vtk format with two wall compartment variables)." << std::endl
                  << "ply (ply format for exchange between plant modellers using vertex-based geometries)." << std::endl
                  << std::endl;
        std::cerr << "-verbose flag - Set flag for verbose (flag=1, default) more verbose (2) or "
                  << "silent (0) output mode to stderr." << std::endl << std::endl;
        std::cerr << "-help - Shows this message." << std::endl << std::endl;
        exit ( EXIT_FAILURE );
    }
    else if ( myConfig::argc() != 2 )
    {
        std::cerr << "Type '" << argv[0] << " -help' for usage." << std::endl;
        exit ( EXIT_FAILURE );
    }
    int verboseFlag=1;
    std::string verboseString;
    verboseString = myConfig::getValue ( "verbose", 0 );
    if ( !verboseString.empty() )
    {
        verboseFlag = atoi ( verboseString.c_str() );
        if ( verboseFlag != 0 && verboseFlag !=1 && verboseFlag !=2 )
        {
            std::cerr << "Flag given to -verbose not recognized [" << verboseFlag << "] (0, 1, 2 allowed)."
                      << " Setting it to zero (silent)." << std::endl;
            verboseFlag=0;
        }
    }

    // Create the tissue and read init file
    Tissue T;
    std::string initFile = myConfig::argv ( 1 );
    std::string inputFormat;
    inputFormat = myConfig::getValue ( "input_format",0 );
    if ( inputFormat.empty() || inputFormat.compare ( "tissue" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Reading init from file " << initFile << " assuming tissue format." << std::endl;
        }
        T.readInit ( initFile.c_str(),verboseFlag );
    }
    else if ( inputFormat.compare ( "organism" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Reading init from file " << initFile << " assuming organism (sphere) format."
                      << std::endl;
        }
        T.readSphereInit ( initFile.c_str(),verboseFlag );
    }
    else if ( inputFormat.compare ( "voronoi" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Reading init from file " << initFile << " assuming voronoi (qhull) format."
                      << std::endl;
        }
        T.readVoronoiInit ( initFile.c_str(),verboseFlag );
    }
    else if ( inputFormat.compare ( "MGXTriMesh" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Reading init from file " << initFile << " assuming MGXTriMesh format."
                      << std::endl;
        }
        T.readInitMGXTriMesh ( initFile.c_str(),verboseFlag );
    }
    else if ( inputFormat.compare ( "MGXTriVtu" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Reading init from file " << initFile << " assuming MGXVtuTri format."
                      << std::endl;
        }
        T.readInitMGXTriVtu ( initFile.c_str(),verboseFlag );
    }
    else if ( inputFormat.compare ( "MGXCellVtu" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Reading init from file " << initFile << " assuming MGXCellVtu format."
                      << std::endl;
        }
        std::cerr << inputFormat << " not yet implemented for reading! Contact HJ." << std::endl;
        exit ( EXIT_FAILURE );
        //T.readMGXCellVtuInit(initFile.c_str(),verboseFlag);
    }
    else if ( inputFormat.compare ( "merryProj" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Reading init from file " << initFile << " assuming merryProj format."
                      << std::endl;
        }
        T.readInitMerryProj ( initFile.c_str(),verboseFlag );
    }
    else if ( inputFormat.compare ( "ply" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Reading init from file " << initFile << " assuming ply format."
                      << std::endl;
        }
        PLY_file pf ( initFile.c_str() );
        PLY_reader P;
        P.read ( pf,T );
        //     std::cerr << "ply read\n ";
        std::cerr << "ncell = " << T.numCell() << "; nedge = " << T.numWall() << "; nvertex = " << T.numVertex() << "\n";
        //Sort all cellWalls and cellVertices to comply with area calculations and plotting
        //T.sortCellWallAndCellVertex();
        //T.checkConnectivity(verboseFlag);
        //T.readInitPly(initFile.c_str(),verboseFlag); ...yet to be defined...
    }
    else
    {
        std::cerr << "Input format " << inputFormat << " not recognized. Use '-help' for allowed formats."
                  << std::endl;
        exit ( EXIT_FAILURE );
    }

    // Print in specified output format
    std::string outputFormat;
    outputFormat = myConfig::getValue ( "output_format",0 );
    if ( outputFormat.empty() || outputFormat.compare ( "tissue" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Printing output using tissue format." << std::endl;
        }
        T.printInit ( std::cout );
    }
    else if ( outputFormat.compare ( "triTissue" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Printing output using triangulated tissue format."
                      << std::endl;
        }
        T.printInitTri ( std::cout );
    }
    else if ( outputFormat.compare ( "fem" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Printing output using fem format." << std::endl;
        }
        T.printInitFem ( std::cout );
    }
    else if ( outputFormat.compare ( "organism" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Printing output using organism format (init + neigh)." << std::endl;
        }
        T.printInitOrganism ( std::cout );
    }
    else if ( outputFormat.compare ( "ply" ) ==0 )
    {
        if ( verboseFlag )
        {
            std::cerr << "Printing output using ply format." << std::endl;
        }
        PLY_ostream ply_os ( std::cout );
        ply_os.bare_geometry_output() = false;
        ply_os.center_triangulation_output() = false;
        ply_os << T;
        //T.printInitPly(std::cout);
    }
    else if ( outputFormat.compare ( "pvd" ) ==0 )
    {
        std::cout << "<?xml version=\"1.0\"?> " << std::endl;
        std::cout << "<VTKFile type=\"Collection\" version=\"0.1\" > " << std::endl;
        std::cout << "<Collection> " << std::endl;

        // Data file name
        std::cout << "<DataSet timestep=\"0\" part=\"0\" file=\"cells.vtu\"/>" << std::endl;
        std::cout << "<DataSet timestep=\"0\" part=\"1\" file=\"walls.vtu\"/>" << std::endl;

        // Close headers
        std::cout << "</Collection> " << std::endl;
        std::cout << "</VTKFile> " << std::endl;

        std::ofstream vtu_f ( "cells.vtu" );
        VTUostream vtuos ( vtu_f );
        vtuos.write_cells ( T );
        vtuos.close();
        vtu_f.close();
        vtu_f.open ( "walls.vtu" );
        vtuos.open ( vtu_f );
        vtuos.write_walls ( T );
        vtuos.close();
        vtu_f.close();
    }
    else
    {
        std::cerr << "Warning: main() - Format " << outputFormat << " not recognized. "
                  << "No outputwritten." << std::endl;
    }
    if ( verboseFlag )
    {
        std::cerr << "All done!" << std::endl;
    }
}
