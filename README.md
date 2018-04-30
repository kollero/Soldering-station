# Soldering-station
JBC compatible

T245B or similar

Uses a 24v 6-9A power supply that can be gotten cheaply from aliexpress.

Atmega328p and only exotic component is LMP8358, 1.3" oled display using u8g lib.

PCB was home made (toner transfered) so no real gerber files.

DIN type connector for the jbc, has to be changed to the t245b end, since couldn't find one that t245b would fit into directly.

temperature is measured in series with heater, so can rougly use 90% pwm max.
PID controlled, t245b uses IrRh temp table with 500x amplification to get correct temp readings 
overshoots a little but works fine after.

25 to 350 C degree tip heating takes about 4 seconds, and depending on tip around 150watts of power, 
so make sure power supply can handle it!

back panel has fused iec connector, also from aliexpress.
holder uses SH12a to hold the tip, and also has cheap brass tin collector.
