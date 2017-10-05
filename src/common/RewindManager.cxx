//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2017 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include <stdlib.h> 

#include "OSystem.hxx"
#include "Serializer.hxx"
#include "StateManager.hxx"
#include "TIA.hxx"

#include "RewindManager.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
RewindManager::RewindManager(OSystem& system, StateManager& statemgr)
  : myOSystem(system),
    myStateManager(statemgr)
{
  myCurrentIt = myStateList.begin();
  
  myLastFrames = myStateCount = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool RewindManager::addState(const string& message)
{
  RewindPtr state = make_unique<RewindState>();  // TODO: get this from object pool
  Serializer& s = state->data;

  s.reset();  // rewind Serializer internal buffers
  if(myStateManager.saveState(s) && myOSystem.console().tia().saveDisplay(s))
  {
    // adding a new save state invalidates all following save states, so we delete them here
    while (myCurrentIt != myStateList.end()) 
      myStateList.pop_back();

    uInt64 size = myStateList.size();
    // check list size and compress if necessary
    if(myStateList.size() == MAX_SIZE)
      compressStates();

    size = myStateList.size();
    myStateCount++;

    state->cycles = myOSystem.console().tia().cycles();
    state->frames = myOSystem.console().tia().frameCount();
    myStateList.push_back(std::move(state));
    myCurrentIt = myStateList.end();

    debugList();
    if(myLastFrames + 1 != myOSystem.console().tia().frameCount()) {      
      uInt64 thisFrames = myOSystem.console().tia().frameCount();
      size = myStateList.size();
    }
    myLastFrames = myOSystem.console().tia().frameCount();

    size = myStateList.size();

    if(myStateCount % 250 == 0) {
      debugList();
      size = myStateList.size();
    }


    return true;
  }
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool RewindManager::rewindState(const bool emulation)
{
  if(myStateList.size() > 0) {
    std::list<RewindPtr>::reverse_iterator rit = myStateList.rbegin();

    while(emulation && myOSystem.console().tia().cycles() - (*rit)->cycles < FRAME_CYCLES * 3 / 4)
      rit++;
    
    Serializer& s = (*rit)->data;

    s.reset();  // rewind Serializer internal buffers
    myStateManager.loadState(s);
    myOSystem.console().tia().loadDisplay(s);

    // Show message indicating the rewind state
    myOSystem.frameBuffer().showMessage(getMessage((*rit)->cycles));

    return true;
  }
  else
    return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool RewindManager::unwindState(bool debugger)
{
  if(myStateList.size() > 0 ) { //&& not at end 



    // Show message indicating the rewind state
    //myOSystem.frameBuffer().showMessage(getMessage(from, to);
    return true;
  }
  else
    return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string RewindManager::getMessage(uInt64 cylcesTo)
{
  // Create variable message with varying units and directions
  stringstream msg;
  uInt64 diff = myOSystem.console().tia().cycles() - cylcesTo;

  if(diff < 0) {
    msg << "Unwind "; diff = -diff;
  } else
    msg << "Rewind ";

  if(diff < 76) 
    msg << diff << " cycle(s)";
  else if(diff < 76 * myOSystem.console().tia().scanlinesLastFrame())
    msg << diff / 76 << " scanline(s)";
  else if(diff < 76 * 262 * 60)
    msg << diff / 76 / 262 / 60 << " frame(s)";
  else if(diff < 76 * 262 * 60 * 60)
    msg << diff / 76 / 262 / 60 / 60 << " second(s)";
  else
    msg << diff / 76 / 262 / 60 / 60 / 60 << " minute(s)";

  return msg.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RewindManager::compressStates()
{
  std::list<RewindPtr>::reverse_iterator rit = myStateList.rbegin();
  //uInt32 topFrame = (*rit)->cycles;
  uInt32 lastStep = 0, stepCount = 0;
  
  //rit += SINGLE_STEPS;
  std::advance(rit, SINGLE_STEPS);
  do {
    std::list<RewindPtr>::reverse_iterator prevRit = rit; prevRit--;
    uInt32 thisStep = (*prevRit)->cycles - (*rit)->cycles;

    if(thisStep == lastStep)
      stepCount++;
    else {              
      // TODO: define emulation or debugger mode and ignore 2nd condition
      if(stepCount >= MERGE_COUNT) {
         //!(lastStep == 64 && (topFrame - (*(rit + 1))->cycles) / 60 < SECOND_STEPS)) {
        std::list<RewindPtr>::reverse_iterator ritErase = rit;
        ritErase--; 
        myStateList.erase(ritErase.base());
        stepCount = 2;
      } else
        stepCount = 1;
      lastStep = thisStep; 
    }    
  } while(++rit != myStateList.rend());

  if(stepCount >= MERGE_COUNT || myStateList.size() == MAX_SIZE) {
    // remove 2nd element
    std::list<RewindPtr>::iterator itErase = myStateList.begin();
    itErase++;
    myStateList.erase(itErase);
  }
  debugList();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void RewindManager::deleteState()
{

}

void RewindManager::debugList()
{
  std::list<RewindPtr>::iterator it = myStateList.begin();
  uInt64 lastFrames = (*it)->frames;

  for(Int32 i = 0; i < myStateList.size(); i++) {
    myFrameLst[i] = (*it)->frames - lastFrames;
    lastFrames = (*it)->frames; 
    it++;
  }
}

