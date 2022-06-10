# SerialCloudPrinter
Serial gateway device to upload Serial data to AWS Cloud TLS v1.2 based on an Espressif ESP32 MCU; 
the ESP32-WROVER-B (TTGO T-Call v1.4) containing an on-board SIM800L 2G Module.

<img width="1216" alt="Screenshot 2022-06-10 at 17 22 44" src="https://user-images.githubusercontent.com/8679150/173098197-95841db6-d6f1-498c-9327-81a06f587872.png">

I am opening up this project so people can perhaps learn from it.
The developments took about 6 months on both the Hardware and Software and is to be considered "Open Source" under GNU license.

Reason for this project was Weighing Scales that were connected to a Serial Matrix Printer (25 Pin's RS232).
These devices were running at their end of lifetime so the idea was to create a Serial Data Sniffer that would send all
messages to the AWS cloud.

Using the ESP32 as an MCU and a Sim800L packed together in what TTGO named the TTGO T-Call SIM800L and a custom PCB design.

I do not intend to support this project page, you are however free to clone it or use components from it for your own projects.

NOTE: This project has been built with the default Arduino IDE which can be downloaded from https://www.arduino.cc/en/software

