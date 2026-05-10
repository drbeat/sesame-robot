#pragma once

#include <Arduino.h>

enum ServoName : uint8_t {
  R1 = 0, 
  R2 = 1,
  L1 = 2,
  L2 = 3,
  R4 = 4,
  R3 = 5,
  L3 = 6,
  L4 = 7
};

const String ServoNames[]={"R1","R2","L1","L2","R4","R3","L3","L4"};

inline int servoNameToIndex(const String& servo) {
  if (servo == "L1") return L1;
  if (servo == "L2") return L2;
  if (servo == "L3") return L3;
  if (servo == "L4") return L4;
  if (servo == "R1") return R1;
  if (servo == "R2") return R2;
  if (servo == "R3") return R3;
  if (servo == "R4") return R4;
  return -1;
}

inline int getServoIndex(const String& input) {
  // Try name first (e.g. "L1")
  int idx = servoNameToIndex(input);
  if (idx != -1) return idx;
  
  // Try numeric (1-8) - note: toInt() returns 0 on failure
  int num = input.toInt();
  if (num >= 1 && num <= 8) return num - 1;
  
  return -1;
}

enum FaceAnimMode : uint8_t {
  FACE_ANIM_LOOP = 0,
  FACE_ANIM_ONCE = 1,
  FACE_ANIM_BOOMERANG = 2
};

enum RobotCommand : uint8_t {
  CMD_NONE = 0,
  CMD_FORWARD,
  CMD_BACKWARD,
  CMD_LEFT,
  CMD_RIGHT,
  CMD_REST,
  CMD_STAND,
  CMD_WAVE,
  CMD_DANCE,
  CMD_SWIM,
  CMD_POINT,
  CMD_PUSHUP,
  CMD_BOW,
  CMD_CUTE,
  CMD_FREAKY,
  CMD_WORM,
  CMD_SHAKE,
  CMD_SHRUG,
  CMD_DEAD,
  CMD_CRAB
};

// External globals and helpers used by movement/pose sequences
extern int frameDelay;
extern int walkCycles;
extern RobotCommand currentCommand;
extern RobotCommand getSafeCommand();
extern void setSafeCommand(RobotCommand cmd);

extern void setServoAngle(uint8_t channel, int angle);
extern void setFace(const String& faceName);
extern void setFaceMode(FaceAnimMode mode);
extern void setFaceWithMode(const String& faceName, FaceAnimMode mode);
extern void delayWithFace(unsigned long ms);
extern void enterIdle();
extern bool pressingCheck(RobotCommand cmd, int ms);

// Pose/animation prototypes
void runRestPose();
void runStandPose(int face = 1);
void runWavePose();
void runDancePose();
void runSwimPose();
void runPointPose();
void runPushupPose();
void runBowPose();
void runCutePose();
void runFreakyPose();
void runWormPose();
void runShakePose();
void runShrugPose();
void runDeadPose();
void runCrabPose();
void runWalkPose();
void runWalkBackward();
void runTurnLeft();
void runTurnRight();

// ====== POSES ======
inline void runRestPose() { 
  Serial.println(F("REST")); 
  setFaceWithMode("rest", FACE_ANIM_BOOMERANG); 
  for (int i = 0; i < 8; i++) setServoAngle(i, 90); 
}

inline void runStandPose(int face) { 
  Serial.println(F("STAND")); 
  if (face == 1) setFaceWithMode("stand", FACE_ANIM_ONCE); 
  setServoAngle(R1, 135); 
  setServoAngle(R2, 45); 
  setServoAngle(L1, 45); 
  setServoAngle(L2, 135); 
  setServoAngle(R4, 0); 
  setServoAngle(R3, 180); 
  setServoAngle(L3, 0); 
  setServoAngle(L4, 180); 
  if (face == 1) enterIdle();
}

