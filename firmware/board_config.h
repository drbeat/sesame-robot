// =============================================================================
// board_config.h — Sesame Robot hardware configuration
// =============================================================================
// All board-specific pin assignments and hardware constants live here.
//
// Select your target at compile time by defining one of:
//   -DBOARD_LOLIN_S2_MINI      Lolin S2 Mini (ESP32-S2)
//   -DBOARD_DISTRO_V1          Sesame Distro Board V1  (ESP32-WROOM32 / Dev Module)
//   -DBOARD_DISTRO_V2          Sesame Distro Board V2  (ESP32-S3)
//
// In Arduino IDE: Tools → Board, then uncomment the matching section below
// and re-comment the others (old behaviour still works).
//
// In GitHub Actions / arduino-cli the flag is injected automatically per board
// via --build-property so you never need to edit this file manually in CI.
// =============================================================================
#pragma once

// ─── Display (same on every board) ───────────────────────────────────────────
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_I2C_ADDR  0x3C

// ─── Board selection ─────────────────────────────────────────────────────────

#if defined(BOARD_LOLIN_S2_MINI)
// -----------------------------------------------------------------------------
// Lolin S2 Mini (ESP32-S2)
// -----------------------------------------------------------------------------
#define BOARD_NAME "Lolin S2 Mini"
#define I2C_SDA 33
#define I2C_SCL 35
#define SERVO_PINS { 1, 2, 4, 6, 8, 10, 13, 14 }

#elif defined(BOARD_DISTRO_V1)
// -----------------------------------------------------------------------------
// Sesame Distro Board V1 — ESP32-WROOM32 / ESP32 Dev Module
// -----------------------------------------------------------------------------
#define BOARD_NAME "Sesame Distro Board V1"
#define I2C_SDA 21
#define I2C_SCL 22
#define SERVO_PINS { 15, 2, 23, 19, 4, 16, 17, 18 }

#elif defined(BOARD_DISTRO_V2)
// -----------------------------------------------------------------------------
// Sesame Distro Board V2 — ESP32-S3
// -----------------------------------------------------------------------------
#define BOARD_NAME "Sesame Distro Board V2"
#define I2C_SDA 8
#define I2C_SCL 9
#define SERVO_PINS { 4, 5, 6, 7, 15, 16, 17, 18 }

#else
// -----------------------------------------------------------------------------
// Fallback — no board flag provided (local Arduino IDE builds without a flag).
// Mirrors the Lolin S2 Mini layout which is the primary development board.
// -----------------------------------------------------------------------------
#warning "No BOARD_* define set. Defaulting to Lolin S2 Mini pin layout."
#define BOARD_NAME "Unknown (S2 Mini fallback)"
#define I2C_SDA 33
#define I2C_SCL 35
#define SERVO_PINS { 1, 2, 4, 6, 8, 10, 13, 14 }

#endif // board selection
