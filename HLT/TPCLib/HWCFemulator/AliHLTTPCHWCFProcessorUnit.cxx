// $Id$
//****************************************************************************
//* This file is property of and copyright by the ALICE HLT Project          * 
//* ALICE Experiment at CERN, All rights reserved.                           *
//*                                                                          *
//* Primary Authors: Sergey Gorbunov, Torsten Alt                            *
//* Developers:      Sergey Gorbunov <sergey.gorbunov@fias.uni-frankfurt.de> *
//*                  Torsten Alt <talt@cern.ch>                              *
//*                  for The ALICE HLT Project.                              *
//*                                                                          *
//* Permission to use, copy, modify and distribute this software and its     *
//* documentation strictly for non-commercial purposes is hereby granted     *
//* without fee, provided that the above copyright notice appears in all     *
//* copies and that both the copyright notice and this permission notice     *
//* appear in the supporting documentation. The authors make no claims       *
//* about the suitability of this software for any purpose. It is            *
//* provided "as is" without express or implied warranty.                    *
//****************************************************************************

//  @file   AliHLTTPCHWCFProcessorUnit.cxx
//  @author Sergey Gorbunov <sergey.gorbunov@fias.uni-frankfurt.de>
//  @author Torsten Alt <talt@cern.ch> 
//  @date   
//  @brief  Channel Processor unit of FPGA ClusterFinder Emulator for TPC
//  @brief  ( see AliHLTTPCHWCFEmulator class )
//  @note 

#include "AliHLTTPCHWCFProcessorUnit.h"
#include <iostream>
#include <cstdio>


AliHLTTPCHWCFProcessorUnit::AliHLTTPCHWCFProcessorUnit()
  :
  fOutput(),
  fkBunch(0),
  fBunchIndex(0),
  fWasDeconvoluted(0),
  fDeconvolute(0),
  fImprovedDeconvolution(0),
  fSingleSeqLimit(0),
  fUseTimeBinWindow(0),
  fDebug(0)
{
  //constructor 
  Init();
}


AliHLTTPCHWCFProcessorUnit::~AliHLTTPCHWCFProcessorUnit()
{   
  //destructor 
}

AliHLTTPCHWCFProcessorUnit::AliHLTTPCHWCFProcessorUnit(const AliHLTTPCHWCFProcessorUnit&)
  :
  fOutput(),
  fkBunch(0),
  fBunchIndex(0),
  fWasDeconvoluted(0),
  fDeconvolute(0),
  fImprovedDeconvolution(0),
  fSingleSeqLimit(0),
  fUseTimeBinWindow(0),
  fDebug(0)
{
  // dummy
  Init();
}

AliHLTTPCHWCFProcessorUnit& AliHLTTPCHWCFProcessorUnit::operator=(const AliHLTTPCHWCFProcessorUnit&)
{
  // dummy  
  return *this;
}

int AliHLTTPCHWCFProcessorUnit::Init()
{
  // initialise  

  fkBunch = 0;
  fBunchIndex = 0;
  fWasDeconvoluted = 0;
  return 0;
}

int AliHLTTPCHWCFProcessorUnit::InputStream( const AliHLTTPCHWCFBunch *bunch )
{
  // input stream of data 
  
  if( bunch && fDebug ){
    printf("\nHWCF Processor: input bunch F %1d R %3d P %3d  NS %2ld:\n",
	   bunch->fFlag, bunch->fRow, bunch->fPad, bunch->fData.size());
    for( unsigned int i=0; i<bunch->fData.size(); i++ ){
      AliHLTTPCHWCFDigit d =  bunch->fData[i];
      printf("   q %2d t %3d peak %2d ", d.fQ, d.fTime, d.fPeak);      
       printf("(");
      for( int j=0; j<3; j++ ) printf(" {%d,%2.0f}",d.fMC.fClusterID[j].fMCID, d.fMC.fClusterID[j].fWeight );
      printf(" )\n");    
    }
  }

  fkBunch = bunch;
  fBunchIndex = 0;
  fWasDeconvoluted = 0;
  return 0;
}

