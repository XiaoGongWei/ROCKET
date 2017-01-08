#pragma ident "$Id: SatArcMarker2MGEX.cpp 2513 2011-02-20 09:29:32Z yanweignss $"

/**
 * @file SatArcMarker2MGEX.cpp
 * This class keeps track of satellite arcs caused by cycle slips.
 */

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
//  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Dagoberto Salazar - gAGE ( http://www.gage.es ). 2008, 2009
//
//============================================================================
//
//  Modifications:
//
//  2013/06/30   This satellite arc marker is different from SatArcMarker, and
//               it will delete unstable periods of each beginning arc, even
//               the first arc, which means this satellite is new.
//               by shjzhang.
//
//  2014/03/10   when cycle slip happens, delete the first satellite, if you
//               want to delete satellites in unstable period. by shjzhang.
//
//
//============================================================================


#include "SatArcMarker2MGEX.hpp"


namespace gpstk
{

      // Index initially assigned to this class
   int SatArcMarker2MGEX::classIndex = 3400000;


      // Returns an index identifying this object.
   int SatArcMarker2MGEX::getIndex() const
   { return index; }



      // Returns a string identifying this object.
   std::string SatArcMarker2MGEX::getClassName() const
   { return "SatArcMarker2MGEX"; }



      /* Common constructor
       *
       * @param watchFlag        Cycle slip flag to be watched.
       * @param delUnstableSats  Whether unstable satellites will be deleted.
       * @param unstableTime     Number of seconds since last arc change
       *                         that a satellite will be considered as
       *                         unstable.
       */
   SatArcMarker2MGEX::SatArcMarker2MGEX( const TypeID& watchFlag,
                               const bool delUnstableSats,
                               const double unstableTime )
      : watchCSFlag(watchFlag), deleteUnstableSats(delUnstableSats)
   {

         // Set unstableTime value
      setUnstablePeriod( unstableTime );

      setIndex();

   }  // End of method 'SatArcMarker2MGEX::SatArcMarker2MGEX()'



      /* Method to set the number of seconds since last arc change that a
       *  satellite will be considered as unstable.
       *
       * @param unstableTime     Number of seconds since last arc change
       *                         that a satellite will be considered as
       *                         unstable.
       */
   SatArcMarker2MGEX& SatArcMarker2MGEX::setUnstablePeriod(const double unstableTime)
   {

         // Check unstableTime value
      if (unstableTime > 0.0)
      {
         unstablePeriod = unstableTime;
      }
      else
      {
         unstablePeriod = 0.0;
      }

      return (*this);

   }  // End of method 'SatArcMarker2MGEX::setUnstablePeriod()'



      /* Method to get the arc changed epoch.
       * @param sat              Interested SatID.
       */
   CommonTime SatArcMarker2MGEX::getArcChangedEpoch(const SatID& sat)
   {
      std::map<SatID, CommonTime>::iterator it = satArcChangeMap.find(sat);
      if(it!=satArcChangeMap.end())
      {
         return it->second;
      }
      else
      {
         return CommonTime(CommonTime::BEGINNING_OF_TIME);
      }

   }  // End of method 'SatArcMarker2MGEX::getArcChangedEpoch()'