inline void runWavePose() { 
  Serial.println(F("WAVE")); 
  setFaceWithMode("wave", FACE_ANIM_ONCE); 
  runStandPose(0); 
  delayWithFace(200);
  setServoAngle(R4, 80); setServoAngle(L3, 180); 
  setServoAngle(L2, 90); setServoAngle(R1, 100); 
  delayWithFace(200);
  setServoAngle(L3, 180); 
  delayWithFace(300); 
  for (int i = 0; i < 4; i++) { 
    setServoAngle(L3, 180); delayWithFace(300); 
    setServoAngle(L3, 100); delayWithFace(300); 
  } 
  runStandPose(1); 
  if (currentCommand == CMD_WAVE) currentCommand = CMD_NONE;
}

inline void runDancePose() { 
  Serial.println(F("DANCE")); 
  setFaceWithMode("dance", FACE_ANIM_LOOP); 
  setServoAngle(R1, 90); setServoAngle(R2, 90); 
  setServoAngle(L1, 90); setServoAngle(L2, 90); 
  setServoAngle(R4, 160); setServoAngle(R3, 160); 
  setServoAngle(L3, 10); setServoAngle(L4, 10); 
  delayWithFace(300); 
  for (int i = 0; i < 5; i++) { 
    setServoAngle(R4, 115); setServoAngle(R3, 115); 
    setServoAngle(L3, 10); setServoAngle(L4, 10); 
    delayWithFace(300); 
    setServoAngle(R4, 160); setServoAngle(R3, 160); 
    setServoAngle(L3, 65); setServoAngle(L4, 65); 
    delayWithFace(300); 
  } 
  runStandPose(1); 
  if (currentCommand == CMD_DANCE) currentCommand = CMD_NONE;
}

inline void runSwimPose() { 
  Serial.println(F("SWIM")); 
  setFaceWithMode("swim", FACE_ANIM_ONCE); 
  for (int i = 0; i < 8; i++) setServoAngle(i, 90); 
  for (int i = 0; i < 4; i++) { 
    setServoAngle(R1, 135); setServoAngle(R2, 45); 
    setServoAngle(L1, 45); setServoAngle(L2, 135); 
    delayWithFace(400); 
    setServoAngle(R1, 90); setServoAngle(R2, 90); 
    setServoAngle(L1, 90); setServoAngle(L2, 90); 
    delayWithFace(400); 
  } 
  runStandPose(1); 
  if (currentCommand == CMD_SWIM) currentCommand = CMD_NONE;
}

inline void runPointPose() { 
  Serial.println(F("POINT")); 
  setFaceWithMode("point", FACE_ANIM_BOOMERANG); 
  setServoAngle(L2, 90); setServoAngle(R1, 135); 
  setServoAngle(R2, 100); setServoAngle(L4, 180); 
  setServoAngle(L1, 25); setServoAngle(L3, 145);
  setServoAngle(R4, 80); setServoAngle(R3, 170); 
  delayWithFace(2000); 
  runStandPose(1); 
  if (currentCommand == CMD_POINT) currentCommand = CMD_NONE;
}

inline void runPushupPose() {
  Serial.println(F("PUSHUP"));
  setFaceWithMode("pushup", FACE_ANIM_ONCE);
  runStandPose(0); 
  delayWithFace(200);
  setServoAngle(L1, 0);
  setServoAngle(R1, 180);
  setServoAngle(L3, 90);
  setServoAngle(R3, 90);
  delayWithFace(500);
  for (int i = 0; i < 4; i++) {
    setServoAngle(L3, 0);
    setServoAngle(R3, 180);
    delayWithFace(600);
    setServoAngle(L3, 90);
    setServoAngle(R3, 90);
    delayWithFace(500);
  }
  runStandPose(1);
  if (currentCommand == CMD_PUSHUP) currentCommand = CMD_NONE;
}

