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
 
/* $Id$ */

#include "AliITSRawCluster.h"
////////////////////////////////////////////////////
//  Cluster classes for set:ITS                   //
//  Base class                                    //
//  This set of classes is used for debugging     //
//  purposes                                      //
////////////////////////////////////////////////////
ClassImp(AliITSRawCluster)
//______________________________________________________________________
  AliITSRawCluster::AliITSRawCluster(){
// default constructor
  fMultiplicity = 0;
}