      /* Returns a satTypeValueMap object, adding the new data generated
       *  when calling this object.
       *
       * @param epoch     Time of observations.
       * @param gData     Data object holding the data.
       */
   satTypeValueMap& SatArcMarker2MGEX::Process( const CommonTime& epoch,
                                           satTypeValueMap& gData )
      throw(ProcessingException)
   {

      try
      {

         double flag(0.0);

         SatIDSet satRejectedSet;

            // Loop through all the satellites
         for ( satTypeValueMap::iterator it = gData.begin();
               it != gData.end();
               ++it )
         {
					// We need to set CSFlag according to the satellite system
					// For GPS and Galileo, CSL1 is set, this is also the default
				SatID sat( it->first );
				if( sat.system == SatID::systemGPS || 
					 sat.system == SatID::systemGalileo )
				{
					watchCSFlag = TypeID::CSL1;	
				}  
				else{
					Exception e("Unsupported satellite system: " + 
							      sat.convertSatelliteSystemToString( sat.system ) );
					GPSTK_THROW(e);
				} // End of ' if( sat.system == SatID::systemGPS || ... '


            try
            {
                  // Try to extract the CS flag value
               flag = (*it).second(watchCSFlag);
            }
            catch(...)
            {               

//             cout << "SatArcMarker2MGEX" << (*it).first << endl;

                  // If flag is missing, then schedule this satellite
                  // for removal
               satRejectedSet.insert( (*it).first );

               continue;
            }

               // Check if satellite currently has entries
            std::map<SatID, double>::const_iterator itArc(
                                                satArcMap.find( (*it).first ) );
            if( itArc == satArcMap.end() )
            {
                  // If it doesn't have an entry, insert one
               satArcMap[ (*it).first ] = 0.0;
               satArcChangeMap[ (*it).first ] = CommonTime::BEGINNING_OF_TIME;
            }

               // Check if there was a cycle slip
            if ( flag > 0.0 )
            {
                  // Increment the value of "TypeID::satArc"
               satArcMap[ (*it).first ] = satArcMap[ (*it).first ] + 1.0;

                  // Update arc change epoch
               satArcChangeMap[ (*it).first ] = epoch;

                  // If we want to delete unstable satellites, we must do it
                  // also when arc changes, whether or not this SV is new
               if ( deleteUnstableSats )
               {
                  satRejectedSet.insert( (*it).first );
               }
            }


               // Check if we are inside unstable period
            bool insideUnstable(std::abs(epoch-satArcChangeMap[(*it).first]) <=
                                                               unstablePeriod );

               // Test if we want to delete unstable satellites. Only do if this
               // satellite in inside the unstable period.
               // whether of not the satellite is new 
            if ( insideUnstable &&
                 deleteUnstableSats )
            {
//             cout << "SatArcMarker2MGEX" << (*it).first << endl;
               satRejectedSet.insert( (*it).first );
            }

               // We will insert satellite arc number
            (*it).second[TypeID::satArc] = satArcMap[ (*it).first ];

         }

            // Remove satellites with missing data
         gData.removeSatID(satRejectedSet);

         return gData;

      }
      catch(Exception& u)
      {
            // Throw an exception if something unexpected happens
         ProcessingException e( getClassName() + ":"
                                + StringUtils::asString( getIndex() ) + ":"
                                + u.what() );

         GPSTK_THROW(e);

      }

   }  // End of method 'SatArcMarker2MGEX::Process()'



      /* Returns a gnnsSatTypeValue object, adding the new data generated
       *  when calling this object.
       *
       * @param gData    Data object holding the data.
       */
   gnssSatTypeValue& SatArcMarker2MGEX::Process(gnssSatTypeValue& gData)
      throw(ProcessingException)
   {

      try
      {

         Process(gData.header.epoch, gData.body);

         return gData;

      }
      catch(Exception& u)
      {
            // Throw an exception if something unexpected happens
         ProcessingException e( getClassName() + ":"
                                + StringUtils::asString( getIndex() ) + ":"
                                + u.what() );

         GPSTK_THROW(e);

      }

   }  // End of method 'SatArcMarker2MGEX::Process()'



      /* Returns a gnnsRinex object, adding the new data generated when
       *  calling this object.
       *
       * @param gData    Data object holding the data.
       */
   gnssRinex& SatArcMarker2MGEX::Process(gnssRinex& gData)
      throw(ProcessingException)
   {

      try
      {

         Process(gData.header.epoch, gData.body);

         return gData;

      }
      catch(Exception& u)
      {
            // Throw an exception if something unexpected happens
         ProcessingException e( getClassName() + ":"
                                + StringUtils::asString( getIndex() ) + ":"
                                + u.what() );

         GPSTK_THROW(e);

      }

   }  // End of method 'SatArcMarker2MGEX::Process()'



}  // End of namespace gpstk
