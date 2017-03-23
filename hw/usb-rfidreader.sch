EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:usb-rfidreader-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L ATTINY45-P IC1
U 1 1 58CDB7A0
P 5950 3900
F 0 "IC1" H 5950 4507 50  0000 C CNN
F 1 "ATTINY45-P" H 5950 4416 50  0000 C CNN
F 2 "Housings_DIP:DIP-8_W7.62mm_LongPads" H 5950 4325 50  0000 C CIN
F 3 "" H 5950 3900 50  0000 C CNN
	1    5950 3900
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X04 P1
U 1 1 58CDB96E
P 1650 4300
F 0 "P1" H 1569 3925 50  0000 C CNN
F 1 "CONN_01X04" H 1569 4016 50  0000 C CNN
F 2 "" H 1650 4300 50  0000 C CNN
F 3 "" H 1650 4300 50  0000 C CNN
	1    1650 4300
	-1   0    0    1   
$EndComp
$Comp
L D_Zener D2
U 1 1 58CDBA59
P 2750 4600
F 0 "D2" V 2704 4679 50  0000 L CNN
F 1 "D_Zener" V 2795 4679 50  0000 L CNN
F 2 "Diodes_THT:D_DO-35_SOD27_P7.62mm_Horizontal" H 2750 4600 50  0001 C CNN
F 3 "" H 2750 4600 50  0000 C CNN
	1    2750 4600
	0    1    1    0   
$EndComp
$Comp
L D_Zener D1
U 1 1 58CDBAC9
P 2350 4600
F 0 "D1" V 2304 4679 50  0000 L CNN
F 1 "D_Zener" V 2395 4679 50  0000 L CNN
F 2 "Diodes_THT:D_DO-35_SOD27_P7.62mm_Horizontal" H 2350 4600 50  0001 C CNN
F 3 "" H 2350 4600 50  0000 C CNN
	1    2350 4600
	0    1    1    0   
$EndComp
$Comp
L R R1
U 1 1 58CDBB3B
P 2350 3850
F 0 "R1" H 2420 3896 50  0000 L CNN
F 1 "R" H 2420 3805 50  0000 L CNN
F 2 "Resistors_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 2280 3850 50  0001 C CNN
F 3 "" H 2350 3850 50  0000 C CNN
	1    2350 3850
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 58CDBBDD
P 3050 4350
F 0 "R2" V 3150 4350 50  0000 C CNN
F 1 "68R" V 3150 4500 50  0000 C CNN
F 2 "Resistors_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 2980 4350 50  0001 C CNN
F 3 "" H 3050 4350 50  0000 C CNN
	1    3050 4350
	0    1    1    0   
$EndComp
$Comp
L R R3
U 1 1 58CDBC4D
P 3050 4250
F 0 "R3" V 2950 4250 50  0000 C CNN
F 1 "68R" V 2950 4400 50  0000 C CNN
F 2 "Resistors_THT:R_Axial_DIN0204_L3.6mm_D1.6mm_P7.62mm_Horizontal" V 2980 4250 50  0001 C CNN
F 3 "" H 3050 4250 50  0000 C CNN
	1    3050 4250
	0    1    1    0   
$EndComp
$Comp
L C_Small C2
U 1 1 58CDBCD7
P 7300 3900
F 0 "C2" H 7392 3946 50  0000 L CNN
F 1 "C_Small" H 7392 3855 50  0000 L CNN
F 2 "" H 7300 3900 50  0000 C CNN
F 3 "" H 7300 3900 50  0000 C CNN
	1    7300 3900
	1    0    0    -1  
$EndComp
$Comp
L CP_Small C1
U 1 1 58CDBD7D
P 7600 3900
F 0 "C1" H 7688 3946 50  0000 L CNN
F 1 "CP_Small" H 7688 3855 50  0000 L CNN
F 2 "" H 7600 3900 50  0000 C CNN
F 3 "" H 7600 3900 50  0000 C CNN
	1    7600 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	1850 4350 2350 4350
Wire Wire Line
	2350 4350 2900 4350
Wire Wire Line
	2350 4000 2350 4350
Wire Wire Line
	2350 4350 2350 4450
Connection ~ 2350 4350
Wire Wire Line
	2900 4250 2750 4250
Wire Wire Line
	2750 4250 1850 4250
Wire Wire Line
	1850 4450 1850 4750
Wire Wire Line
	1850 4750 2350 4750
Wire Wire Line
	2350 4750 2750 4750
Wire Wire Line
	2750 4750 7600 4750
Connection ~ 2350 4750
Wire Wire Line
	7300 4000 7300 4150
Connection ~ 2750 4750
Wire Wire Line
	7300 3650 7600 3650
Wire Wire Line
	7600 3650 7600 3800
Wire Wire Line
	7300 4150 7600 4150
Wire Wire Line
	7600 4750 7600 4150
Wire Wire Line
	7600 4150 7600 4000
Wire Wire Line
	3200 4350 4250 4350
Wire Wire Line
	4250 4350 4250 3650
Wire Wire Line
	4250 3650 4600 3650
Wire Wire Line
	4600 3850 3800 3850
Wire Wire Line
	3800 3850 3800 4250
Wire Wire Line
	3800 4250 3200 4250
Wire Wire Line
	7300 3150 7300 3650
Wire Wire Line
	7300 3650 7300 3800
Connection ~ 7600 4150
Wire Wire Line
	2750 4450 2750 4250
Connection ~ 2750 4250
Wire Wire Line
	1800 3150 2350 3150
Wire Wire Line
	2350 3150 7300 3150
Wire Wire Line
	2350 3150 2350 3700
Connection ~ 7300 3650
Wire Wire Line
	1850 4150 1800 4150
Wire Wire Line
	1800 4150 1800 3150
Connection ~ 2350 3150
$EndSCHEMATC
