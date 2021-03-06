/*!
LTC SMBus Support: Implementation for a LTC2975 Fault Log

@verbatim

This API is shared with Linduino and RTOS code. End users should code to this
API to enable use of the PMBus code without modifications.

@endverbatim

REVISION HISTORY
$Revision: 4376 $
$Date: 2015-11-20 14:31:33 -0700 (Fri, 20 Nov 2015) $

Copyright (c) 2014, Linear Technology Corp.(LTC)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of Linear Technology Corp.

The Linear Technology Linduino is not affiliated with the official Arduino team.
However, the Linduino is only possible because of the Arduino team's commitment
to the open-source community.  Please, visit http://www.arduino.cc and
http://store.arduino.cc , and consider a purchase that will help fund their
ongoing work.
*/

/*! @file
    @ingroup LT_2975FaultLog
    Library Header File for LT_2975FaultLog
*/

#ifndef LT_2975FaultLog_H_
#define LT_2975FaultLog_H_

#include "../LT_PMBUS/LT_PMBus.h"
#include "../LT_PMBUS/LT_PMBusMath.h"
#include "LT_CommandPlusFaultLog.h"


//! class that handles LTC2975 fault logs.
//! contains structs for interpreting the data read from the part.
class LT_2975FaultLog : public LT_CommandPlusFaultLog
{

  public:
#pragma pack(push, 1)


    /********************************************************************
     * LTC2975 types
     ********************************************************************/

    struct ChanStatus
    {
      public:
        struct RawByte status_vout;
        struct RawByte status_iout;
        struct RawByte status_mfr_specific;
    };

    struct VoutData
    {
      public:
        struct RawByte status_mfr;
        struct RawByte status_vout;
        struct Lin16WordReverse read_vout;
    };

    struct IoutData
    {
      public:
        struct Lin5_11WordReverse read_iout;
        struct RawByte status_iout;
    };

    struct PoutData
    {
      public:
        struct Lin5_11WordReverse read_pout;
    };

    struct TempData
    {
      public:
        struct RawByte status_temp;
        struct Lin5_11WordReverse read_temp1;
    };

    struct VinStatus
    {
      public:
        struct RawByte status_vin;
    };

    struct VinData
    {
      public:
        struct RawByte status_vin;
        struct Lin5_11WordReverse vin;
    };

    struct IinData
    {
      public:
        struct Lin5_11WordReverse read_iin;
    };

    struct PinData
    {
      public:
        struct Lin5_11WordReverse read_pin;
    };

    struct Peak16Words
    {
      public:
        struct Lin16Word peak;
        struct Lin16Word min;
    };

    struct Peak5_11Words
    {
      public:
        struct Lin5_11Word peak;
        struct Lin5_11Word min;
    };

    struct FaultLogPeaksLtc2975
    {
      public:
        struct Peak16Words vout0_peaks;
        struct Peak5_11Words temp0_peaks;
        struct Peak5_11Words iout0_peaks;

        struct Peak5_11Words vin_peaks;
        struct Peak5_11Words iin_peaks;
        struct Peak5_11Words pin_peaks;

        struct Peak16Words vout1_peaks;
        struct Peak5_11Words temp1_peaks;
        struct Peak5_11Words iout1_peaks;

        struct Peak16Words vout2_peaks;
        struct Peak5_11Words temp2_peaks;
        struct Peak5_11Words iout2_peaks;

        struct Peak16Words vout3_peaks;
        struct Peak5_11Words temp3_peaks;
        struct Peak5_11Words iout3_peaks;
    };

    struct FaultLogReadStatusLtc2975
    {
      public:
        struct ChanStatus chan_status0;
        struct ChanStatus chan_status1;
        struct ChanStatus chan_status2;
        struct ChanStatus chan_status3;
    };

    struct FaultLogPreambleLtc2975
    {
      public:
        uint8_t position_last;
        struct FaultLogTimeStamp shared_time;
        struct FaultLogPeaksLtc2975 peaks;
        struct FaultLogReadStatusLtc2975 fault_log_status;
    };

    struct FaultLogReadLoopLtc2975
    {
      public:
        struct PoutData pout_data3;
        struct IoutData iout_data3;
        struct TempData temp_data3;
        struct VoutData vout_data3;
        struct PoutData pout_data2;
        struct IoutData iout_data2;
        struct TempData temp_data2;
        struct VoutData vout_data2;
        struct PoutData pout_data1;
        struct IoutData iout_data1;
        struct TempData temp_data1;
        struct VoutData vout_data1;

        struct PinData pin_data;
        struct IinData iin_data;

        struct RawByte reserved;

        struct VinData vin_data;

        struct PoutData pout_data0;
        struct IoutData iout_data0;
        struct TempData temp_data0;
        struct VoutData vout_data0;

        struct Lin5_11WordReverse read_temp2;
    };

    struct FaultLogLtc2975
    {
      public:
        struct FaultLogPreambleLtc2975 preamble;
        uint8_t telemetryData[176]; // 176 = 255 - 79
        struct FaultLogReadLoopLtc2975 *loops; //loops[0] - loops[4]; lays over telemetry data based on Fault-Log Pointer; 0, 3 have potential for invalid/phantom data; 4 necessarily contains some or all invalid/phantom data; based on following valid byte pointers
        uint8_t firstValidByte; //relative to struct.  firstValidByte should reference start of telemetryData (71).  See datasheet.
        uint8_t lastValidByte; //lastValidByte should reference end of telemetryData (237).  See datasheet.

        bool isValidData(void *pos, uint8_t size = 2); //helper function to identify loop data in valid locations.  Pass in the pointer to the data, and size of the data (length in bytes).
    };

#pragma pack(pop)

  protected:
    FaultLogLtc2975   *faultLog2975;

  public:
    //! Constructor
    LT_2975FaultLog(LT_PMBus *pmbus //!< pmbus object reference for this fault log handler to use.
                   );

    //! Pretty prints this part's fault log to a Print inheriting object, or Serial if none specified.
    void print(Print *printer = 0 //!< Print inheriting object to print the fault log to.
              );

    //! Get binary of the fault log or NULL if no log
    uint8_t *getBinary();

    //! Get size of binary data
    uint16_t getBinarySize();

    //! Dumps binary of the fault log to a Print inheriting object, or Serial if none specified.
    void dumpBinary(Print *printer = 0  //!< Print inheriting object to print the binary to.
                   );

    //! Reads the fault log from the specified address, reserves memory to hold the data.
    //! @return a reference to the data read from the part.
    void read(uint8_t address  //!< the address to read the fault log from.
             );

    // ! Get the fault log data
    struct FaultLogLtc2975 *get()
    {
      return faultLog2975;
    }

    //! Frees the memory reserved for the fault log.
    void release();

  private:
    char *buffer;
    Peak16Words **voutPeaks;
    Peak5_11Words **ioutPeaks;
    Peak5_11Words **tempPeaks;
    ChanStatus **chanStatuses;
    VoutData **voutDatas;
    IoutData **ioutDatas;
    PoutData **poutDatas;
    TempData **tempDatas;

    void printTitle(Print *);
    void printTime(Print *);
    void printPeaks(Print *);
    void printFastChannel(uint8_t index, Print *);
    void printAllLoops(Print *);
    void printLoop(uint8_t index, Print *);
    void printLoopChannel(uint8_t index, Print *);

};

#endif /* LT_FaultLog_H_ */
