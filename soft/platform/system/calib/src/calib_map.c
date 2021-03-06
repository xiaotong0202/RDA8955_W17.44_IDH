/* Copyright (C) 2016 RDA Technologies Limited and/or its affiliates("RDA").
* All rights reserved.
*
* This software is supplied "AS IS" without any warranties.
* RDA assumes no responsibility or liability for the use of the software,
* conveys no license or title under any patent, copyright, or mask work
* right to the product. RDA reserves the right to make changes in the
* software without notification.  RDA also make no representation or
* warranty that such application will be suitable for the specified use
* without further testing or modification.
*/


#include "cs_types.h"

#include "hal_map_engine.h"

#include "calibp_calibration.h"

#include "calib_m.h"
#include "calib_map.h"



// =============================================================================
//  MACROS
// =============================================================================

#define CALIB_VERSION_STRING "Calibration Daemon v" \
                             STRINGIFY_VALUE(CALIB_MAJ_VERSION) "." \
                             STRINGIFY_VALUE(CALIB_MIN_VERSION)
#include "calibp_version.h"



// =============================================================================
//  GLOBAL VARIABLES
// =============================================================================


// =============================================================================
// g_calibMapVersion
// -----------------------------------------------------------------------------
/// Contains the version of the module.
/// The macro used is generated by the make process.
// =============================================================================
PROTECTED CONST HAL_MAP_VERSION_T HAL_MAP_VER_CONST_LOC g_calibMapVersion = CALIB_VERSION_STRUCT;



// =============================================================================
//  FUNCTIONS
// =============================================================================


// =============================================================================
// calib_RegisterYourself
// -----------------------------------------------------------------------------
/// This function registers the module itself to HAL so that the version and
/// the map accessor are filled. Then, the CSW get version function and the
/// CoolWatcher get version command will work.
// =============================================================================
PUBLIC VOID calib_RegisterYourself(VOID)
{
    //  Init the pointer used by HST to say talk with the Platform's calibration code.
    //  This will allow HST to read the calib buffer.
    hal_MapEngineRegisterModule(HAL_MAP_ID_CALIB, &g_calibMapVersion, &g_calibCalibration);
}
