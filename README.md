Spark Core and [Plotly][1]
--------------------------

Sample Application to stream data to [Plotly][2] using Spark Core. The application stream temperature value from DHT22 Sensor to Plotly. To use this application create a Plotly account, download the source and open it in Spark Editor and replace the tokens ***streamtoken***, ***username***, ***apikey*** and ***filename*** with corresponding values from Plotly settings.

This application uses Plotly SDK form [GitHub][3]. I took the ***plotly_streaming_wifi*** source and ported to Spark Core. It is only a simple and straightforward  port, only two modification done. 1. Copy the method ***dtostrf*** form [this location][4] and included in Plotly class and 2. commented the method **void print_(const __FlashStringHelper*d)** because this method was causing some compiler errors (I didn't dig deep into it, just commented the method).

**Screenshots**

![spark core wiring][5] 

![enter image description here][6]


  [1]: https://plot.ly
  [2]: https://plot.ly
  [3]: https://github.com/plotly/arduino-api
  [4]: https://github.com/spark/core-firmware/blob/master/src/spark_wiring_string.cpp
  [5]: https://raw.githubusercontent.com/krvarma/Plotly_SparkCore/master/IMG_0084.JPG
  [6]: https://raw.githubusercontent.com/krvarma/Plotly_SparkCore/master/IMG_0085.JPG