inline void runBowPose() {
  Serial.println(F("BOW"));
  setFaceWithMode("bow", FACE_ANIM_ONCE);
  runStandPose(0); 
  delayWithFace(200);
  setServoAngle(L1, 0);
  setServoAngle(R1, 180);
  setServoAngle(L3, 0);
  setServoAngle(R3, 180);
  setServoAngle(L2, 180);
  setServoAngle(R2, 0);
  setServoAngle(R4, 0);
  setServoAngle(L4, 180);
  delayWithFace(600);
  setServoAngle(L3, 90);
  setServoAngle(R3, 90);
  delayWithFace(3000);
  runStandPose(1);
  if (currentCommand == CMD_BOW) currentCommand = CMD_NONE;
}

inline void runCutePose() {
  Serial.println(F("CUTE"));
  setFaceWithMode("cute", FACE_ANIM_ONCE);
  runStandPose(0); 
  delayWithFace(200);
  setServoAngle(L2, 160);
  setServoAngle(R2, 20);
  setServoAngle(R4, 180);
  setServoAngle(L4, 0);

  setServoAngle(L1, 0);
  setServoAngle(R1, 180);
  setServoAngle(L3, 180);
  setServoAngle(R3, 0);
  delayWithFace(200);
  for (int i = 0; i < 5; i++) {
    setServoAngle(R4, 180);
    setServoAngle(L4, 45);
    delayWithFace(300);
    setServoAngle(R4, 135);
    setServoAngle(L4, 0);
    delayWithFace(300);
  }
  runStandPose(1);
  if (currentCommand == CMD_CUTE) currentCommand = CMD_NONE;
}

inline void runFreakyPose() {
  Serial.println(F("FREAKY"));
  setFaceWithMode("freaky", FACE_ANIM_ONCE);
  runStandPose(0); 
  delayWithFace(200);
  setServoAngle(L1, 0);
  setServoAngle(R1, 180);
  setServoAngle(L2, 180);
  setServoAngle(R2, 0);
  setServoAngle(R4, 90);
  setServoAngle(R3, 0);
  delayWithFace(200);
  for (int i = 0; i < 3; i++) {
    setServoAngle(R3, 25);
    delayWithFace(400);
    setServoAngle(R3, 0);
    delayWithFace(400);
  }
  runStandPose(1);
  if (currentCommand == CMD_FREAKY) currentCommand = CMD_NONE;
}

inline void runWormPose() {
  Serial.println(F("WORM"));
  setFaceWithMode("worm", FACE_ANIM_ONCE);
  runStandPose(0);
  delayWithFace(200);
  setServoAngle(R1, 180); setServoAngle(R2, 0); setServoAngle(L1, 0); setServoAngle(L2, 180);
  setServoAngle(R4, 90); setServoAngle(R3, 90); setServoAngle(L3, 90); setServoAngle(L4, 90);
  delayWithFace(200);
  for(int i=0; i<5; i++) {
    setServoAngle(R3, 45); setServoAngle(L3, 135); setServoAngle(R4, 45); setServoAngle(L4, 135);
    delayWithFace(300);
    setServoAngle(R3, 135); setServoAngle(L3, 45); setServoAngle(R4, 135); setServoAngle(L4, 45);
    delayWithFace(300);
  }
  runStandPose(1);
  if (currentCommand == CMD_WORM) currentCommand = CMD_NONE;
}

inline void runShakePose() {
  Serial.println(F("SHAKE"));
  setFaceWithMode("shake", FACE_ANIM_ONCE);
  runStandPose(0);
  delayWithFace(200);
  setServoAngle(R1, 135); setServoAngle(L1, 45); setServoAngle(L3, 90); setServoAngle(R3, 90);
  setServoAngle(L2, 90); setServoAngle(R2, 90);
  delayWithFace(200);
  for(int i=0; i<5; i++) {
    setServoAngle(R4, 45); setServoAngle(L4, 135);
    delayWithFace(300);
    setServoAngle(R4, 0); setServoAngle(L4, 180);
    delayWithFace(300);
  }
  runStandPose(1);
  if (currentCommand == CMD_SHAKE) currentCommand = CMD_NONE;
}

