#include "plotly_spark.h"


plotly::plotly(char *username, char *api_key, char* stream_tokens[], char *filename, int nTraces)
  {
    log_level = 2;  // 0 = Debugging, 1 = Informational, 2 = Status, 3 = Errors, 4 = Quiet (// Serial Off)
    dry_run = false;
    username_ = username;
    api_key_ = api_key;
    stream_tokens_ = stream_tokens;
    filename_ = filename;
    nTraces_ = nTraces;
    maxpoints = 30;
    fibonacci_ = 1;
    world_readable = true;
    convertTimestamp = true;
    timezone = "America/Montreal";
    fileopt = "overwrite";
}

bool plotly::init(){
    //
    //  Validate a stream with a REST post to plotly
    //
    if(dry_run && log_level < 3){
        Serial.println(F("... This is a dry run, we are not connecting to plotly's servers..."));
    }
    else if(log_level < 3) {
        Serial.println(F("... Attempting to connect to plotly's REST servers"));
    }
    while ( !client.connect("plot.ly", 80) ) {
        if(log_level < 4){
            Serial.println(F("... Couldn\'t connect to plotly's REST servers... trying again!"));
        }
        fibonacci_ += fibonacci_;
        delay(min(fibonacci_, 60000));
    }
    fibonacci_ = 1;
    if(log_level < 3){} Serial.println(F("... Connected to plotly's REST servers"));
    if(log_level < 3){} Serial.println(F("... Sending HTTP Post to plotly"));
    print_(F("POST /clientresp HTTP/1.1\r\n"));
    print_(F("Host: 107.21.214.199\r\n"));
    print_(F("User-Agent: Arduino/0.5.1\r\n"));

    print_(F("Content-Length: "));
    int contentLength = 126 + len_(username_) + len_(fileopt) + nTraces_*(87+len_(maxpoints)) + (nTraces_-1)*2 + len_(filename_);
    if(world_readable){
        contentLength += 4;
    } else{
        contentLength += 5;
    }
    print_(contentLength);
    // contentLength =
    //   44  // first part of querystring below
    // + len_(username)  // upper bound on username length
    // + 5   // &key=
    // + 10  // api_key length
    // + 7  // &args=[...
    // + nTraces*(87+len(maxpoints)) // len({\"y\": [], \"x\": [], \"type\": \"scatter\", \"stream\": {\"token\": \") + 10 + len(\", "maxpoints": )+len(maxpoints)+len(}})
    // + (nTraces-1)*2 // ", " in between trace objects
    // + 22  // ]&kwargs={\"fileopt\": \"
    // + len_(fileopt)
    // + 16  // \", \"filename\": \"
    // + len_(filename)
    // + 21 // ", "world_readable":
    // + 4 if world_readable, 5 otherwise
    // + 1   // closing }
    //------
    // 126 + len_(username) + len_(fileopt) + nTraces*(86+len(maxpoints)) + (nTraces-1)*2 + len_(filename)
    //
    // Terminate headers with new lines
    print_(F("\r\n\r\n"));

    // Start printing querystring body
    print_(F("version=2.3&origin=plot&platform=arduino&un="));
    print_(username_);
    print_(F("&key="));
    print_(api_key_);
    print_(F("&args=["));
    // print a trace for each token supplied
    for(int i=0; i<nTraces_; i++){
        print_(F("{\"y\": [], \"x\": [], \"type\": \"scatter\", \"stream\": {\"token\": \""));
        print_(stream_tokens_[i]);
        print_(F("\", \"maxpoints\": "));
        print_(maxpoints);
        print_(F("}}"));
        if(nTraces_ > 1 && i != nTraces_-1){
            print_(F(", "));
        }
    }
    print_(F("]&kwargs={\"fileopt\": \""));
    print_(fileopt);
    print_(F("\", \"filename\": \""));
    print_(filename_);
    print_(F("\", \"world_readable\": "));
    if(world_readable){
        print_("true");
    } else{
        print_("false");
    }
    print_(F("}"));
    // final newline to terminate the POST
    print_(F("\r\n"));

    //
    // Wait for a response
    // Parse the response for the "All Streams Go!" and proceed to streaming
    // if we find it
    //
    char allStreamsGo[] = "All Streams Go!";
    char error[] = "\"error\": \"";
    int asgCnt = 0; // asg stands for All Streams Go
    char url[] = "\"url\": \"http://107.21.214.199/~";
    char fid[4];
    int fidCnt = 0;
    int urlCnt = 0;
    int usernameCnt = 0;
    int urlLower = 0;
    int urlUpper = 0;
    bool proceed = false;
    bool fidMatched = false;

    if(log_level < 2){
        Serial.println(F("... Sent message, waiting for plotly's response..."));
    }

    if(!dry_run){
        while(client.connected()){
            if(client.available()){
                char c = client.read();
                if(log_level < 2) Serial.print(c);

                //
                // Attempt to read the "All streams go" msg if it exists
                // by comparing characters as they roll in
                //

                if(asgCnt == len_(allStreamsGo) && !proceed){
                    proceed = true;
                }
                else if(allStreamsGo[asgCnt]==c){
                    asgCnt += 1;
                } else if(asgCnt > 0){
                    // reset counter
                    asgCnt = 0;
                }

                //
                // Extract the last bit of the URL from the response
                // The url is in the form http://107.21.214.199/~USERNAME/FID
                // We'll character-count up through char url[] and through username_, then start
                // filling in characters into fid
                //

                if(log_level < 3){
                    if(url[urlCnt]==c && urlCnt < len_(url)){
                        urlCnt += 1;
                    } else if(urlCnt > 0 && urlCnt < len_(url)){
                        // Reset counter
                        urlCnt = 0;
                    }
                    if(urlCnt == len_(url) && fidCnt < 4 && !fidMatched){
                        // We've counted through the url, start counting through the username
                        if(usernameCnt < len_(username_)+2){
                            usernameCnt += 1;
                        } else {
                            // the url ends with "
                            if(c != '"'){
                                fid[fidCnt] = c;
                                fidCnt += 1;
                            } else if(fidCnt>0){
                                fidMatched = true;
                            }

                        }
                    }
                }
            }
        }
        client.stop();
    }

    if(!dry_run && !proceed && log_level < 4){
        Serial.println(F("... Error initializing stream, aborting. Try again or get in touch with Chris at chris@plot.ly"));
    }

    if(!dry_run && proceed && log_level < 3){
        Serial.println(F("... A-ok from plotly, All Streams Go!"));
        if(fidMatched){
            Serial.print(F("... View your streaming plot here: https://plot.ly/~"));
            Serial.print(username_);
            Serial.print(F("/"));
            for(int i=0; i<fidCnt; i++){
                Serial.print(fid[i]);
            }
            Serial.println(F(""));
        }
    }
    return proceed;
}
void plotly::openStream() {
    //
    // Start request to stream servers
    //
    if(log_level < 3){} Serial.println(F("... Connecting to plotly's streaming servers..."));
    char server[] = "arduino.plot.ly";
    int port = 80;
    while ( !client.connect(server, port) ) {
        if(log_level < 4) Serial.println(F("... Couldn\'t connect to servers... trying again!"));
        fibonacci_ += fibonacci_;
        delay(min(fibonacci_, 60000));
    }
    fibonacci_ = 1;
    if(log_level < 3){} Serial.println(F("... Connected to plotly's streaming servers\n... Initializing stream"));

    print_(F("POST / HTTP/1.1\r\n"));
    print_(F("Host: arduino.plot.ly\r\n"));
    print_(F("User-Agent: Python\r\n"));
    print_(F("Transfer-Encoding: chunked\r\n"));
    print_(F("Connection: close\r\n"));
    if(convertTimestamp){
        print_(F("plotly-convertTimestamp: \""));
        print_(timezone);
        print_(F("\"\r\n"));
    }
    print_(F("\r\n"));

    if(log_level < 3){} Serial.println(F("... Done initializing, ready to stream!"));
}

