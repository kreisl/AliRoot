/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
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

//_________________________________________________________________________
// Manager class for PHOS version for fast simulations
//*-- Author : Y. Schutz SUBATECH 
//////////////////////////////////////////////////////////////////////////////

// --- ROOT system ---

#include "TBRIK.h"
#include "TNode.h"
#include "TParticle.h"

// --- Standard library ---

#include <cstdio>
#include <cassert>

// --- AliRoot header files ---

#include "AliPHOSvFast.h"
#include "AliRun.h"
#include "AliConst.h"

ClassImp(AliPHOSvFast)

//____________________________________________________________________________
AliPHOSvFast::AliPHOSvFast()
{
  fFastRecParticles = 0 ; 
  fNRecParticles = 0 ; 
}

//____________________________________________________________________________
AliPHOSvFast::AliPHOSvFast(const char *name, const char *title):
  AliPHOS(name,title)
{
  // gets an instance of the geometry parameters class  
   
  fGeom =  AliPHOSGeometry::GetInstance(title, "") ; 

  if (fGeom->IsInitialized() ) 
    cout << "AliPHOSvFast : PHOS geometry intialized for " << fGeom->GetName() << endl ;
  else
    cout << "AliPHOSvFast : PHOS geometry initialization failed !" << endl ;   
  
  SetBigBox(0, fGeom->GetOuterBoxSize(0) ) ;
  SetBigBox(1, fGeom->GetOuterBoxSize(1) + fGeom->GetPPSDBoxSize(1) ) ; 
  SetBigBox(2, fGeom->GetOuterBoxSize(0) ); 

  fNRecParticles = 0 ; 
  fFastRecParticles = new FastRecParticlesList("AliPHOSFastRecParticle", 100) ;

  fResPara1 = 0.030 ;    // GeV
  fResPara2 = 0.00003 ; 
  fResPara3 = 0.00001 ; 

  fPosParaA0 = 2.87 ;    // mm
  fPosParaA1 = -0.0975 ;  
  fPosParaB0 = 0.257 ;   
  fPosParaB1 = 0.137 ; 
  fPosParaB2 = 0.00619 ; 
}

//____________________________________________________________________________
AliPHOSvFast::~AliPHOSvFast()
{
 
  fFastRecParticles->Delete() ; 
  delete fFastRecParticles ;
  fFastRecParticles = 0 ; 

}

//____________________________________________________________________________
void AliPHOSvFast::AddRecParticle(const AliPHOSFastRecParticle & rp)
{  
  new( (*fFastRecParticles)[fNRecParticles] ) AliPHOSFastRecParticle(rp) ;
  fNRecParticles++ ; 
}

//____________________________________________________________________________
void AliPHOSvFast::BuildGeometry()
{

  // Build the PHOS geometry for the ROOT display
  
  const Int_t kColorPHOS = kRed ;
  
  Double_t const kRADDEG = 180.0 / kPI ;
  
  new TBRIK( "BigBox", "PHOS box", "void", GetBigBox(0)/2, 
	     GetBigBox(1)/2, 
	     GetBigBox(2)/2 );
  
  // position PHOS into ALICE

  Float_t r = fGeom->GetIPtoOuterCoverDistance() + GetBigBox(1) / 2.0 ;
  Int_t number = 988 ; 
  Float_t pphi =  TMath::ATan( GetBigBox(0)  / ( 2.0 * fGeom->GetIPtoOuterCoverDistance() ) ) ;
  pphi *= kRADDEG ;
  TNode * top = gAlice->GetGeometry()->GetNode("alice") ;
 
  char * nodename = new char[20] ;  
  char * rotname  = new char[20] ; 

  for( Int_t i = 1; i <= fGeom->GetNModules(); i++ ) { 
   Float_t angle = pphi * 2 * ( i - fGeom->GetNModules() / 2.0 - 0.5 ) ;
   sprintf(rotname, "%s%d", "rot", number++) ;
   new TRotMatrix(rotname, rotname, 90, angle, 90, 90 + angle, 0, 0);
   top->cd();
   sprintf(nodename,"%s%d", "Module", i) ;    
   Float_t x =  r * TMath::Sin( angle / kRADDEG ) ;
   Float_t y = -r * TMath::Cos( angle / kRADDEG ) ;
   TNode * bigboxnode = new TNode(nodename, nodename, "BigBox", x, y, 0, rotname ) ;
   bigboxnode->SetLineColor(kColorPHOS) ;
   fNodes->Add(bigboxnode) ;
  }
  delete[] nodename ; 
  delete[] rotname ; 
}