inline void runShrugPose() {
  Serial.println(F("SHRUG"));
  runStandPose(0);
  setFaceWithMode("dead", FACE_ANIM_ONCE);
  delayWithFace(200);
  setServoAngle(R3, 90); setServoAngle(R4, 90); setServoAngle(L3, 90); setServoAngle(L4, 90);
  delayWithFace(1000);
  setFaceWithMode("shrug", FACE_ANIM_ONCE);
  setServoAngle(R3, 0); setServoAngle(R4, 180); setServoAngle(L3, 180); setServoAngle(L4, 0);
  delayWithFace(1500);
  runStandPose(1);
  if (currentCommand == CMD_SHRUG) currentCommand = CMD_NONE;
}

inline void runDeadPose() {
  Serial.println(F("DEAD"));
  runStandPose(0);
  setFaceWithMode("dead", FACE_ANIM_BOOMERANG);
  delayWithFace(200);
  setServoAngle(R3, 90); setServoAngle(R4, 90); setServoAngle(L3, 90); setServoAngle(L4, 90);
  if (currentCommand == CMD_DEAD) currentCommand = CMD_NONE;
}

inline void runCrabPose() {
  Serial.println(F("CRAB"));
  setFaceWithMode("crab", FACE_ANIM_ONCE);
  runStandPose(0);
  delayWithFace(200);
  setServoAngle(R1, 90); setServoAngle(R2, 90); setServoAngle(L1, 90); setServoAngle(L2, 90);
  setServoAngle(R4, 0); setServoAngle(R3, 180); setServoAngle(L3, 45); setServoAngle(L4, 135);
  for(int i=0; i<5; i++) {
    setServoAngle(R4, 45); setServoAngle(R3, 135); setServoAngle(L3, 0); setServoAngle(L4, 180);
    delayWithFace(300);
    setServoAngle(R4, 0); setServoAngle(R3, 180); setServoAngle(L3, 45); setServoAngle(L4, 135);
    delayWithFace(300);
  }
  runStandPose(1);
  if (currentCommand == CMD_CRAB) currentCommand = CMD_NONE;
}

// --- MOVEMENT ANIMATIONS ---
inline void runWalkPose() {
  Serial.println(F("WALK FWD"));
  setFaceWithMode("walk", FACE_ANIM_ONCE);
  // Initial Step
  setServoAngle(R3, 135); setServoAngle(L3, 45);
  setServoAngle(R2, 100); setServoAngle(L1, 25);
  if (!pressingCheck(CMD_FORWARD, frameDelay)) return;
  
  for (int i = 0; i < walkCycles; i++) {
    setServoAngle(R3, 135); setServoAngle(L3, 0);
    if (!pressingCheck(CMD_FORWARD, frameDelay)) return;
    setServoAngle(L4, 135); setServoAngle(L2, 90);
    setServoAngle(R4, 0); setServoAngle(R1, 180);
    if (!pressingCheck(CMD_FORWARD, frameDelay)) return;    
    setServoAngle(R2, 45); setServoAngle(L1, 90);
    if (!pressingCheck(CMD_FORWARD, frameDelay)) return;
    setServoAngle(R4, 45); setServoAngle(L4, 180);
    if (!pressingCheck(CMD_FORWARD, frameDelay)) return;
    setServoAngle(R3, 180); setServoAngle(L3, 45);
    setServoAngle(R2, 90); setServoAngle(L1, 0);
    if (!pressingCheck(CMD_FORWARD, frameDelay)) return;  
    setServoAngle(L2, 135); setServoAngle(R1, 90);
    if (!pressingCheck(CMD_FORWARD, frameDelay)) return;
  }
  runStandPose(1);
}

