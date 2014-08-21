#include <cmath>

#include "Simulation.h"

void setupTrackByToyMC(SVector3& pos, SVector3& mom, SMatrixSym66& covtrk, std::vector<Hit>& hits, int& charge, float pt, Geometry *theGeom, std::vector<Hit>& initHits) {
  //void setupTrackByToyMC(SVector3& pos, SVector3& mom, SMatrixSym66& covtrk, std::vector<Hit>& hits, int& charge, float pt, Geometry* theGeom) {

  unsigned int nTotHit = theGeom->CountLayers();

  //assume beam spot width 1mm in xy and 1cm in z
  pos=SVector3(0.1*g_gaus(g_gen), 0.1*g_gaus(g_gen), 1.0*g_gaus(g_gen));

  //std::cout << "pos x=" << pos[0] << " y=" << pos[1] << " z=" << pos[2] << std::endl;

  if (charge==0) {
    if (g_unif(g_gen) > 0.5) charge = -1;
    else charge = 1;
  }

  //float phi = 0.5*TMath::Pi()*(1-g_unif(g_gen)); // make an angle between 0 and pi/2 //fixme
  float phi = 0.5*TMath::Pi()*g_unif(g_gen); // make an angle between 0 and pi/2
  float px = pt * cos(phi);
  float py = pt * sin(phi);

  if (g_unif(g_gen)>0.5) px*=-1.;
  if (g_unif(g_gen)>0.5) py*=-1.;
  float pz = pt*(2.3*(g_unif(g_gen)-0.5));//so that we have -1<eta<1

  mom=SVector3(px,py,pz);
  covtrk=ROOT::Math::SMatrixIdentity();
  //initial covariance can be tricky
  for (unsigned int r=0;r<6;++r) {
    for (unsigned int c=0;c<6;++c) {
      if (r==c) {
      if (r<3) covtrk(r,c)=pow(1.0*pos[r],2);//100% uncertainty on position
      else covtrk(r,c)=pow(1.0*mom[r-3],2);  //100% uncertainty on momentum
      } else covtrk(r,c)=0.;                   //no covariance
    }
  }

  //std::cout << "track with p=" << px << " " << py << " " << pz << " pt=" << sqrt(px*px+py*py) << " p=" << sqrt(px*px+py*py+pz*pz) << std::endl;

  const float hitposerrXY = 0.01;//assume 100mum uncertainty in xy coordinate
  const float hitposerrZ = 0.1;//assume 1mm uncertainty in z coordinate
  const float hitposerrR = hitposerrXY/10.;

  TrackState initState;
  initState.parameters=SVector6(pos[0],pos[1],pos[2],mom[0],mom[1],mom[2]);
  initState.errors=covtrk;
  initState.charge=charge;

  TrackState tmpState = initState;

  //do 4 cm in radius using propagation.h
  for (unsigned int nhit=1;nhit<=nTotHit;++nhit) {
    //TrackState propState = propagateHelixToR(tmpState,4.*float(nhit));//radius of 4*nhit
    TrackState propState = propagateHelixToNextSolid(tmpState,theGeom);

    // xy smear
    // float hitx = hitposerrXY*g_gaus(g_gen)+propState.parameters.At(0);
    // float hity = hitposerrXY*g_gaus(g_gen)+propState.parameters.At(1);
    // float hitz = hitposerrZ*g_gaus(g_gen)+propState.parameters.At(2);
    //std::cout << "hit#" << nhit << " " << hitx << " " << hity << " " << hitz << std::endl;
   
    //rphi smear
    float initX   = propState.parameters.At(0);
    float initY   = propState.parameters.At(1);
    float initZ   = propState.parameters.At(2);
    float initPhi = atan2(initY,initX);
    float initRad = sqrt(initX*initX+initY*initY);

    float hitZ    = hitposerrZ*g_gaus(g_gen)+initZ;
    float hitPhi  = ((hitposerrXY/initRad)*g_gaus(g_gen))+initPhi;
    float hitRad  = (hitposerrR)*g_gaus(g_gen)+initRad;
    float hitRad2 = hitRad*hitRad;
    float hitX    = hitRad*cos(hitPhi);
    float hitY    = hitRad*sin(hitPhi);

    float varXY  = hitposerrXY*hitposerrXY;
    float varPhi = varXY/hitRad2;
    float varR   = hitposerrR*hitposerrR;
    float varZ   = hitposerrZ*hitposerrZ;

    SVector3 x1(hitX,hitY,hitZ);
    SMatrixSym33 covx1 = ROOT::Math::SMatrixIdentity();

    //xy smear
    //covx1(0,0)=hitposerrXY*hitposerrXY; 
    //covx1(1,1)=hitposerrXY*hitposerrXY;
    //covx1(2,2)=hitposerrZ*hitposerrZ;

    //rphi smear
    covx1(0,0) = hitX*hitX*varR/hitRad2 + hitY*hitY*varPhi;
    covx1(1,1) = hitX*hitX*varPhi + hitY*hitY*varR/hitRad2;
    covx1(2,2) = varZ;
    covx1(0,1) = hitX*hitY*(varR/hitRad2 - varPhi);
    covx1(1,0) = covx1(0,1);

    Hit hit1(x1,covx1);    
    hits.push_back(hit1);  
    tmpState = propState;
    
    
    SVector3 initVecXYZ(initX,initY,initZ);
    Hit initHitXYZ(initVecXYZ,covx1);
    initHits.push_back(initHitXYZ);

  }

  /*
  //do 4 cm along path
  for (unsigned int nhit=1;nhit<=nTotHit;++nhit) {
    float distance = 4.*float(nhit);//~4 cm distance along curvature between each hit
    float angPath = distance/curvature;
    float cosAP=cos(angPath);
    float sinAP=sin(angPath);
    float hitx = gRandom->Gaus(0,hitposerr)+(pos.At(0) + k*(px*sinAP-py*(1-cosAP)));
    float hity = gRandom->Gaus(0,hitposerr)+(pos.At(1) + k*(py*sinAP+px*(1-cosAP)));
    //float hity = sqrt((pos.At(0) + k*(px*sinAP-py*(1-cosAP)))*(pos.At(0) + k*(px*sinAP-py*(1-cosAP)))+
    //           (pos.At(1) + k*(py*sinAP+px*(1-cosAP)))*(pos.At(1) + k*(py*sinAP+px*(1-cosAP)))-
    //		    	            hitx*hitx);//try to get the fixed radius
    float hitz = gRandom->Gaus(0,hitposerr)+(pos.At(2) + distance*ctgTheta);    
    //std::cout << "hit#" << nhit << " " << hitx << " " << hity << " " << hitz << std::endl;
    SVector3 x1(hitx,hity,hitz);
    SMatrixSym33 covx1 = ROOT::Math::SMatrixIdentity();
    covx1(0,0)=hitposerr*hitposerr; 
    covx1(1,1)=hitposerr*hitposerr;
    covx1(2,2)=hitposerr*hitposerr;
    Hit hit1(x1,covx1);    
    hits.push_back(hit1);
  }
  */

}
