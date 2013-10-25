#ifndef BCutilities_hh
#define BCutilities_hh 1

#include <gear/GEAR.h>
#include <gear/GearParameters.h>
#include <gear/LayerLayout.h>
#include <gear/CalorimeterParameters.h>

#include <EVENT/SimCalorimeterHit.h>
#include <UTIL/CellIDDecoder.h>

namespace BCUtil{

  template<class t> inline t vectorprod(const t *a,const  t *b) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
  }

  template<class t> inline t vectorabs(const t *a) {
    return sqrt(vectorprod(a, a));
  }
  
  template<class t> inline t vectorangle(const t *a, const t *b) {
    return acos(vectorprod(a, b)/(vectorabs(a)*vectorabs(b)));
  }

  template<class t> inline void vectoradd(t *a, t *b) {
    a[0] += b[0];
    a[1] += b[1];
    a[2] += b[2];
    a[3] += b[3];
  }

  template<int ANGLE, class t> inline void RotateToBeamCal(const t* vector, t* rotated) {
    const double Cos10mrad = cos(double(ANGLE)/1000.0);
    const double Sin10mrad = sin(double(ANGLE)/1000.0);
    rotated[0] = Cos10mrad*vector[0]-Sin10mrad*vector[2];
    rotated[1] = vector[1];
    rotated[2] = Sin10mrad*vector[0]+Cos10mrad*vector[2];
  }

  template<int ANGLE,class t> inline t AngleToBeamCal(const t* vec) {
    const double Cos10mrad = cos(double(ANGLE)/1000);
    const double Sin10mrad = sin(double(ANGLE)/1000);
    if(vec[2] > 0) {
      const t zP[3] = {Sin10mrad, 0, Cos10mrad};
      return vectorangle(zP, vec);
    } else {
      //-Cos, because the beamcal is on the other side, but still rotated to plus x
      const t zP[3] = {Sin10mrad, 0, -Cos10mrad};
      return vectorangle(zP, vec);
    }  
  }

  template<int ANGLE, class t> void RotateFromBeamCal(const t* vector, t* rotated) {
    const double Cos10mrad = cos(double(ANGLE)/1000.0);
    const double Sin10mrad = sin(double(ANGLE)/1000.0);
    rotated[0] = Cos10mrad*vector[0]+Sin10mrad*vector[2];
    rotated[1] = vector[1];
    rotated[2] = -Sin10mrad*vector[0]+Cos10mrad*vector[2];
  }


  void DecodeCellID(lcio::CellIDDecoder<lcio::SimCalorimeterHit> &mydecoder, const lcio::SimCalorimeterHit* hit, 
		    int& side, int& layer, int& cylinder, int& sector);
 
  // inline const gear::CalorimeterParameters& BCPs() { return marlin::Global::GEAR->getBeamCalParameters(); }

  // //Wrappters around Gear Interface:
  // inline double                getBCInnerRadius(){ return BCPs().getExtent()[0]; }
  // inline double                getBCOuterRadius(){ return BCPs().getExtent()[1]; }
  // inline int                   getBCLayers()     { return BCPs().getLayerLayout().getNLayers(); } 
  // inline int                   getBCRings()      { return BCPs().getDoubleVals("phi_segmentation").size(); }
  // inline std::vector<double>   getSegmentation() { return BCPs().getDoubleVals("phi_segmentation"); }
  // inline std::vector<int>      getNSegments()    { return BCPs().getIntVals("nPhi_segmentation"); }
  // inline double                getCutout()       { return BCPs().getDoubleVal("dead_area_outer_r")+0.1; }
  // inline double getBCZDistanceToIP() {    static const double globalBeamCalDist = 3350; return globalBeamCalDist; }


  inline double BXperYear () { return 134784000000.; } //100 * 24 * 60 * 60 * 50 * 312; // 


  //Functions from BCPadEnergies.hh
  //They should be made to only rely on the GearInterface wrappers from above...
  //Or we write a class which either uses Gear or DD4hep accesses and point to that...












}//Namespace

#endif // BCutilities_hh
