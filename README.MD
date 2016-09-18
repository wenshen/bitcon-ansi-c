Make sure you've install the following libs:
1. crypto
2. cunit
3. pthread
(I think you've installed the above 3 libs)

4. sqlite3

for sqlite3:

sudo apt-get install sqlite3 libsqlite3-dev
-------------------------------------------------------------------
To run the Code from Eclipse:

1.This code is tested under Ubuntu 12.04  OS environment.
2. We use Eclipse IDE Version 3.7.2 for development.
3. Please create a C project named "Bitcoin", copy all the code to the project folder. 
4. The compiler and linker configurations are shown as following:
GCC C Compiler: gcc -O0 -g3 -pedantic -Wall -c -fmessage-length=0 -ansi
GCC C Linker: gcc -lcrypto -lcunit -pthread -lsqlite3
5. Then build and run.

-------------------------------------------------------------------

To run the Code from command line:
Go into the source folder, type "make dev" in command line.
You'll see the results for the test cases.

-------------------------------------------------------------------
To do transactions:
type "make exe",
type "cd EXE", go to the "/EXE" folder.
type "./BitcoinS" to run the server
type "./BitcoinC" to run the client

Then you will see instructions like this:
"Please input your command(sendBitcoins  getBalance  quit): "

You may input "getbalance", then you'll see "Begin to calculate your balance"
and "Your Balance is: xxxx"

If you input "sendbitcoins", then you'll see "Please input your destination(IP Address): "
please input the valid ipv4 address (e.g 10.104.67.221)of your receiver. The program will
do the basic validation. You can not send bitcoins to yourself.

Then you'll see "Please input the amount: "
please input the mount of bitcoins you need to send. Also, the program will do basic validation 
to ensure that your wallet has enough bitcoins to send.

After this you'll see the notification.

You may input "getbalance" to check your balance and then type "quit" to exit the program.


-------------------------------------------------------------------
Then run the Address Generator:
You may go to the upper folder where you could find "MIBitcoin".

Then, type:

./MIBitcoin


Then, you'll see the instructions:

Enter the number of keys: 2 (integer only)

Enter a string of text for the key(30 max): helloworld2012

You'll get two Private keys, Public Keys and Addresses like the following:

Private key (hex):  6D 58 C5 3C 8F C4 E6 84 4B 01 9F 86 11 64 86 06 8E 5B 8E 77 AD 6E 1C 64 76 3D 8B 45 46 67 AF 53
Public key (hex):  04 0D BC F2 CC DD C3 29 23 8F C4 2F 2B 1F 96 73 A3 D6 75 E4 BD 95 5B 50 AE 6E F4 89 31 92 3F EF 66 53 C2 D8 D5 C8 BC DC 20 08 65 0B 47 E7 A7 54 38 3C 56 A1 E7 80 98 53 54 0A 41 14 53 B4 CE BA 50
Address (base-58): 1MJmdASWM7gogHYWR38EH3szQdvAStpt1D

Private key (hex):  F3 EB B0 B5 04 3C DC C3 48 6C 81 48 20 C2 4E AD 35 57 72 BF 92 29 68 F3 7B 5E 82 AC 03 67 73 8D
Public key (hex):  04 48 00 C5 7C B5 B3 C8 89 71 DF 53 C7 94 18 C8 09 BA 7B C7 CC 7D 31 0F C9 1C E8 60 CC 12 D3 B3 97 D7 B6 38 39 BD 84 51 7F 67 88 BF 24 07 04 C4 F6 05 62 43 B7 D3 75 DD 51 78 E8 67 DD 93 A0 96 C2
Address (base-58): 1H8kEd5vSFJgZTie9pvUrKAE2976X8xJaS


Please note that we use the real IPv4 address to do the network communication.

-----------------------------------------------------------------------------------------------------------



Thanks!





Copyright (c) 2012 The MIBitcoin Project Team




