#include "calibration.h"

int main( int argc, char** argv )
{
    const string inputSettingsFile = argc > 1 ? argv[1] : "settings.yml";
    runFromSettings(inputSettingsFile);
}
