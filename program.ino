#include <ESP8266WiFi.h>
#include <Servo.h>
// create servo object to control a servo
// twelve servo objects can be created on most boards
Servo myservo;

// GPIO the servo is attached to
static const int servoPin = D4;

// Replace with your network credentials
const char *ssid = "Wi-Fi_SSID";
const char *password = "password";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Decode HTTP GET value
String valueString = String(5);
int pos1 = 0;
int pos2 = 0;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup()
{
    Serial.begin(115200);
    // attaches the servo on the servoPin to the servo object
    myservo.attach(servoPin);

    // Connect to Wi-Fi network with SSID and password
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
}

void loop()
{
    WiFiClient client = server.available(); // Listen for incoming clients

    if (client)
    {
        // If a new client connects,
        currentTime = millis();
        previousTime = currentTime;
        Serial.println("New Client."); // print a message out in the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected() && currentTime - previousTime <= timeoutTime)
        {
            // loop while the client's connected
            currentTime = millis();
            if (client.available())
            {
                // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                header += c;
                if (c == '\n')
                {
                    // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();

                        // Display the HTML web page
                        // sends the head of the html document along with the css and javascipt
                        client.println("<!DOCTYPE html><html lang='en'><head> <meta charset='UTF-8'>");
                        client.println("<meta http-equiv='X-UA-Compatible' content='IE=edge'> <meta name='viewport' content='width=device-width, initial-scale=1.0'> <title>Servo Contol</title>");
                        client.println("<script src='https://cdn.jsdelivr.net/npm/jquery@3.6.0/dist/jquery.min.js' integrity='sha256-/xUj+3OJU5yExlq6GSYGSHk7tPXikynS7ogEvDej/m4=' crossorigin='anonymous'></script>");
                        client.println("<link href='https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css' rel='stylesheet' integrity='sha384-EVSTQN3/azprG1Anm3QDgpJLIm9Nao0Yz1ztcQTwFspd3yD65VohhpuuCOmLASjC' crossorigin='anonymous'>");
                        client.println("<script src='https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/js/bootstrap.bundle.min.js' integrity='sha384-MrcW6ZMFYlzcLA8Nl+NtUVF0sA7MsXsP1UyJoMp4YLEuNSfAP+JcXn/tWtIaxVXM' crossorigin='anonymous'></script></head>");
                        // sends the body of the html document
                        client.println("<body> <div class='container-fluid m-0 p-0'> <nav class='navbar navbar-dark bg-primary'>");
                        client.println("<div class='container-fluid'> <span class='navbar-brand mb-0 h1'>Servo Motor Web</span> </div></nav>");
                        client.println("<div class='row' style='height:50vh; max-width:100vw;'> <div class='col-4 align-self-center'>");
                        client.println("<div class='card text-center'> <div class='card-body w-100' style='height:20vh'>");
                        client.println("<div class='card-text'> This page allows us to change the position of servo motor by using the slider on the right. </div></div></div></div>");
                        client.println("<div class='col-4 align-self-center'> <h3>Position: <span id='servoPos'></span></h3> <div class='slidecontainer'>");
                        client.println("<div class='container'> <div class='row'> <div class='col'> <input type='range' min='0' max='180' class='slider w-100' id='servoSlider' onchange='servo(this.value)' value='' + valueString + ''/> </div></div>");
                        client.println("<div class='row'> <div class='col'>0</div><div class='col text-center'>90</div><div class='col text-end'>180</div></div></div></div></div><div class='col-4 m-0 p-0'></div></div></div>");
                        // sends our custom java-script code written in j-query
                        client.println("<script type='text/javascript'> var slider=document.getElementById('servoSlider'); var servoP=document.getElementById('servoPos');");
                        client.println("servoP.innerHTML=slider.value; slider.oninput=function (){slider.value=this.value; servoP.innerHTML=this.value;}$.ajaxSetup({timeout: 1000}); function servo(pos){$.get('/?value=' + pos + '&');{Connection: close};}</script>");
                        client.println("<script>var slider = document.getElementById(\"servoSlider\");");
                        client.println("var servoP = document.getElementById(\"servoPos\"); servoP.innerHTML = slider.value;");
                        client.println("slider.oninput = function() { slider.value = this.value; servoP.innerHTML = this.value; }");
                        client.println("$.ajaxSetup({timeout:1000}); function servo(pos) { ");
                        client.println("$.get(\"/?value=\" + pos + \"&\"); {Connection: close};}</script>");

                        // uses http method GET to get the values from the webpage using j-Query
                        // GET /?value=180& HTTP/1.1
                        if (header.indexOf("GET /?value=") >= 0)
                        {
                            pos1 = header.indexOf('=');
                            pos2 = header.indexOf('&');
                            valueString = header.substring(pos1 + 1, pos2);

                            // Rotate the servo
                            myservo.write(valueString.toInt());
                            Serial.println(valueString);
                        }

                        // The HTTP response ends with another blank line
                        client.println();
                        // Break out of the while loop
                        break;
                    }
                    else
                    {
                        // if you got a newline, then clear currentLine
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {
                    // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }
            }
        }

        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println("");
    }
}