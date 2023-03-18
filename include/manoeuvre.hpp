#include <iostream>
#include <wiringPi.h>

class moving
{
public:
    void wiringPi()
    {
        wiringPiSetup();
        pinMode(21, OUTPUT);
        pinMode(22, OUTPUT);
        pinMode(23, OUTPUT);
        pinMode(24, OUTPUT);
    }

    void forward(int Result)
    {
        if (Result == 0)
        {
            digitalWrite(21, 0);
            digitalWrite(22, 0); // Decimal = 0
            digitalWrite(23, 0);
            digitalWrite(24, 0);

            std::cout << "Forward"
                      << "\n";
        }
        else if (Result > 0 && Result < 5)
        {
            digitalWrite(21, 1);
            digitalWrite(22, 0); // Decimal = 1
            digitalWrite(23, 0);
            digitalWrite(24, 0);

            std::cout << "Right1"
                 << "\n";
        }
        else if (Result >= 5 && Result < 10)
        {
            digitalWrite(21, 0);
            digitalWrite(22, 1); // Decimal = 2
            digitalWrite(23, 0);
            digitalWrite(24, 0);

            std::cout << "Right2"
                      << "\n";
        }
        else if (Result >= 10)
        {
            digitalWrite(21, 1);
            digitalWrite(22, 1); // Decimal = 3
            digitalWrite(23, 0);
            digitalWrite(24, 0);

            std::cout << "Right3"
                      << "\n";
        }
        else if (Result < 0 && Result > -10)
        {
            digitalWrite(21, 0);
            digitalWrite(22, 0); // Decimal = 4
            digitalWrite(23, 1);
            digitalWrite(24, 0);

            std::cout << "Left1"
                      << "\n";
        }
        else if (Result <= -10 && Result > -20)
        {
            digitalWrite(21, 1);
            digitalWrite(22, 0); // Decimal = 5
            digitalWrite(23, 1);
            digitalWrite(24, 0);

            std::cout << "Left2"
                      << "\n";
        }
        else if (Result < -20)
        {
            digitalWrite(21, 0);
            digitalWrite(22, 1); // Decimal = 6
            digitalWrite(23, 1);
            digitalWrite(24, 0);

            std::cout << "Left3"
                      << "\n";
        }
    }

    void noLane()
    {
        std::cout << "Lane End" << "\n";
        
        digitalWrite(21, 1);
        digitalWrite(22, 1); // Decimal = 7
        digitalWrite(23, 1);
        digitalWrite(24, 0);
    }

    void stop()
    {
        std::cout << "stop"
                  << "\n";

        digitalWrite(21, 0);
        digitalWrite(22, 0); // Decimal = 8
        digitalWrite(23, 0);
        digitalWrite(24, 1);
    }
};