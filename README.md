# Smart-Hybrid-Incubator

 *About the System*

The Smart Hybrid Incubator System is an intelligent device that simulates optimal environmental conditions—temperature, humidity, and egg turning—to support successful egg incubation. It operates using both AC and DC power sources for flexibility and reliability, especially in areas with inconsistent electricity.

A 60W AC bulb heats the chamber to a baseline temperature of 36°C, which is essential for embryo development. Two 12V DC bulbs provide additional heating when needed, raising the temperature up to 38°C. These elements are controlled dynamically based on real-time sensor feedback to maintain stable internal conditions.
Humidity is regulated using a humidifier that activates automatically when moisture levels drop below the required threshold. Proper humidity is vital for membrane development and successful hatching.
An automated motorized system periodically rotates the eggs to prevent embryos from sticking to the shell walls. This mimics natural behavior and enhances hatch rates.

*Startup and Operation*

When powered on, the system first attempts to connect to a pre-configured WiFi network. If successful, it enters IoT mode, enabling remote monitoring and control via a mobile application. If no internet connection is available, the system switches to offline mode and proceeds with preset or manually configured values for temperature, humidity, and number of turns per day—ensuring uninterrupted operation.
Sensor data is continuously monitored and compared to setpoints. Based on this, the system adjusts the heating elements, humidifier, and turning mechanism accordingly. All current readings—temperature, humidity, and number of turns—are displayed on an onboard LCD screen for local visibility.
Data is also logged internally and can be accessed later for analysis or troubleshooting. The system ensures consistent environmental conditions throughout the incubation period, regardless of operational mode.

*IoT Mode and Mobile Application*

In IoT mode, the incubator connects to a cloud-based server where all operational data is stored securely. Users interact with the system through a dedicated Android app, which provides real-time visualization of key parameters such as temperature, humidity, number of turns, and incubation progress.
The app allows users to adjust settings remotely, including target temperature and humidity ranges, daily turning frequency, and candling schedules. Candling helps identify infertile or non-developing eggs early, saving time and resources.
Notifications and alerts are sent directly to the user’s mobile device in case of anomalies like temperature fluctuations or mechanical issues, ensuring timely intervention.
