#pragma ident "$Id$"
//============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 2.1 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
//  
//  Copyright 2004, The University of Texas at Austin
//
//============================================================================

/**
 * @file VisSupport.hpp - Support functions shared by compStaVis and compSatVis.
 */

#ifndef GPSTK_VISSUPPORT_HPP
#define GPSTK_VISSUPPORT_HPP


#include <iostream>
#include <string>
#include <list>
#include <vector>


#include "StringUtils.hpp"
#include "CommandOption.hpp"
#include "FileFilterFrame.hpp"
#include "Position.hpp"
#include "Matrix.hpp"
#include "geometry.hpp"

// rinex
#include "RinexNavStream.hpp"
#include "RinexNavData.hpp"
#include "RinexNavHeader.hpp"

// fic
#include "FICStream.hpp"
#include "FICHeader.hpp"
#include "FICData.hpp"
#include "FICFilterOperators.hpp"

#include "AlmOrbit.hpp"
#include "GPSAlmanacStore.hpp"
#include "YumaAlmanacStore.hpp"
#include "SEMAlmanacStore.hpp"
#include "GPSEphemerisStore.hpp"

// monitor station coordinates
#include "MSCStore.hpp"
#include "Position.hpp"

// SP3
#include "SP3Stream.hpp"
#include "SP3EphemerisStore.hpp"

#include "EngEphemeris.hpp"

namespace gpstk
{
   typedef std::map<std::string,gpstk::Position> StaPosList; 

   namespace VisSupport
   {
      StaPosList  getStationCoordinates( 
                       const gpstk::CommandOptionWithAnyArg& mscFileName,
                       const gpstk::CommonTime&                 dt, 
                       const gpstk::CommandOptionWithAnyArg& includeStation,
                       const gpstk::CommandOptionWithAnyArg& excludeStation);
                       
      void readFICNavData( 
                       const gpstk::CommandOptionWithAnyArg& navFileNameOpt,
                             gpstk::GPSAlmanacStore&         BCAlmList,  
                             gpstk::GPSEphemerisStore&       BCEphList);
                             
      void readRINEXNavData(
                       const gpstk::CommandOptionWithAnyArg& navFileNameOpt,
                             gpstk::GPSEphemerisStore&       BCEphList);
                           
      void readYumaData(
                       const gpstk::CommandOptionWithAnyArg& navFileNameOpt,
                             gpstk::YumaAlmanacStore&        yumaAlmStore );
                           
      void readSEMData(
                       const gpstk::CommandOptionWithAnyArg& navFileNameOpt,
                             gpstk::SEMAlmanacStore&         semAlmStore );
                             
      void readPEData(
                       const gpstk::CommandOptionWithAnyArg& navFileNameOpt,
                             gpstk::SP3EphemerisStore&       SP3EphList );

      bool checkIOD( const gpstk::EngEphemeris ee );
      
      class M4 : public Matrix<double> // trick to declare a 4x4 double array of Matrix
      {
      public:
         M4() : Matrix<double>(4,4) {};
      }; 

   // Note:  observerVectors is a vector of XYZ->UENT coordinate transform
   //        4x4 matrices.  See calculateObserverVectors below.
   // Note:  The invert flag is used in the case where we are considering
   //        the SV to be the OBSERVER.  In this case, the elevation angle
   //        will need to be calculated in the opposite direction from normal.
      double computeDOP(const Position& observer, 
                const std::vector<Position>& sources,
                const VisSupport::M4& observerVectors,
                const double elevLimit,
                const bool invert);
                
      M4 calculateObserverVectors(const Position& observer);

   } // namespace VisSupport

   
} // namespace gpstk

#endif
