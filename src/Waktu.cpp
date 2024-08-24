class Waktu
{
public:
    char date[11]; // yyyy-mm-dd
    char time[9];  // hh:mm:ss
    char hour[3];  // hh
    int getHour()
    {
        return atoi(hour);
    }
    String fullDateTime()
    {
        return String(date) + " " + time;
    }
};