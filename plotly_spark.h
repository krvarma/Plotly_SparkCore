#ifndef plotly_streaming_spark_h
#define plotly_streaming_spark_h

#include "application.h"

class plotly
{
    public:
        plotly(char *username, char *api_key, char* stream_tokens[], char *filename, int nTraces);
        TCPClient client;
        bool init();
        void openStream();
        void closeStream();
        void reconnectStream();
        void jsonStart(int i);
        void jsonMiddle();
        void jsonEnd(char *token);
        char* dtostrf (double val, signed char width, unsigned char prec, char *sout);

        void plot(unsigned long x, int y, char *token);
        void plot(unsigned long x, float y, char *token);

        int log_level;
        bool dry_run;
        int maxpoints;
        bool world_readable;
        bool convertTimestamp;
        char *timezone;
        char *fileopt;

    private:
        void print_(int d);
        void print_(unsigned long d);
        void print_(float d);
        void print_(char *d);
        //void print_(const __FlashStringHelper* d);

        int len_(int i);
        int len_(unsigned long i);
        int len_(char *i);

        unsigned long fibonacci_;
        char *username_;
        char *api_key_;
        char** stream_tokens_;
        char *filename_;
        int nTraces_;

};
#endif