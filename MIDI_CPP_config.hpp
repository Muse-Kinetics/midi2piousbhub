#ifndef MIDI_CPP_CONFIG_H
#define MIDI_CPP_CONFIG_H

/*
  ----------------------------------------------------------------------------
  File:        MIDI_CPP_config.hpp
  Description: Configuration for MIDI_CPP library in midi2piousbhub project

  Copyright © 2025 KMI Music, Inc. All rights reserved.
  Unauthorized copying of this file, via any medium, is strictly prohibited.
  Proprietary and confidential.

  ----------------------------------------------------------------------------
*/

// library includes
#include "MIDI_device_metadata.hpp"

// Product ID - using a the USB ID for the USB hub project
#define SYX_PRODUCT_ID_LSB 0x3C  // BRIDGE

// SysEx ID Reply Versions - simple versioning for hub project
#define SYX_ID_BL_VER1 1  // Bootloader version
#define SYX_ID_BL_VER2 0
#define SYX_ID_BL_VER3 0

#define SYX_ID_APP_VER1 1  // Application version
#define SYX_ID_APP_VER2 0
#define SYX_ID_APP_VER3 0

// Status and error codes - simple return codes
#define SYX_SEND_RETURN_CODE_OK 0                    // success code 
#define SYX_SEND_RETURN_CODE_NO_SEND_FUNCTION -1     // if no syx send function has been defined

// block size to limit size of vectors
#define SYX_TX_BLOCK_SIZE 48 
#define SYX_RX_BLOCK_SIZE 64

#endif /* MIDI_CPP_CONFIG_H */
