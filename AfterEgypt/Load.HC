#help_index "Games"

Cd(__DIR__);;
#include "Comics"
#include "Camp"
PrintI("l",1);
Sleep(100);
#include "Clouds"
PrintI("l",2);
#include "Mountain"
PrintI("l",3);
#include "Map"
PrintI("l",4);
#include "Battle"
PrintI("l",5);
#include "WaterRock"
PrintI("l",6);
#include "Quail"
PrintI("l",7);
#include "HorebA"

#if mp_cnt>=4
  #include "HorebBMP"
#else
  #include "HorebBSP"
#endif

#include "HorebC"
#include "GodTalking"
#include "AfterEgypt"

#help_index ""
