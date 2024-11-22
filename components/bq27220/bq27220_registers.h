#pragma once

// commands
#define BQ27220_COMMAND_CONTROL 0X00         // Control()
#define BQ27220_COMMAND_AR 0X02              // AtRate()
#define BQ27220_COMMAND_ARTTE 0X04           // AtRateEmpty()
#define BQ27220_COMMAND_TEMP 0X06            // Temperature()
#define BQ27220_COMMAND_VOLT 0X08            // Voltage()
#define BQ27220_COMMAND_BAT_STA 0X0A         // BatteryStatus()
#define BQ27220_COMMAND_CURR 0X0C            // Current()
#define BQ27220_COMMAND_REMAIN_CAPACITY 0X10 // RemaininfCapacity()
#define BQ27220_COMMAND_FCHG_CAPATICY 0X12   // FullCharageCapacity()
#define BQ27220_COMMAND_AVG_CURR 0x14        // AverageCurrent();
#define BQ27220_COMMAND_TTE 0X16             // TimeToEmpty()
#define BQ27220_COMMAND_TTF 0X18             // TimeToFull()
#define BQ27220_COMMAND_STANDBY_CURR 0X1A    // StandbyCurrent()
#define BQ27220_COMMAND_STTE 0X1C            // StandbyTimeToEmpty()
#define BQ27220_COMMAND_MLTTE 0X20           // MaxLoadTimeToEmpty()
#define BQ27220_COMMAND_RAW_COUL 0X22        // RawCoulombCount()
#define BQ27220_COMMAND_AVG_POW 0X24         // AveragePower()
#define BQ27220_COMMAND_INT_TEMP 0X28        // InternalTemp()
#define BQ27220_COMMAND_CYCLE_CNT 0X2A       // CycleCount()
#define BQ27220_COMMAND_STATE_CHARGE 0X2C    // RelativeStateOfCharge(
#define BQ27220_COMMAND_STATE_HEALTH 0X2E    // StateOfHealth()
#define BQ27220_COMMAND_CHARGING_VOLT 0X30   // ChargeVoltage()
#define BQ27220_COMMAND_CHARGING_CURR 0X32   // ChargeCurrent()
#define BQ27220_COMMAND_BTP_DIS 0x34         // BTPDischargeSet()
#define BQ27220_COMMAND_BTP_CHARGE 0x36      // BTPChargeSet()
#define BQ27220_COMMAND_OP_STATUS 0x3A       // OperationStatus()
#define BQ27220_COMMAND_CAP_DESIGN 0x3C      // Designed Capacity()
#define BQ27220_COMMAND_RAW_CURR 0X7A        // RawCurrent()
#define BQ27220_COMMAND_RAW_VOLT 0X7C        // RawVoltage()
#define BQ27220_COMMAND_RAW_TEMP 0X7E        // RawIntTemp()

// Unseal key
#define BQ27220_UNSEAL_KEY 0x8000