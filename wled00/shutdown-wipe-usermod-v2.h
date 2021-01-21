#include "wled.h"

/*
 * Usermod that runs the wipe animation on shutdown
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * 
 * Using this usermod:
 * 1. Copy the usermod into the sketch folder (same folder as wled00.ino)
 * 2. Register the usermod by adding `#include "shutdown-wipe-usermod-v2.h"` in the top and `usermods.add(new ShutdownWipeUsermod());` in the bottom of usermods_list.cpp
 * 3. Adjust the `json.cpp` file to check if power actually switched and set there `userVar0 = 2;`
 */

class ShutdownWipeUsermod : public Usermod
{
private:
  //Private class members.
  byte wipeState = 0; //0: inactive 1: wiping 2: shutdown finished

public:
  void loop()
  {
    // trigger via API or from other code
    if (userVar0 == 2)
    {
      startWipe();
      wipeState = 1;
      userVar0 = 0;
    }
    else if (wipeState == 1)
    {
      uint32_t cycleTime = 360 + (255 - effectSpeed) * 75; //this is how long one wipe takes (minus 5 ms to make sure we switch in time)
      if (millis() + strip.timebase > (cycleTime - 5))
      { //wipe complete
        wipeState = 2;
        toggleOnOff();
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
    bri = briLast; //turn on
    transitionDelayTemp = 10;
    colorFromUint32(0x000000);
    effectCurrent = FX_MODE_COLOR_WIPE;
    effectSpeed = 222;
    resetTimebase(); //make sure wipe starts from beginning

    //set wipe direction backwards
    WS2812FX::Segment &seg = strip.getSegment(0);
    seg.setOption(1, true);

    colorUpdated(NOTIFIER_CALL_MODE_NOTIFICATION);
  }
};