// Logic reversed from Walk
inline void runWalkBackward() {
  Serial.println(F("WALK BACK"));
  setFaceWithMode("walk", FACE_ANIM_ONCE);
  if (!pressingCheck(CMD_BACKWARD, frameDelay)) return;
  
  for (int i = 0; i < walkCycles; i++) {
    setServoAngle(R3, 135); setServoAngle(L3, 0);
    if (!pressingCheck(CMD_BACKWARD, frameDelay)) return;
    setServoAngle(L4, 135); setServoAngle(L2, 135);
    setServoAngle(R4, 0); setServoAngle(R1, 90);
    if (!pressingCheck(CMD_BACKWARD, frameDelay)) return;    
    setServoAngle(R2, 90); setServoAngle(L1, 0);
    if (!pressingCheck(CMD_BACKWARD, frameDelay)) return;
    setServoAngle(R4, 45); setServoAngle(L4, 180);
    if (!pressingCheck(CMD_BACKWARD, frameDelay)) return;
    setServoAngle(R3, 180); setServoAngle(L3, 45);
    setServoAngle(R2, 45); setServoAngle(L1, 90);
    if (!pressingCheck(CMD_BACKWARD, frameDelay)) return;  
    setServoAngle(L2, 90); setServoAngle(R1, 180);
    if (!pressingCheck(CMD_BACKWARD, frameDelay)) return;
  }
  runStandPose(1);
}

// Simple turn logic
inline void runTurnLeft() {
  Serial.println(F("TURN LEFT"));
  setFaceWithMode("walk", FACE_ANIM_ONCE);
  for (int i = 0; i < walkCycles; i++) {
    //legset 1 (R1 L2)
    setServoAngle(R3, 135); setServoAngle(L4, 135); 
    if (!pressingCheck(CMD_LEFT, frameDelay)) return;
    setServoAngle(R1, 180); setServoAngle(L2, 180); 
    if (!pressingCheck(CMD_LEFT, frameDelay)) return;
    setServoAngle(R3, 180); setServoAngle(L4, 180); 
    if (!pressingCheck(CMD_LEFT, frameDelay)) return;
    setServoAngle(R1, 135); setServoAngle(L2, 135);
    if (!pressingCheck(CMD_LEFT, frameDelay)) return;
      //legset 2 (R2 L1)
    setServoAngle(R4, 45); setServoAngle(L3, 45); 
    if (!pressingCheck(CMD_LEFT, frameDelay)) return;
    setServoAngle(R2, 90); setServoAngle(L1, 90); 
    if (!pressingCheck(CMD_LEFT, frameDelay)) return;
    setServoAngle(R4, 0); setServoAngle(L3, 0); 
    if (!pressingCheck(CMD_LEFT, frameDelay)) return;
    setServoAngle(R2, 45); setServoAngle(L1, 45);
    if (!pressingCheck(CMD_LEFT, frameDelay)) return;  
  }
  runStandPose(1);
}

inline void runTurnRight() {
  Serial.println(F("TURN RIGHT"));
  setFaceWithMode("walk", FACE_ANIM_ONCE);
  for (int i = 0; i < walkCycles; i++) {
    //legset 2 (R2 L1)
    setServoAngle(R4, 45); setServoAngle(L3, 45); 
    if (!pressingCheck(CMD_RIGHT, frameDelay)) return;
    setServoAngle(R2, 0); setServoAngle(L1, 0); 
    if (!pressingCheck(CMD_RIGHT, frameDelay)) return;
    setServoAngle(R4, 0); setServoAngle(L3, 0); 
    if (!pressingCheck(CMD_RIGHT, frameDelay)) return;
    setServoAngle(R2, 45); setServoAngle(L1, 45);
    if (!pressingCheck(CMD_RIGHT, frameDelay)) return;  
    //legset 1 (R1 L2)
    setServoAngle(R3, 135); setServoAngle(L4, 135); 
    if (!pressingCheck(CMD_RIGHT, frameDelay)) return;
    setServoAngle(R1, 90); setServoAngle(L2, 90); 
    if (!pressingCheck(CMD_RIGHT, frameDelay)) return;
    setServoAngle(R3, 180); setServoAngle(L4, 180); 
    if (!pressingCheck(CMD_RIGHT, frameDelay)) return;
    setServoAngle(R1, 135); setServoAngle(L2, 135);
    if (!pressingCheck(CMD_RIGHT, frameDelay)) return;
  }
  runStandPose(1);
}
