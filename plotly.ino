// This #include statement was automatically added by the Spark IDE.
#include "plotly_spark.h"

// This #include statement was automatically added by the Spark IDE.
#include "dht.h"

#define DHTPIN D4
#define SOUNDPIN A0
#define DHTTYPE DHT22

#define NUM_TRACES 1

DHT dht(DHTPIN, DHTTYPE);
char szInfo[64];

char *streaming_tokens[NUM_TRACES] = {"<<streamtoken>>"};
plotly graph = plotly("<<username>>", "<<apikey>>", streaming_tokens, "<<filename>>", NUM_TRACES);

// Publush event
void Publish(char* szEventInfo){
    Spark.publish("plotlyinfo", szEventInfo);
}

void setup() {
    dht.begin();
    
    graph.init();
    graph.openStream(); 
}

void loop() {
    float t = dht.readTemperature();

    sprintf(szInfo, "Temperature=%.2f °C", t);
    
    graph.plot(millis(), t, streaming_tokens[0]);

    Publish(szInfo);
    
    delay(60000);
}