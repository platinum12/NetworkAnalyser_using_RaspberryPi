###############################################################
# C based local server to be run on a network capture device
# in this case Raspberry Pi
###############################################################
- This program depends in 'libpcap' library to be correctly installed
  on the target platform and uses it to capture network packets.

- This program uses Free software for parsing RadioTap Headers from Beacon packets.
  This piece of code was pulled from 'airscan' of Google Code site 
  'https://code.google.com/archive/p/airscan/'

- Files 'radiotap-parser.h', 'radiotap-parser.c', 'ieee80211_radiotap.h', 'byteorder.h'
  help parsing RadioTap Headers.

- File 'client_main.c' is the only file which contains entire code for
  (i) server component (ii) network capture (iii) parsing radio-tap information
  (iv) JSON generation

## Compile with:
gcc -Wall -pedantic client_main.c -lpcap (-o output_file_name)

## Usage:
a.out (# of packets) "filter string"

## Testing:
Software was compiled with gcc that was available on Raspberry Pi and
its working on Raspberry Pi. 

## Application flow : 
- On start accepts connection from local clients. Accepts commands from local clients.
- When it get ‘capture’ command from local clients, it will capture Beacon packets and 
  convert them into JSON string. This JSON will be sent back as response to client