const AliHLTTPCHWCFClusterFragment *AliHLTTPCHWCFProcessorUnit::OutputStream()
{ 
  // output stream of data 

  //const AliHLTUInt32_t kTimeBinWindow = 5;
  const AliHLTUInt32_t kHalfTimeBinWindow = 2;
 
  if( !fkBunch ) return 0;
  
  fOutput.fFlag = fkBunch->fFlag;
  fOutput.fRow = fkBunch->fRow;
  fOutput.fPad = fkBunch->fPad;
  fOutput.fBranch = fkBunch->fBranch;
  fOutput.fBorder = fkBunch->fBorder;
  fOutput.fEdge = 0; //Set below with some more constraints
  fOutput.fQmax = 0;
  fOutput.fQ = 0;
  fOutput.fT = 0;
  fOutput.fT2 = 0;
  fOutput.fP = 0;
  fOutput.fP2 = 0;
  fOutput.fTMean = 0;
  fOutput.fNPads = 0;
  fOutput.fNDeconvolutedTime = 0;
  fOutput.fIsDeconvolutedPad = 0;
  fOutput.fConsecutiveTimeDeconvolution = 0;
  fOutput.fLargestQ = 0;
  fOutput.fLargestQ2 = 0;
  fOutput.fLargestQPad = 0;
  fOutput.fMC.clear();
  
  if( fkBunch->fFlag==2 && fkBunch->fData.size()==1 ){ // rcu trailer word, forward it 
    fOutput.fRow = fkBunch->fData[0].fQ;
  }
  
  if( fkBunch->fFlag >1 ){
    fkBunch = 0;    
    return &fOutput;
  }

  if( fkBunch->fFlag < 1 ) return 0;
  
  if( fBunchIndex >= fkBunch->fData.size() ) return 0;
    
  AliHLTUInt32_t iStart = fBunchIndex;
  AliHLTUInt32_t iPeak = fBunchIndex;
  AliHLTUInt32_t qPeak = 0;

  // find next/best peak
    
  for( ; fBunchIndex<fkBunch->fData.size(); fBunchIndex++ ){
    const AliHLTTPCHWCFDigit &d = fkBunch->fData[fBunchIndex];            
    if( d.fPeak != 1 ) continue;
    if( fDeconvolute ){
      iPeak = fBunchIndex;
      qPeak = d.fQ;
      fBunchIndex++;
      break;
    } else {	
      if( d.fQ>qPeak ){
	qPeak = d.fQ;
	iPeak = fBunchIndex;
      }
    }
  }
    
  if( qPeak == 0 ) return 0;

  // find next minimum !!! At the moment the minimum finder is on only when no timebin window set

  bool isDeconvoluted = 0;

  if( !fUseTimeBinWindow ){
    for( ; fBunchIndex<fkBunch->fData.size(); fBunchIndex++ ){
      if( fDeconvolute ){
	if( fkBunch->fData[fBunchIndex].fPeak != 0 ){	    
	  fBunchIndex++;
	  break;
	}
      }
    }
    if( fBunchIndex<fkBunch->fData.size() ) isDeconvoluted = 1;    
  } else { 
    if( !fDeconvolute ){
      fBunchIndex = fkBunch->fData.size();
    } else if ( fImprovedDeconvolution) {
      fWasDeconvoluted = 0;
      for ( int i = std::max((int) iStart, (int) (iPeak - kHalfTimeBinWindow)); i < iPeak; i++) {
        if ( fkBunch->fData[i].fPeak == 2 )
        {
          iStart = i + 1;
          fWasDeconvoluted = 1;
        }
      }
      //Do not mark as deconvoluted when iPeak + 2 is a minimum (because we use that sample), do not consider minima add end of bunch!
      for ( ; fBunchIndex < fkBunch->fData.size(); fBunchIndex++ ) {
        if (fBunchIndex == iPeak + kHalfTimeBinWindow)
        {
          fBunchIndex++;
          break;
        }
        if (fkBunch->fData[fBunchIndex].fPeak == 2) {
          if (fBunchIndex + 1 != fkBunch->fData.size()) isDeconvoluted = 1;
          fBunchIndex++;
          break;
        }
      }
    } else {
      // find next peak
      if( fBunchIndex+1<fkBunch->fData.size() && fkBunch->fData[fBunchIndex+1].fPeak==1 ){
	fBunchIndex = fBunchIndex+1;
	isDeconvoluted = 1; 
      } else   if( fBunchIndex+2<fkBunch->fData.size() && fkBunch->fData[fBunchIndex+2].fPeak==1 ){
	fBunchIndex = fBunchIndex+1;
	isDeconvoluted = 1; 
      } else   if( fBunchIndex+3<fkBunch->fData.size() && fkBunch->fData[fBunchIndex+3].fPeak==1 ){
	fBunchIndex = fBunchIndex+2;
	isDeconvoluted = 1; 
      } else   if( fBunchIndex+1<fkBunch->fData.size() ){
	fBunchIndex = fBunchIndex+2;
      } else   if( fBunchIndex<fkBunch->fData.size() ){
	fBunchIndex = fBunchIndex+1;
      }
    }
  }
    
  AliHLTUInt32_t iEnd = fBunchIndex;

  if( fUseTimeBinWindow ){
    if( iPeak > iStart + kHalfTimeBinWindow ) iStart = iPeak - kHalfTimeBinWindow;
    if( iEnd  > iPeak + kHalfTimeBinWindow + 1) iEnd = iPeak + kHalfTimeBinWindow + 1;
  }

  fOutput.fQmax = 0;
  fOutput.fQ = 0;
  fOutput.fT = 0;
  fOutput.fT2 = 0;
  fOutput.fP = 0;
  fOutput.fP2 = 0;
  fOutput.fTMean = fkBunch->fData[iPeak].fTime;
  fOutput.fNPads = 1;
  fOutput.fNDeconvolutedTime = ( fWasDeconvoluted || isDeconvoluted ) ? 1 :0;
  fOutput.fConsecutiveTimeDeconvolution = fOutput.fNDeconvolutedTime;
  fOutput.fMC.clear();

  fWasDeconvoluted = isDeconvoluted;  

  for( AliHLTUInt32_t i=iStart; i<iEnd; i++ ){
    const AliHLTTPCHWCFDigit &d = fkBunch->fData[i];
    AliHLTUInt64_t q = d.fQ*fkBunch->fGain;
    if (q > fOutput.fQmax) fOutput.fQmax = q;
    fOutput.fQ += q;
    fOutput.fT += q*d.fTime;
    fOutput.fT2+= q*d.fTime*d.fTime;
    fOutput.fP += q*fkBunch->fPad;
    fOutput.fP2+= q*fkBunch->fPad*fkBunch->fPad;
    fOutput.fMC.push_back(d.fMC);
  }
  fOutput.fEdge = fkBunch->fEdge && iEnd > iStart + 2 && qPeak > 5;
  fOutput.fLargestQ = fOutput.fQ;
  fOutput.fLargestQPad = fkBunch->fEdge ? fkBunch->fPad : 0xFFFFFFFF; //We only correct if the largest charge is at the edge cluster. Mark other pads as -1
  
  if( fkBunch->fData.size()==1 && fOutput.fQ < fSingleSeqLimit ) return 0;  
  
  return &fOutput;
}
