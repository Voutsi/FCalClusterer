#ifndef LumiCalClusterer_getCalHits_h
#define LumiCalClusterer_getCalHits_h 1
// Local
#include "LumiCalClusterer.h"
//LCIO
#include <Exceptions.h>
#include <EVENT/LCCollection.h>
#include <EVENT/LCEvent.h>
#include <IMPL/SimCalorimeterHitImpl.h>
#include <IMPL/CalorimeterHitImpl.h>
#include <UTIL/CellIDDecoder.h>
// Stdlib
#include <map>
#include <vector>
#include <iostream>
#include <cmath>
#include <iomanip>

/* --------------------------------------------------------------------------
   Loop over al hits in the LCCollection and write the hits into vectors
   of CalorimeterHitImpl. Hits are split in two vectors, one for each arm
   of LumiCal.
   -------------------------------------------------------------------------- */
int LumiCalClustererClass::getCalHits(	EVENT::LCEvent * evt,
					MapIntMapIntVCalHit & calHits) {

  EVENT::LCCollection * col = NULL;
  try {
    col = evt->getCollection(_lumiName.c_str());
  } // try
  // if an exception has been thrown (no *col for this event) then do....
  catch( EVENT::DataNotAvailableException &e){
#if _GENERAL_CLUSTERER_DEBUG == 1
    streamlog_out( ERROR ) << "Event has a SimCalorimeterHitImpl exception"<< std::endl;
#endif
    return 0;
  }

#if _GENERAL_CLUSTERER_DEBUG == 1
  streamlog_out( MESSAGE4 ) << std::endl  << "Getting hit information .... event: "<< evt->getEventNumber() << std::endl;
#endif

    if (not _mydecoder) {
      _mydecoder = std::unique_ptr< CellIDDecoder<SimCalorimeterHit> >( new CellIDDecoder<SimCalorimeterHit>(col) );
    }
    const int nHitsCol = col->getNumberOfElements();
    if ( nHitsCol < _clusterMinNumHits ) return 0;

    for (int i=0; i<nHitsCol; ++i) {
      
      int arm(0), layer(0);
      int rCell(0), phiCell(0);

      // get the hit from the LCCollection with index i
      IMPL::SimCalorimeterHitImpl * calHitIn = static_cast<IMPL::SimCalorimeterHitImpl*> (col->getElementAt(i));

      const double engyHit = (double)calHitIn -> getEnergy();

      if(engyHit < _hitMinEnergy)	continue;

      // ???????? DECIDE/FIX - the global coordinates are going to change in new Mokka
      // versions, so the z must be extracted from the cellId instead ... ????????
      /// APS: Can be taken from the position of the calohit, when it is stored

      // get parameters from the input IMPL::CalorimeterHitImpl
     

      //using Mokka simulated files
      if( not _useDD4hep ) {
	arm     = (*_mydecoder)( calHitIn )["S-1"]; // from 0
	rCell   = (*_mydecoder)( calHitIn )["I"]; // from 0
	phiCell = (*_mydecoder)( calHitIn )["J"]; // from 0
	layer   = (*_mydecoder)( calHitIn )["K"]; // counts from 1
	// detector layer  - count layers from zero and not from one
	layer -= 1 ;
	// determine the side (arm) of the hit -> (+,-)1
	arm = (( arm == 0 ) ? -1 : 1);

	if( arm < 0 ) {
	  //for rotation around the X-axis, so that the phiCell increases counter-clockwise for z<0
	  phiCell = _cellPhiMax - phiCell;
	  phiCell += _cellPhiMax/2;
	  if(phiCell >= _cellPhiMax) phiCell -= _cellPhiMax;
	}

      } else {
	arm =(*_mydecoder)( calHitIn )["barrel"]; // from 1 and 2
	if( arm == 2 ) arm = -1;
	phiCell = (*_mydecoder)( calHitIn )["phi"]; // goes from -phiMax/2 to +phiMax/2

	if( arm < 0 ) {
	  //for rotation around the X-axis, so that the phiCell increases counter-clockwise for z<0
	  if( phiCell > 0) phiCell =  int(_cellPhiMax/2) - phiCell;
	  if( phiCell < 0) phiCell = -int(_cellPhiMax/2) - phiCell;
	  //LumiCall is (or not, if we fix it) rotated by pi around Z for negative side
	  phiCell += int(_gmc._backwardRotationPhi/(2.0*M_PI)*_cellPhiMax+0.5);
	}

	if(phiCell >= _cellPhiMax) phiCell -= _cellPhiMax;
	if(phiCell < 0 ) phiCell += _cellPhiMax; // need to put into positive range only
	rCell = (*_mydecoder)( calHitIn )["r"];
	layer = (*_mydecoder)( calHitIn )["layer"]; // counts from 0
      }

      //Calculate internal cellID
      int cellId = GlobalMethodsClass::CellIdZPR(layer, phiCell, rCell, arm);

      // skip this hit if the following conditions are met
      if(layer >= _maxLayerToAnalyse || layer < 0 )	continue;

      
      /*(BP) it is not safe - in case non-zero crossing angle
            - phi sectors numbering order changes on -ve side
            - in some models there is layers relative phi offset  
            - also in local system zHit is +ve always
     
      const double rHit = (rCell+0.5) * _rCellLength + _rMin;
      //const double phiHit = (phiCell+0.5) * _phiCellLength;
      const double phiHit =  phiCell * _phiCellLength + double( (layer+1)%2 )*_zLayerPhiOffset;

      // compute x.y.z hit coordinates ( local )
      const float xHit = rHit * cos(phiHit);
      const float yHit = rHit * sin(phiHit);
      const float zHit = _zFirstLayer + float( layer ) * _zLayerThickness;

      // write x,y,z to an array
      float hitPosV[3] = {xHit, yHit, zHit};
      */
      const float* Pos = calHitIn->getPosition();
      float locPos[3] = {0.0, 0.0, 0.0};
      locPos[0] =  Pos[0]*RotMat[arm]["cos"] - Pos[2]*RotMat[arm]["sin"];
      locPos[1] =  Pos[1];
      locPos[2] =  Pos[0]*RotMat[arm]["sin"] + Pos[2]*RotMat[arm]["cos"];

#if _GENERAL_CLUSTERER_DEBUG == 1
        streamlog_out(DEBUG2) << std::scientific << std::setprecision(3);

        streamlog_out(DEBUG2) << "\t Arm, CellId, Pos(x,y,z), hit energy [MeV]: "
                              << std::setw(5) << arm
                              << std::setw(13)
                              << cellId << "\t ("
                              << std::setw(13) << locPos[0] << ", "
                              << std::setw(13) << locPos[1] << ", "
                              << std::setw(13) << locPos[2] << "), "
                              << 1000.*engyHit
                              <<std::endl;
#endif    
     // 
      // create a new IMPL::CalorimeterHitImpl
      IMPL::CalorimeterHitImpl *calHitNew = new IMPL::CalorimeterHitImpl();

      // write the parameters to the new IMPL::CalorimeterHitImpl
      calHitNew -> setCellID0(cellId);
      calHitNew -> setEnergy(engyHit);
      calHitNew -> setPosition(locPos);

      // add the IMPL::CalorimeterHitImpl to a vector according to the detector
      // arm, and sum the total collected energy at either arm
      calHits[arm][layer].push_back( calHitNew );
      _numHitsInArm[arm]++;
      _totEngyArm[arm] += engyHit;
    }//for all simHits

#if _GENERAL_CLUSTERER_DEBUG == 1
    streamlog_out( MESSAGE4 ) << std::endl  << "Energy deposit: "<< _totEngyArm[-1] << "\t" << _totEngyArm[1] <<"\n"
			   << "Number of hits: "<< _numHitsInArm[-1] << "\t" << _numHitsInArm[1] << "\n\n";
#endif

    if(    (( _numHitsInArm[-1] < _clusterMinNumHits) || (_totEngyArm[-1] < _minClusterEngySignal))
	   && (( _numHitsInArm[ 1] < _clusterMinNumHits) || (_totEngyArm[ 1] < _minClusterEngySignal)) ){
      cleanCalHits( calHits );
      return 0;
    }else{ return 1; }
}

#endif // LumiCalClusterer_getCalHits_h
