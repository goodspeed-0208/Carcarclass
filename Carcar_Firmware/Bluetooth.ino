
void initBlueTooth() {
  Serial.begin(115200);  // Debug Monitor (USB)
  while (!Serial)
    ;
  Serial.println("Initializing HM-10...");

  // 1. Automatic Baud Rate Detection
  for (int i = 0; i < 9; i++) {
    Serial.print("Testing baud rate: ");
    Serial.println(baudRates[i]);

    Serial3.begin(baudRates[i]);
    Serial3.setTimeout(100);

    // 2. Force Disconnection
    // Sending "AT" while connected forces the module to disconnect [2].
    Serial3.print("AT");

    if (waitForResponse("OK", 800)) {
      Serial.println("HM-10 detected and ready.");
      blueToothModuleReady = true;
      break;
    } else {
      Serial3.end();
    }
  }

  if (!blueToothModuleReady) {
    Serial.println("Failed to detect HM-10. Check 3.3V VCC and wiring.");
    return;
  }

  // 3. Restore Factory Defaults
  Serial.println("Restoring factory defaults...");
  sendATCommand("AT+RENEW");  // Restores all setup values

  // 4. Set Custom Name via Macro
  Serial.print("Setting name to: ");
  Serial.println(CUSTOM_NAME);
  String nameCmd = "AT+NAME" + String(CUSTOM_NAME);
  sendATCommand(nameCmd.c_str());  // Max length is 12

  // 5. Enable Connection Notifications
  Serial.println("Enabling notifications...");
  sendATCommand("AT+NOTI1");  // Notify when link is established/lost

  // 6. Get the Bluetooth MAC Address
  Serial.println("Querying Bluetooth Address");
  sendATCommand("AT+ADDR?");

  // 7. Restart the module to apply changes
  Serial.println("Restarting module...");
  sendATCommand("AT+RESET");  // Restart the module
  Serial3.begin(9600);        // Now the module would use baudrate 9600

  Serial.println("Initialization Complete.");
}

void sendATCommand(const char* command) {
  Serial3.print(command);
  waitForResponse("", 1000);
}

/**
 * Helper to check response for specific substrings
 */
bool waitForResponse(const char* expected, unsigned long timeout) {
  unsigned long start = millis();
  String response = "";

  // Keep checking until the timeout is reached
  while (millis() - start < timeout) {
    // Read available characters from the serial buffer immediately
    while (Serial3.available()) {
      char c = Serial3.read();
      response += c;
    }

    // If we are looking for a specific string (like "OK")
    if (strlen(expected) > 0) {
      // Check if the expected string is already inside the response
      if (response.indexOf(expected) != -1) {
        Serial.print("HM10 Response: ");
        Serial.println(response);
        return true; // Early exit! Saves massive amount of time.
      }
    }
    
    // Small delay to prevent tight loop from blocking the CPU completely
    delay(1); 
  }

  // If we reach here, it means we waited the full timeout
  if (response.length() > 0) {
    Serial.print("HM10 Response (Timeout or finished): ");
    Serial.println(response);
    
    // Final check just in case the string arrived at the very last millisecond
    if (strlen(expected) > 0) {
        return (response.indexOf(expected) != -1);
    }
  }
  
  // If expected was "" (empty), return true since we successfully waited
  return (strlen(expected) == 0); 
}

void processBluetoothCommand(String command) {
  command.trim();
  Serial.println(command);

  /*if (command.startsWith("L ") || command.startsWith("R ")) {
    char dir = command.charAt(0);
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);

    if (firstSpace != -1 && secondSpace != -1) {
      int outer = command.substring(firstSpace + 1, secondSpace).toInt();
      float ratio = command.substring(secondSpace + 1).toFloat();
      
      Serial.print("Command received -> Dir: "); Serial.print(dir);
      Serial.print(" | Outer: "); Serial.print(outer);
      Serial.print(" | Ratio: "); Serial.println(ratio);
      
      mycar.start_turn_test(dir, outer, ratio);
    }
    return; // Exit here so it doesn't trigger other commands
  }*/

  if(command.startsWith("Dir:")){
    int space = command.indexOf(':');
    String s_dir = command.substring(space + 1);
    // Check the direction string and assign the corresponding state
    if (s_dir == "f") {
      mycar.next_dir = FORWARD;
    } else if (s_dir == "l") {
      mycar.next_dir = LEFT;
    } else if (s_dir == "r") {
      mycar.next_dir = RIGHT;
    } else if (s_dir == "t") {
      mycar.next_dir = TURN_BACK;
    } else if (s_dir == "b") {
      mycar.next_dir = BACKWARD;
    } else if (s_dir == "lb") {
      mycar.next_dir = LEFT_AFTER_BACKWARD;
    } else if (s_dir == "rb") {
      mycar.next_dir = RIGHT_AFTER_BACKWARD;
    } else if(s_dir == "stop"){
      mycar.next_dir = STAY_STOP;
    } else {
      // Optional: Handle unexpected input to prevent undefined behavior
      Serial3.println("the command is wrong.");
    }
  }

  if (command == "receive init") {
    // sendTime is a global variable in the main tab
    Serial.print(millis() - sendTime);
    Serial3.print(millis() - sendTime);
  }
  else if (command == "e") {
    mycar.stop();
  }
  else if (command == "s") {
    mycar.restart();
  }
  else if (command.indexOf(' ') != -1) {
    mycar.adjust_start = 0;
    int spaceIndex = command.indexOf(' ');

    // Extract and convert to integers
    mycar.forwardspeed = command.substring(0, spaceIndex).toInt();
    mycar.motor_error = command.substring(spaceIndex + 1).toInt();

    // Call restart to set isRunning = 1 and reset parameters
    mycar.restart();

    // Print to USB Serial monitor for debugging
    Serial.print("Calibration started. Speed: ");
    Serial.print(mycar.forwardspeed);
    Serial.print(" | Error: ");
    Serial.println(mycar.motor_error);

    // Send to Bluetooth terminal via Serial3
    Serial3.print("Calibration started. Speed: ");
    Serial3.print(mycar.forwardspeed);
    Serial3.print(" | Error: ");
    Serial3.println(mycar.motor_error);
  }
}