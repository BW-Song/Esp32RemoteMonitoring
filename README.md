# RemoteMonitoring

## Create an Azure IoT Suite

1. Go to [Azure IoT Suite site](https://www.azureiotsuite.com/) and click Create a new solution.
2. Select **Remote monitoring preview**.
3. Enter a solution name, select a subscription and a region, and then click **Create solution**. The solution may take a while to be provisioned.
4. After the provision is completed, click **Launch**. Some simulated devices are created for the solution during the provision process. Click DEVICES to check them out.
5. Create a new physical device in the solution. Please take a note of the device connection string.

## Steps to start
1. Setup development environment by following [Get Started](https://github.com/microsoft/vscode-iot-workbench/blob/master/docs/esp32/esp32-get-started.md)
2. Open VS Code
3. Press **F1** or **Ctrl + Shift + P** - `IoT Workbench: Examples` and select RemoteMonitoring.

## Config Device Code
1. Open the source file(.ino) and update the lines with your WiFi ssid and password:
2. Paste the device connection string copied above into the line with device connection string.

## Build and upload the device code
1. Open the command palette and select Azure IoT Device Workbench: Upload Device Code.
2. VS Code then starts verifying and uploading the code to your DevKit.
3. The ESP32 device reboots and starts running the code.

## Test the project
When the sample app runs, DevKit sends sensor data over Wi-Fi to your Azure IoT Suite. To see the result, follow these steps:

1. Go to your Azure IoT Suite, and click DASHBOARD.
2. On the Azure IoT Suite solution console, you will see your DevKit sensor status.
