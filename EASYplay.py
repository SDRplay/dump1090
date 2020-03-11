#!/usr/bin/python
import gtk
import easygui
import time
import os
import sys

#calculate window position
pos = int(gtk.gdk.screen_width() * 0.5) - 250, int(gtk.gdk.screen_height() * 0.5 - 150)
rootWindowPosition = "+%d+%d" % pos

# patch rootWindowPosition
easygui.rootWindowPosition = rootWindowPosition

image = "/home/pi/SDRplay/images/logo.png"
msg = "Welcome to SDRplay's Raspberry Pi EASYplay!\n\nWould you like to continue to use EASYplay or exit?"
title = "EASYplay V0.6 Welcome Screen"
choices = ["EASYplay", "Exit to OS"]
reply = easygui.buttonbox(msg, title, choices, image=image)
if reply == "Exit to OS": exit

if reply == "EASYplay": 

	while 1:
		msg = "Select application to run"
		title = "EASYplay V0.6 Menu"
		choices = ["01. USER GUIDE - IMPORTANT INFORMATION (PDF)", "02. SoapySDRUtil (test RSP connection)", "03. SoapyRemote", "04. CubicSDR", "05. Gnu Radio", "06. GQRX", "07. ADS-B (Interactive & Network)", "08. QT-DAB DAB Receiver", "09. RSP TCP Server", "10. SoapySDR website - pothosware", "11. CubicSDR website - Charles J Cliffe", "12. QT-DAB website - Jan Van Katwijk", "13. Gnu Radio website", "14. GQRX website - Alex Csete", "15. RSP TCP Server Github", "16. RSP1 Datasheet (PDF)", "17. RSP1A Datasheet (PDF)", "18. RSP2 Datasheet (PDF)", "19. RSPduo Datasheet (PDF)", "20. API Specification (PDF)", "21. ADS-B User Guide (PDF)", "22. SDRplay website", "23. Exit"]
		choice = easygui.choicebox(msg, title, choices)

		if choice == "01. USER GUIDE - IMPORTANT INFORMATION (PDF)": os.system("/usr/bin/xpdf /home/pi/SDRplay/docs/SDRplay_RPi_Image_Guide.pdf")
		if choice == "02. SoapySDRUtil (test RSP connection)":
			os.system("x-terminal-emulator -e bash -c 'SoapySDRUtil --probe=sdrplay; read -p \"Press RETURN to go back to EASYplay...\"'")
		if choice == "03. SoapyRemote":
			os.system("x-terminal-emulator -e bash -c 'SoapySDRServer --bind'")
		if choice == "04. CubicSDR":
			os.system("x-terminal-emulator -e bash -c 'CubicSDR; read -p \"Press RETURN to go back to EASYplay...\"'")
		if choice == "05. Gnu Radio":
			os.system("x-terminal-emulator -e bash -c 'gnuradio-companion; read -p \"Press RETURN to go back to EASYplay...\"'")
		if choice == "06. GQRX":
			os.system("x-terminal-emulator -e bash -c 'LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH;PYTHONPATH=/usr/local/lib/python-2.7/dist-packages:$PYTHONPATH;/usr/local/bin/gqrx -c sdrplay.conf; read -p \"Press RETURN to go back to EASYplay...\"'")
		if choice == "07. ADS-B (Interactive & Network)":
			os.system("x-terminal-emulator -e bash -c 'cd /home/pi;dump1090 --dev-sdrplay --interactive --net; read -p \"Press RETURN to go back to EASYplay...\"'")
		if choice == "08. QT-DAB DAB Receiver": 
			os.system("x-terminal-emulator -e bash -c '/usr/local/bin/qt-dab-2.7.1; read -p \"Press RETURN to go back to EASYplay...\"'")
		if choice == "09. RSP TCP Server": 
			os.system("x-terminal-emulator -e bash -c '/usr/local/bin/rsp_tcp -E -a 0.0.0.0; read -p \"Press RETURN to go back to EASYplay...\"'")
		if choice == "10. SoapySDR website - pothosware": os.system("/usr/bin/chromium-browser https://github.com/pothosware/SoapySDR/wiki")
		if choice == "11. CubicSDR website - Charles J Cliffe": os.system("/usr/bin/chromium-browser http://cubicsdr.com")
		if choice == "12. QT-DAB website - Jan Van Katwijk": os.system("/usr/bin/chromium-browser https://www.sdr-j.tk")
		if choice == "13. Gnu Radio website": os.system("/usr/bin/chromium-browser https://gnuradio.org")
		if choice == "14. GQRX website - Alex Csete": os.system("/usr/bin/chromium-browser http://gqrx.dk")
		if choice == "15. RSP TCP Server Github": os.system("/usr/bin/chromium-browser https://github.com/SDRplay/RSPTCPServer")
		if choice == "16. RSP1 Datasheet (PDF)": os.system("/usr/bin/xpdf /home/pi/SDRplay/docs/RSP1_Datasheet.pdf")
		if choice == "17. RSP1A Datasheet (PDF)": os.system("/usr/bin/xpdf /home/pi/SDRplay/docs/RSP1A_Datasheet.pdf")
		if choice == "18. RSP2 Datasheet (PDF)": os.system("/usr/bin/xpdf /home/pi/SDRplay/docs/RSP2_Datasheet.pdf")
		if choice == "19. RSPduo Datasheet (PDF)": os.system("/usr/bin/xpdf /home/pi/SDRplay/docs/RSPduo_Datasheet.pdf")
		if choice == "20. API Specification (PDF)": os.system("/usr/bin/xpdf /home/pi/SDRplay/docs/SDRplay_SDR_API_Specification.pdf")
		if choice == "21. ADS-B User Guide (PDF)": os.system("/usr/bin/xpdf /home/pi/SDRplay/docs/SDRplay_ADS-B_User_Guide.pdf")
		if choice == "22. SDRplay website": os.system("/usr/bin/chromium-browser https://www.sdrplay.com")
		if choice == "23. Exit": sys.exit(0)
