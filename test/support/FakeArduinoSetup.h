#pragma once

namespace npb::test
{
    void resetArduinoFakes();
    void installDefaultArduinoFakes();

    inline void resetAndInstallDefaultArduinoFakes()
    {
        resetArduinoFakes();
        installDefaultArduinoFakes();
    }
}
