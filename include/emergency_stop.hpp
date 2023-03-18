
#include <wiringPi.h>
#include <sys/time.h> // gettimeofday
#include <unistd.h>   // sleep/usleep

#include <iostream>

using namespace std;

class e_stop
{
public:
    struct timeval tv;

    const int TRIG = 15;
    const int ECHO = 1;

    const int coefficient = 17150;

    void initSensor()
    {
        wiringPiSetup();

        pinMode(TRIG, OUTPUT);
        pinMode(ECHO, INPUT);
    }

    double detectDistance()
    {
        digitalWrite(TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG, LOW);
        // usleep(10);
        // digitalWrite(TRIG, LOW);

        if (waitValue(0))
        {
            double pulseStart = getTime();

            // std::cout << "pulseStart" << pulseStart << "\n";

            if (waitValue(1))
            {
                double pulseEnd = getTime();

                double duration = pulseEnd - pulseStart;
                double distance = duration * coefficient;
                // std::cout << "duration" << duration << "\n";

                return distance;
            }
        }

        // std::cout << "Measurement error!"
        //           << "\n";

        return 0.0 / 0.0;
    }

private:
    double getTime()
    {
        gettimeofday(&tv, NULL);

        return (double)tv.tv_sec + (double)tv.tv_usec * 0.000001;
    }

    bool waitValue(int value, int limit = 10000)
    {
        for (int i = 0; digitalRead(ECHO) == value; ++i)
        {
            if (i >= limit)
                return false;
        }
        return true;
    }
};