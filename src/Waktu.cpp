class Waktu
{
public:
    char date[11]; // yyyy-mm-dd
    char time[6];  // hh:mm
    char hour[3];  // hh
    int getHour()
    {
        return atoi(hour);
    }
};