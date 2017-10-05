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

#ifndef REWIND_MANAGER_HXX
#define REWIND_MANAGER_HXX

class OSystem;
class StateManager;

#include <list>
#include "bspf.hxx"

/**
  This class is used to save (and later 'rewind') system save states.

  TODO: This will eventually be converted to use object pools
        Currently, it uses a C++ doubly-linked list as a stack, with
        add/remove happening at the front of the list
        Also, the additions are currently unbounded

  @author  Stephen Anthony
*/
class RewindManager
{
  public:
    RewindManager(OSystem& system, StateManager& statemgr);

  public:
    /**
      Add a new state file with the given message; this message will be
      displayed when the state is rewound.

      @param message  Message to display when rewinding to this state
    */
    bool addState(const string& message);

    /**
      Rewind one level of the state list, and display the message associated
      with that state.
    */
    bool rewindDebuggerState()
    {
      return rewindState(false);
    }

    /**
    Rewind ~1s of the state list, and display the message associated
    with that state.
    */
    bool rewindEmulationState()
    {
      return rewindState(true);
    }

    // TODO make private
    bool rewindState(const bool emulation = false);

    /**
    */
    bool unwindDebuggerState()
    {
      return unwindState(false);
    }


    /**
    */
    bool unwindEmulationState()
    {
      return unwindState(true);
    }

    bool empty() const { return myStateList.size() == 0; }
    void clear() { myStateList.clear(); }



  private:
    static constexpr uInt64 FRAME_CYCLES = 76 * 262 * 60;

    static constexpr uInt32 SINGLE_STEPS = 60; // number of guaranteed single cycle rewinds
    static constexpr uInt32 SECOND_STEPS = 10; // number of guaranteed ~60 frames rewinds 
    static constexpr uInt32 MERGE_COUNT = 4;   // threshold for deleting same step entries (4 -> 2/3 each)
    static constexpr uInt32 MAX_SIZE = SINGLE_STEPS + (SECOND_STEPS - MERGE_COUNT) + 46; // Maximum number of states to save

    OSystem& myOSystem;
    StateManager& myStateManager;

    struct RewindState {
      Serializer data;
      uInt64 cycles;
      uInt64 frames; // required for guaranteeing 10s emulation rewind
    };

    using RewindPtr = unique_ptr<RewindState>;
    std::list<RewindPtr> myStateList;
    std::list<RewindPtr>::iterator myCurrentIt;

    uInt64 myFrameLst[MAX_SIZE];
    uInt32 myStateCount;
    uInt64 myLastFrames;

    /**
    */
    //bool rewindState(const bool emulation = false);

    /**
    */
    bool unwindState(bool emulation);

    /**
    */
    void compressStates();
    /**
    */
    void deleteState();

    string getMessage(uInt64 cylcesTo);

    void debugList();

  private:
    // Following constructors and assignment operators not supported
    RewindManager() = delete;
    RewindManager(const RewindManager&) = delete;
    RewindManager(RewindManager&&) = delete;
    RewindManager& operator=(const RewindManager&) = delete;
    RewindManager& operator=(RewindManager&&) = delete;
};

#endif
