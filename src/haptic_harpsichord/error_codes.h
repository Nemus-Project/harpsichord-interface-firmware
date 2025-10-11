/**
 * @file error_codes.h
 * @brief Enum for error codes
 *
 * Error codes for raising errors.
 *
 * @date 2025-10-11
 * @author Matthew Hamilton
 */

enum HapticHarpsichordErrorCodes
{
  FRAM_NOT_FOUND, 
  FRAM_WRITE_FAIL,
  FRAM_READ_FAIL,
  SENSOR_READ_FAIL, 
  SENSOR_VALUE_FAIL,
};