/* SerrOne_Version.h */ // SVN
#pragma once

//class Version {
//  public:
namespace Version {

    const uint8_t   MAJOR     = 0;
    const uint8_t   MINOR     = 14;
    const uint16_t  REVISION  = 1004;
    const char*     STAGE     = "pre-alpha";
    String version;

    inline const char* toString() {
      version +=    String(MAJOR) + ".";
      version +=    String(MINOR) + ".";
      version += String(REVISION) + " ";
      version += STAGE;
      return version.c_str();
    }
}
//} Version;

/* End */