void plotly::closeStream(){
    print_(F("0\r\n\r\n"));
    client.stop();
}
void plotly::reconnectStream(){
    while(!client.connected()){
        if(log_level<4) Serial.println(F("... Disconnected from streaming servers"));
        closeStream();
        openStream();
    }
}
void plotly::jsonStart(int i){
    // Print the length of the message in hex:
    // 15 char for the json that wraps the data: {"x": , "y": }\n
    // + 23 char for the token: , "token": "abcdefghij"
    // = 38
    if(log_level<2) Serial.print(i+44, HEX);
    if(!dry_run) client.print(i+44, HEX);
    print_("\r\n{\"x\": ");
}
void plotly::jsonMiddle(){
    print_(", \"y\": ");
}
void plotly::jsonEnd(char *token){
    print_(", \"streamtoken\": \"");
    print_(token);
    print_("\"}\n\r\n");
}

int plotly::len_(int i){
    // int range: -32,768 to 32,767
    if(i > 9999) return 5;
    else if(i > 999) return 4;
    else if(i > 99) return 3;
    else if(i > 9) return 2;
    else if(i > -1) return 1;
    else if(i > -10) return 2;
    else if(i > -100) return 3;
    else if(i > -1000) return 4;
    else if(i > -10000) return 5;
    else return 6;
}
int plotly::len_(unsigned long i){
    // max length of unsigned long: 4294967295
    if(i > 999999999) return 10;
    else if(i > 99999999) return 9;
    else if(i > 9999999) return 8;
    else if(i > 999999) return 7;
    else if(i > 99999) return 6;
    else if(i > 9999) return 5;
    else if(i > 999) return 4;
    else if(i > 99) return 3;
    else if(i > 9) return 2;
    else return 1;
}
int plotly::len_(char *i){
    return strlen(i);
}
void plotly::plot(unsigned long x, int y, char *token){
    reconnectStream();
    jsonStart(len_(x)+len_(y));
    print_(x);
    jsonMiddle();
    print_(y);
    jsonEnd(token);
}
void plotly::plot(unsigned long x, float y, char *token){
    reconnectStream();

    char s_[15];
    dtostrf(y,2,3,s_);

    jsonStart(len_(x)+len_(s_)-1);
    print_(x);
    jsonMiddle();
    print_(y);
    jsonEnd(token);
}
void plotly::print_(int d){
    if(log_level < 2) Serial.print(d);
    if(!dry_run) client.print(d);
}
void plotly::print_(unsigned long d){
    if(log_level < 2) Serial.print(d);
    if(!dry_run) client.print(d);
}
void plotly::print_(float d){
    if(log_level < 2) Serial.print(d);
    if(!dry_run) client.print(d);
}
void plotly::print_(char *d){
    if(log_level < 2) Serial.print(d);
    if(!dry_run) client.print(d);
}
/*
void plotly::print_(const __FlashStringHelper* d){
    if(log_level < 2) Serial.print(d);
    if(!dry_run) client.print(d);
}
*/

//convert double to ascii
char *plotly::dtostrf (double val, signed char width, unsigned char prec, char *sout) {
  char fmt[20];
  sprintf(fmt, "%%%d.%df", width, prec);
  sprintf(sout, fmt, val);
  return sout;
}

