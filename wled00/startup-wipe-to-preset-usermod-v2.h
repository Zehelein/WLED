#include "wled.h"

/*
 * Usermod that runs the wipe animation on startup and then starts preset 1
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * Using this usermod:
 * 1. Copy the usermod into the sketch folder (same folder as wled00.ino)
 * 2. Register the usermod by adding `#include "startup-wipe-to-preset-usermod-v2.h"` in the top and `usermods.add(new StartupWipeToPresetUsermod());` in the bottom of usermods_list.cpp
 */

class StartupWipeToPresetUsermod : public Usermod
{
private:
  //Private class members.
  byte wipeState = 0; //0: inactive 1: wiping 2: preset started

  void getWipeColorFromPreset(byte index)
  {
    if (fileDoc)
    {
      errorFlag = readObjectFromFileUsingId("/presets.json", index, fileDoc) ? ERR_NONE : ERR_FS_PLOAD;
      JsonObject fdo = fileDoc->as<JsonObject>();
      getWipeColorFromPresetJson(fdo);
    }
    else
    {
      DynamicJsonDocument fDoc(JSON_BUFFER_SIZE);
      errorFlag = readObjectFromFileUsingId("/presets.json", index, &fDoc) ? ERR_NONE : ERR_FS_PLOAD;
      JsonObject fdo = fDoc.as<JsonObject>();
      getWipeColorFromPresetJson(fdo);
    }
  }

  void getWipeColorFromPresetJson(JsonObject root)
  {
    JsonVariant segVar = root["seg"];
    if (segVar.is<JsonObject>())
    {
      int id = segVar[F("id")] | -1;
      if (id >= 0)
      {
        getWipeColorFromPresetSeg(segVar);
      }
    }
    else
    {
      JsonArray segs = segVar.as<JsonArray>();
      JsonObject segItem = segs.getElement(0);
      getWipeColorFromPresetSeg(segItem);
    }
  }

  void getWipeColorFromPresetSeg(JsonObject elem)
  {
    JsonArray colarr = elem[F("col")];
    if (!colarr.isNull())
    {
      JsonArray colX = colarr[0];
      byte sz = colX.size();
      if (sz > 2)
      {
        col[0] = colX[0];
        col[1] = colX[1];
        col[2] = colX[2];
        //col[3] = rgbw[3];
      }
    }
  }

public:
  void loop()
  {
    if (wipeState == 0)
    {
      startWipe();
      wipeState = 1;
    }
    else if (wipeState == 1)
    {
      uint32_t cycleTime = 360 + (255 - effectSpeed) * 75; //this is how long one wipe takes (minus 25 ms to make sure we switch in time)
      if (millis() + strip.timebase > (cycleTime - 25))
      { //wipe complete
        wipeState = 2;
        applyPreset(1);
        colorUpdated(NOTIFIER_CALL_MODE_NOTIFICATION);
      }
    }
  }

  uint16_t getId()
  {
    return USERMOD_ID_STARTUP_WIPE;
  }

  void startWipe()
  {
    bri = briLast;           //turn on
    transitionDelayTemp = 0; //no transition
    getWipeColorFromPreset(99);
    effectCurrent = FX_MODE_COLOR_WIPE;
    resetTimebase(); //make sure wipe starts from beginning
    colorUpdated(NOTIFIER_CALL_MODE_NOTIFICATION);
  }
};
