
//! \file RzErrors.h
//! \brief Error codes.

#ifndef _RZERRORS_H_
#define _RZERRORS_H_

#pragma once

// Error codes
//! Invalid
#define     RZRESULT_INVALID                    -1L
//! Success
#define     RZRESULT_SUCCESS                    0L
//! Access denied
#define     RZRESULT_ACCESS_DENIED              5L
//! Not supported
#define     RZRESULT_NOT_SUPPORTED              50L
//! Invalid parameter.
#define     RZRESULT_INVALID_PARAMETER          87L
//! Cannot start more than one instance of the specified program.
#define     RZRESULT_SINGLE_INSTANCE_APP        1152L
//! Request aborted.
#define     RZRESULT_REQUEST_ABORTED            1235L
//! Resource not available or disabled
#define     RZRESULT_RESOURCE_DISABLED          4309L
//! General failure.
#define     RZRESULT_FAILED                     2147500037L

#endif
