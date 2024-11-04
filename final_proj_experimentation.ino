// Load Wi-Fi library
#include <ESP8266WiFi.h>

// Replace with your network credentials
const char* ssid     = "moto g stylus 8643";
const char* password = "55d9257d5bfe";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output2State = "off";

// Assign output variables to GPIO pins
const int output5 = 5;
const int output2 = 2;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(output2, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output5, LOW);
  digitalWrite(output2, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
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

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            bool enteredMotorSpeedWasValid = true;
            int motorSpeedValue = 0;
            // Handle the form submission for setting motor speed
            if (header.indexOf("GET /setSpeed") >= 0) {
                // Extract the motor speed value from the URL
                int digitCount = 0;
            
                // Start parsing from the position after "GET /setSpeed"
                int startPos = header.indexOf("GET /setSpeed") + 25;
            
                // Iterate through the characters until a non-digit character is encountered
                while (isdigit(header[startPos])) {
                    motorSpeedValue = motorSpeedValue * 10 + (header[startPos] - '0');
                    digitCount++;
                    startPos++;
                }
                // Check if the entered value is valid (0 to 100)
                if (digitCount > 0 && digitCount <= 3 && motorSpeedValue >= 0 && motorSpeedValue <= 100) {
                    // Use the motorSpeedValue as needed (e.g., set motor speed)
                    int dutyValue = map(motorSpeedValue, 0, 100, 0, 255);
                    analogWrite(output2, dutyValue);
                } else {
                    enteredMotorSpeedWasValid = false;
                }
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>DC Motor Web Server</h1>");

            // Add a text box for user input
            client.println("<form method=\"get\" action=\"/setSpeed\">");
            client.println("<input type=\"text\" name=\"motorSpeed\" placeholder=\"Enter motor speed\">");
            client.println("<input type=\"submit\" value=\"Set Speed\">");
            client.println("</form>");
            
            if (enteredMotorSpeedWasValid) {
                    client.print("<p>Motor speed set to: ");
                    client.print(motorSpeedValue);
                    client.println("</p>");
            } else {
              client.println("<p>Invalid motor speed value. Please enter a number between 1 and 100.</p>");
            }
            
            client.println("</body></html>");


            // The HTTP response ends with another blank line
            client.println();                 
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
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
