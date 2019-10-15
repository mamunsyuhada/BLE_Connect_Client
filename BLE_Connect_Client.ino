// Service 0000fee0-0000-1000-8000-00805f9b34fb
// Characteristic 0000fee1-0000-1000-8000-00805f9b34fb
// Connect 78:04:73:be:5f:e1

#include <BLEDevice.h> //Header file for BLE 
static BLEUUID serviceUUID("22222221-B5A3-F393-E0A9-E50E24DCCA9E"); //Service UUID of fitnessband obtained through nRF connect application
static BLEUUID    rxCharUUID("22222222-B5A3-F393-E0A9-E50E24DCCA9E"); //Characteristic  UUID of fitnessband obtained through nRF connect application
static BLEUUID    txCharUUID("22222223-B5A3-F393-E0A9-E50E24DCCA9E"); //Characteristic  UUID of fitnessband obtained through nRF connect application
static BLERemoteCharacteristic* pRemoteCharacteristic;
BLEScan* pBLEScan; //Name the scanning device as pBLEScan
BLEScanResults foundDevices;
BLEAddress *Server_BLE_Address;
String Scaned_BLE_Address,
       Target_BLE_Address;
// boolean paired = false; //boolean variable to togge light
BLEClient*  pClient  = BLEDevice::createClient();

int countIsNotConnected = 0;
boolean connecting = false;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

    void onResult(BLEAdvertisedDevice advertisedDevice) {
      String nameBle = advertisedDevice.getName().c_str();
      if (nameBle == "BLE-Mamun") {
        BLEAddress *bleAddr = new BLEAddress(advertisedDevice.getAddress());
        Target_BLE_Address = bleAddr->toString().c_str();

        String typeBle = String(advertisedDevice.getAddressType());

        Serial.print("Target_BLE_Address\t" + Target_BLE_Address + "\t");
        Serial.print("typeBle\t" + typeBle + "\t");
        Serial.println();
      }

      Server_BLE_Address = new BLEAddress(advertisedDevice.getAddress());
      Scaned_BLE_Address = Server_BLE_Address->toString().c_str();

      Serial.println("Scaned_BLE_Address\t" + Scaned_BLE_Address);
      Serial.println("Target_BLE_Address\t" + Target_BLE_Address);
      if (Scaned_BLE_Address == Target_BLE_Address) {
        Serial.println("connecting to Target BLE Server as client");
        connecting = true;
      }

    }
};

void setup() {
  Serial.begin(115200); //Start serial monitor
  Serial.println("ESP32 BLE Server program"); //Intro message
  BLEDevice::init("ESP32 BLE");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); //Call the class that is defined above
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  pinMode (13, OUTPUT); //Declare the in-built LED pin as output
}

void loop() {
  boolean bleConnected = pClient->isConnected();
  if (bleConnected) {
    connecting = false;
    countIsNotConnected = 0;

    // Read the value of the characteristic.
    std::string value = pClient->getValue(serviceUUID, txCharUUID);
    Serial.println(value.c_str());
    Serial.print("The rx characteristic value was: ");

    // Write the value of the characteristic.
    String message = String(random(2, 99));
    pClient->setValue(serviceUUID, rxCharUUID, message.c_str());
    Serial.print("The tx characteristic value was: ");
    Serial.println(message.c_str());


  }

  while (!bleConnected) {
    foundDevices = pBLEScan->start(3); //Scan for 3 seconds to find the Fitness band
    int countDevices = foundDevices.getCount();
    Serial.println("countDevices\t" + String(countDevices));

    if (connecting) {
      if (connectToserver(Target_BLE_Address)) {
        break;
      }
    }

    countIsNotConnected++;
    if (countIsNotConnected > 3) {
      ESP.restart();
    }
  }
}

bool connectToserver (String address) {
  BLEAddress pAddress = BLEAddress(address.c_str());
  Serial.println(" - Created client");
  // Connect to the BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to target BLE");
  // Obtain a reference to the service we are after in the remote BLE server.

  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService != nullptr) {
    Serial.println(" - Found our service");
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(txCharUUID);
    if (pRemoteCharacteristic != nullptr) {
      Serial.println(" - Found our characteristic");
      return true;
    }
  }
  else
    return false;
}