//____________________________________________________________________________
void AliPHOSvFast::CreateGeometry()
{

  AliPHOSvFast *phostmp = (AliPHOSvFast*)gAlice->GetModule("PHOS") ;

  if ( phostmp == NULL ) {
    
    fprintf(stderr, "PHOS detector not found!\n") ;
    return ;
    
  }

  // Get pointer to the array containing media indeces
  Int_t *idtmed = fIdtmed->GetArray() - 699 ;
  
  Float_t bigbox[3] ; 
  bigbox[0] =   GetBigBox(0) / 2.0 ;
  bigbox[1] =   GetBigBox(1) / 2.0 ;
  bigbox[2] =   GetBigBox(2) / 2.0 ;
  
  gMC->Gsvolu("PHOS", "BOX ", idtmed[798], bigbox, 3) ;
  
  // --- Position  PHOS mdules in ALICE setup ---
  
  Int_t idrotm[99] ;
  Double_t const kRADDEG = 180.0 / kPI ;
  
  for( Int_t i = 1; i <= fGeom->GetNModules(); i++ ) {
    
    Float_t angle = fGeom->GetPHOSAngle(i) ;
    AliMatrix(idrotm[i-1], 90.0, angle, 90.0, 90.0+angle, 0.0, 0.0) ;
 
    Float_t r = fGeom->GetIPtoOuterCoverDistance() + GetBigBox(1) / 2.0 ;

    Float_t xP1 = r * TMath::Sin( angle / kRADDEG ) ;
    Float_t yP1 = -r * TMath::Cos( angle / kRADDEG ) ;
    gMC->Gspos("PHOS", i, "ALIC", xP1, yP1, 0.0, idrotm[i-1], "ONLY") ;
 
  } // for GetNModules

}


//____________________________________________________________________________
void AliPHOSvFast::Init(void)
{
 
  Int_t i;

  printf("\n");
  for(i=0;i<35;i++) printf("*");
  printf(" FAST PHOS_INIT ");
  for(i=0;i<35;i++) printf("*");
  printf("\n");

  // Here the PHOS initialisation code (if any!)

  for(i=0;i<80;i++) printf("*");
  printf("\n");
  
}

//___________________________________________________________________________
Float_t AliPHOSvFast::GetBigBox(Int_t index)
{
  Float_t rv = 0 ; 

  switch (index) {
  case 0:
    rv = fBigBoxX ; 
    break ; 
  case 1:
     rv = fBigBoxY ; 
    break ; 
  case 2:
     rv = fBigBoxZ ; 
    break ; 
 }
  return rv ; 
}

//___________________________________________________________________________
void AliPHOSvFast::MakeBranch(Option_t* opt)
{  
  //
  // Create a new branch in the current Root Tree
 
  AliDetector::MakeBranch(opt) ;
  
  char branchname[10];
  sprintf(branchname,"%s",GetName());
  char *cd = strstr(opt,"R");
  
  if (fFastRecParticles && gAlice->TreeR() && cd) {
    gAlice->TreeR()->Branch(branchname, &fFastRecParticles, fBufferSize);
  }
}

//____________________________________________________________________________
Double_t AliPHOSvFast::MakeEnergy(const Double_t energy)
{  
  Double_t sigma  = SigmaE(energy) ; 
  return  fRan.Gaus(energy, sigma) ;   
}

//____________________________________________________________________________
TVector3 AliPHOSvFast::MakePosition(const Double_t energy, const TVector3 pos, const Double_t theta, const Double_t phi)
{
  TVector3 newpos ;
  Double_t sigma = SigmaP( energy, theta*180./TMath::Pi() ) ;
  Double_t x = fRan.Gaus( pos.X(), sigma ) ;
  sigma = SigmaP( energy, phi*180./TMath::Pi() ) ;
  Double_t z = fRan.Gaus( pos.Z(), sigma ) ;
  Double_t y = pos.Y() ; 
  
  newpos.SetX(x) ; 
  newpos.SetY(y) ; 
  newpos.SetZ(z) ; 
	      
  return newpos ; 
}

//____________________________________________________________________________
void AliPHOSvFast::MakeRecParticle(const Int_t modid, const TVector3 pos, AliPHOSFastRecParticle & rp)
{
  // get the detected type of particle
 
  Int_t type = MakeType( rp ) ;
  rp.SetType(type) ;

  
  // get the detected energy

  TLorentzVector momentum ;  
  rp.Momentum(momentum) ; 
  Double_t kineticenergy = TMath::Sqrt( TMath::Power(momentum.E(), 2) - TMath::Power(rp.GetMass(), 2) ) ; 
  Double_t modifiedkineticenergy = MakeEnergy(kineticenergy ) ;
  Double_t modifiedenergy = TMath::Sqrt( TMath::Power(modifiedkineticenergy, 2)  
					 + TMath::Power( rp.GetMass(), 2) ) ;
 
  // get the angle of incidence 
  
  Double_t incidencetheta = 90. * TMath::Pi() /180 - rp.Theta() ; 
  Double_t incidencephi   = ( 270 + fGeom->GetPHOSAngle(modid) ) * TMath::Pi() / 180. - rp.Phi() ;   

  // get the detected direction
  
  TVector3 modifiedposition = MakePosition(kineticenergy, pos, incidencetheta, incidencephi) ; 
  modifiedposition *= modifiedkineticenergy / modifiedposition.Mag() ; 

  // Set the modified 4-momentum of the reconstructed particle

  rp.SetMomentum(modifiedposition.X(), modifiedposition.Y(), modifiedposition.Z(), modifiedenergy) ; 

 }

