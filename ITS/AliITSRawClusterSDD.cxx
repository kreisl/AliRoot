/**************************************************************************
 * Copyright(c) 2000-2004, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
#include <Riostream.h>
#include <TMath.h>
 
#include "AliITSRawClusterSDD.h"

////////////////////////////////////////////////////
//  Cluster classes for set:ITS                   //
//  Raw Clusters for SDD                          //
//                                                //
////////////////////////////////////////////////////

ClassImp(AliITSRawClusterSDD)
//______________________________________________________________________
AliITSRawClusterSDD::AliITSRawClusterSDD(){
	// default constructor
	fX=fZ=fQ=0;
	fWing=0;
	fNanodes=1;
	fAnode=fTime=fPeakAmplitude=0;
	fPeakPosition=-1;
	fMultiplicity=0;
	fTstart=fTstop=fTstartf=fTstopf=0;
	fAstart=fAstop=0;
}

//______________________________________________________________________
AliITSRawClusterSDD::AliITSRawClusterSDD(Int_t wing,
					 Float_t Anode,Float_t Time,
					 Float_t Charge,Float_t PeakAmplitude,
					 Int_t PeakPosition,
					 Float_t Asigma,Float_t Tsigma,
					 Float_t DriftPath,
					 Float_t AnodeOffset,
					 Int_t Samples,Int_t Tstart,
					 Int_t Tstop,Int_t Tstartf,
					 Int_t Tstopf,Int_t Anodes, 
					 Int_t Astart, Int_t Astop){
    // constructor

    fWing          = wing;
    fAnode         = Anode;
    fTime          = Time;
    fQ             = Charge;
    fPeakAmplitude = PeakAmplitude;
    fPeakPosition  = PeakPosition;
    fAsigma        = Asigma;
    fTsigma        = Tsigma;
    fNanodes       = Anodes;
    fTstart        = Tstart;
    fTstop         = Tstop;
    fTstartf       = Tstartf;
    fTstopf        = Tstopf;
    fAstart        = Astart;
    fAstop         = Astop;
    fMultiplicity  = Samples;
    fSumAmplitude  = 0;

    Int_t sign = 1;
    for(Int_t i=0;i<fWing; i++) sign *= (-1);
    fX = DriftPath*sign/10000.;
    fZ = AnodeOffset/10000.;
}
//______________________________________________________________________
AliITSRawClusterSDD::AliITSRawClusterSDD(const AliITSRawClusterSDD & source):
    AliITSRawCluster(source){
    // copy constructor

    fWing          = source.fWing;
    fAnode         = source.fAnode;
    fTime          = source.fTime;
    fQ             = source.fQ;
    fPeakAmplitude = source.fPeakAmplitude;
    fPeakPosition  = source.fPeakPosition;
    fAsigma        = source.fAsigma;
    fTsigma        = source.fTsigma;
    fNanodes       = source.fNanodes;
    fTstart        = source.fTstart;
    fTstop         = source.fTstop;
    fTstartf       = source.fTstartf;
    fTstopf        = source.fTstopf;
    fAstart        = source.fAstart;
    fAstop         = source.fAstop;

    fMultiplicity  = source.fMultiplicity;
    fSumAmplitude  = source.fSumAmplitude;
    fX             = source.fX;
    fZ             = source.fZ;
}
//______________________________________________________________________
void AliITSRawClusterSDD::Add(AliITSRawClusterSDD* clJ) {
    // add

    fAnode = (fAnode*fQ + clJ->A()*clJ->Q())/(fQ+clJ->Q());
    fTime  = ( fTime*fQ + clJ->T()*clJ->Q())/(fQ+clJ->Q());
    fX     = (    fX*fQ + clJ->X()*clJ->Q())/(fQ+clJ->Q());
    fZ     = (    fZ*fQ + clJ->Z()*clJ->Q())/(fQ+clJ->Q());
    fQ += clJ->Q();
    if(fSumAmplitude == 0) fSumAmplitude += fPeakAmplitude;
    /*
      fAnode = (fAnode*fSumAmplitude+clJ->A()*clJ->PeakAmpl())/
               (fSumAmplitude+clJ->PeakAmpl());
      fTime = (fTime*fSumAmplitude +clJ->T()*clJ->PeakAmpl())/
              (fSumAmplitude+clJ->PeakAmpl());
      fX = (fX*fSumAmplitude +clJ->X()*clJ->PeakAmpl())/
           (fSumAmplitude+clJ->PeakAmpl());
      fZ = (fZ*fSumAmplitude +clJ->Z()*clJ->PeakAmpl())/
           (fSumAmplitude+clJ->PeakAmpl());
    */
    fSumAmplitude += clJ->PeakAmpl();

    fTstart = clJ->Tstart();
    fTstop  = clJ->Tstop();
    if(fTstartf > clJ->Tstartf()) fTstartf = clJ->Tstartf();
    if( fTstopf < clJ->Tstopf() ) fTstopf  = clJ->Tstopf();
    if(  fAstop < clJ->Astop()  ) fAstop   = clJ->Astop();

    fMultiplicity += (Int_t) (clJ->Samples());
    (fNanodes)++;
    if(clJ->PeakAmpl() > fPeakAmplitude) {
	fPeakAmplitude = clJ->PeakAmpl();
	fPeakPosition = clJ->PeakPos();
    } // end if

    return;
}
//______________________________________________________________________
Bool_t AliITSRawClusterSDD::Brother(AliITSRawClusterSDD* cluster,
				    Float_t danode,Float_t dtime) {
  // compare this with "cluster"
    Bool_t brother = kFALSE;
    Bool_t test2 = kFALSE;
    Bool_t test3 = kFALSE;
    Bool_t test4 = kFALSE;
    Bool_t test5 = kFALSE;
  
    if(fWing != cluster->W()) return brother;

    if(fTstopf >= cluster->Tstart() &&
       fTstartf <= cluster->Tstop()) test2 = kTRUE;
    if(cluster->Astop() == (fAstop+1)) test3 = kTRUE;

    if(TMath::Abs(fTime-cluster->T()) < dtime) test4 = kTRUE;
    if(TMath::Abs(fAnode-cluster->A()) < danode) test5 = kTRUE;

    if((test2 && test3) || (test4 && test5) ) {
	return brother = kTRUE;
    } // end if
  
    return brother;
}
//______________________________________________________________________
void AliITSRawClusterSDD::PrintInfo() const {
    // print

    cout << ", Anode " << fAnode << ", Time: " << fTime << ", Charge: " << fQ;
    cout << ", Samples: " << fMultiplicity;
    cout << ", X: " << fX << ", Z: " << fZ << "tstart " << fTstart 
	 << "tstop "<< fTstop <<endl;
}