//____________________________________________________________________________
Int_t AliPHOSvFast::MakeType(AliPHOSFastRecParticle & rp )
{
  Int_t rv =  kUNDEFINED ;
  Int_t charge = (Int_t)rp.GetPDG()->Charge() ;
  Int_t test ; 
  Float_t ran ; 
  if ( charge != 0 && ( TMath::Abs(rp.GetPdgCode()) != 11 ) ) 
    test = - 1 ;
  else
    test = rp.GetPdgCode() ; 

  switch (test) { 

  case 22:    // it's a photon
    ran = fRan.Rndm() ; 
    if ( ran <= 0.5 )  // 50 % 
      rv = kGAMMA ; 
    else {
      ran = fRan.Rndm() ; 
      if( ran <= 0.9498 )
	rv = kNEUTRALEM ; 
      else
	rv = kNEUTRALHADRON ; 
    }     
    break ; 

  case 2112:  // it's a neutron
    ran = fRan.Rndm() ; 
    if ( ran <= 0.9998 )
      rv = kNEUTRALHADRON ; 
    else 
      rv = kNEUTRALEM ; 
    break ; 

  case -2112: // it's a anti-neutron
    ran = fRan.Rndm() ; 
    if ( ran <= 0.9984 )
      rv = kNEUTRALHADRON ; 
    else 
      rv = kNEUTRALEM ; 
    break ; 
    
  case 11:    // it's a electron
    ran = fRan.Rndm() ; 
    if ( ran <= 0.9996 )
      rv = kELECTRON ; 
    else 
      rv = kCHARGEDHADRON ; 
    break; 

  case -11:   // it's a positon
    ran = fRan.Rndm() ; 
    if ( ran <= 0.9996 )
      rv = kELECTRON ; 
    else 
      rv = kCHARGEDHADRON ; 
    break; 

  case -1:    // it's a charged
    ran = fRan.Rndm() ; 
    if ( ran <= 0.9996 )
      rv = kCHARGEDHADRON ; 
    else 
      rv = kGAMMA ; 

    break ; 
  }
    
  
  return rv ;
}

//___________________________________________________________________________
void AliPHOSvFast::ResetPoints()
{
  ResetFastRecParticles() ; 
}

//___________________________________________________________________________
void AliPHOSvFast::ResetFastRecParticles()
{
  if (fFastRecParticles) 
    fFastRecParticles->Clear() ;
  fNRecParticles = 0 ; 
}

//___________________________________________________________________________
void AliPHOSvFast::SetBigBox(Int_t index, Float_t value)
{

  switch (index) {
  case 0:
    fBigBoxX = value ; 
    break ; 
  case 1:
    fBigBoxY = value ; 
    break ; 
  case 2:
    fBigBoxZ = value ; 
    break ; 
 }

}

//____________________________________________________________________________
Double_t AliPHOSvFast::SigmaE(Double_t energy)
{
  Double_t rv = -1 ; 
  
  rv = TMath::Sqrt( TMath::Power(fResPara1/energy, 2) 
	       + TMath::Power(fResPara2/TMath::Sqrt(energy), 2) 
	       + TMath::Power(fResPara3, 2) ) ;  

  return rv * energy ; 
}

//____________________________________________________________________________
Double_t AliPHOSvFast::SigmaP(Double_t energy, Int_t incidence)
{
 
  Double_t paraA = fPosParaA0 + fPosParaA1 * incidence ; 
  Double_t paraB = fPosParaB0 + fPosParaB1 * incidence + fPosParaB2 * incidence * incidence ; 

  return ( paraA / TMath::Sqrt(energy) + paraB ) * 0.1   ; // in cm  
}

//____________________________________________________________________________
void AliPHOSvFast::StepManager(void)
{

  Int_t primary =  gAlice->GetPrimary( gAlice->CurrentTrack() ); 
  TLorentzVector lv ; 
  gMC->TrackPosition(lv) ;
  TVector3 pos = lv.Vect() ; 
  Int_t modid  ; 
  gMC->CurrentVolID(modid);
  
  // Makes a reconstructed particle from the primary particle

  TClonesArray * particlelist = gAlice->Particles() ;
  TParticle * part = (TParticle *)particlelist->At(primary) ;  

  AliPHOSFastRecParticle rp(*part) ;

  // Adds the response of PHOS to the particle

  MakeRecParticle(modid, pos, rp) ;
  
  // add the primary particle to the FastRecParticles list
  
  AddRecParticle(rp) ;

  // stop the track as soon PHOS is reached
  
  gMC->StopTrack() ; 

}

